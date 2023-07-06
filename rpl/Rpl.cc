/*
 * Simulation model for RPL (Routing Protocol for Low-Power and Lossy Networks)
 *
 * Copyright (C) 2021  Institute of Communication Networks (ComNets),
 *                     Hamburg University of Technology (TUHH)
 *           (C) 2021  Yevhenii Shudrenko
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <deque>
#include <numeric>
#include <string>
#include <random>
#include <algorithm>
#include <regex>
#include <math.h>

#include "inet/routing/rpl/Rpl.h"

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>

#include "inet/routing/rpl/ObjectiveFunction.h" //2022-04-28 to avoid circular dependency

//#include <Python.h>
//#include "C:/Users/carlo/Anaconda3/Python.h"

namespace inet {

Define_Module(Rpl);

std::ostream& operator<<(std::ostream& os, std::vector<Ipv6Address> &list)
{
    for (auto const &addr: list) {
        os << addr << ", " << endl;
    }

    return os;
}

std::ostream& operator<<(std::ostream& os, std::map<Ipv6Address, std::pair<cMessage *, uint8_t>> acks)
{
    for (auto a : acks)
        os << a.first << " (" << a.second.second << " attempts), timeout event - " << a.second.first << endl;
    return os;
}

// TODO: Store DODAG-related info in a separate object
// TODO: Refactor utility functions out

Rpl::Rpl() :
    isRoot(false),
    dodagId(Ipv6Address::UNSPECIFIED_ADDRESS),
    daoDelay(DEFAULT_DAO_DELAY),
    hasStarted(false),
    daoAckTimeout(10),
    daoRtxCtn(0),
    detachedTimeout(2), // manually suppressing previous DODAG info [RFC 6550, 8.2.2.1]
    daoSeqNum(0),
    prefixLength(128),
    preferredParent(nullptr),
    objectiveFunctionType("hopCount"),
    dodagColor(cFigure::BLACK),
    floating(false),
    prefParentConnector(nullptr),
    numDaoDropped(0),
    udpPacketsRecv(0),
    isLeaf(false),
    numParentUpdates(0),
    numDaoForwarded(0)
{}

Rpl::~Rpl()
{
    stop();
}

void Rpl::initialize(int stage)
{
    EV_DETAIL << "Initialization stage: " << stage << endl;

    RoutingProtocolBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        interfaceTable = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        networkProtocol = getModuleFromPar<INetfilter>(par("networkProtocolModule"), this);
        nd = check_and_cast<Ipv6NeighbourDiscovery*>(getModuleByPath("^.ipv6.neighbourDiscovery"));

        routingTable = getModuleFromPar<Ipv6RoutingTable>(par("routingTableModule"), this);
        trickleTimer = check_and_cast<TrickleTimer*>(getModuleByPath("^.trickleTimer"));

        daoEnabled = par("daoEnabled").boolValue();
        host = getContainingNode(this);

        objectiveFunction = new ObjectiveFunction(par("objectiveFunctionType").stdstringValue());
        objectiveFunction->setMinHopRankIncrease(par("minHopRankIncrease").intValue());
        daoRtxThresh = par("numDaoRetransmitAttempts").intValue();
        allowDodagSwitching = par("allowDodagSwitching").boolValue();
        pDaoAckEnabled = par("daoAckEnabled").boolValue();
        pUseWarmup = par("useWarmup").boolValue();

        // statistic signals
        dioReceivedSignal = registerSignal("dioReceived");
        daoReceivedSignal = registerSignal("daoReceived");
        parentUnreachableSignal = registerSignal("parentUnreachable");

        DIOsent.setName("DIOsent");  //CL 2021-08-05
        DAOsent.setName("DAOsent");  //CL 2021-08-05

        startDelay = par("startDelay").doubleValue();

        WATCH(numParentUpdates);


    }
    else if (stage == INITSTAGE_ROUTING_PROTOCOLS) {
        registerService(Protocol::manet, nullptr, gate("ipIn"));
        registerProtocol(Protocol::manet, gate("ipOut"), nullptr);
        host->subscribe(linkBrokenSignal, this);
        networkProtocol->registerHook(0, this);
    }

}

int Rpl::getNodeId(std::string nodeName) {
    const std::regex base_regex("host\\[([0-9]+)\\]");
    std::smatch base_match;

    if (std::regex_match(nodeName, base_match, base_regex)) {
        // The first sub_match is the whole string; the next
        // sub_match is the first parenthesized expression.
        if (base_match.size() == 2) {
            std::ssub_match base_sub_match = base_match[1];
            std::string base = base_sub_match.str();
            return std::stoi(base);
        }
    }
    return -1;
}

cFigure::Color Rpl::pickRandomColor() {
    auto palette = selfId % 2 == 0 ? cFigure::GOOD_DARK_COLORS : cFigure::GOOD_LIGHT_COLORS;
    auto numColors = selfId % 2 == 0 ? cFigure::NUM_GOOD_DARK_COLORS : cFigure::NUM_GOOD_LIGHT_COLORS;
    return host->getParentModule()->par("numSinks").intValue() == 1 ?  cFigure::BLACK : palette[intrand(numColors, 0) - 1];
}

cModule* Rpl::findSubmodule(std::string sname, cModule *host) {
    for (cModule::SubmoduleIterator it(host); !it.end(); ++it) {
        cModule *subm = *it;
        std::string s(subm->getFullName());

        if (s.find(sname) != std::string::npos)
            return *it;
    }
    return nullptr;
}

std::vector<int> Rpl::pickRandomly(int total, int numRequested) {
    // shuffle the array
    std::random_device rd;
    std::mt19937 e{rd()};

    std::vector<int> values(total);

    std::iota(values.begin(), values.end(), 1);

    std::shuffle(values.begin(), values.end(), e);

    // copy the picked ones
    std::vector<int> picked(numRequested);
//    for (auto i = 0; i < numRequested; i++)
//        picked.push_back(inputVec[i]);

    std::copy(values.begin(), values.begin() + numRequested, picked.begin());
    return picked;
}

//
// Lifecycle operations
//

void Rpl::start()
{
    InterfaceEntry* ie = nullptr;

    if (startDelay > 0) {
        scheduleAt(simTime() + startDelay, new cMessage("", RPL_START));
        return;
    }
    hasStarted = true;

    isRoot = par("isRoot").boolValue(); // Initialization of this parameter should be here to ensure
                                        // multi-gateway configurator will have time to assign 'root' roles
                                        // to randomly chosen nodes
    position = *(new Coord());

    // set network interface entry pointer (TODO: Update for IEEE 802.15.4)
    for (int i = 0; i < interfaceTable->getNumInterfaces(); i++)
    {
        ie = interfaceTable->getInterface(i);
        if (strstr(ie->getInterfaceName(), "wlan") != nullptr)
        {
            interfaceEntryPtr = ie;
            break;
        }
    }

    selfId = interfaceTable->getInterface(1)->getMacAddress().getInt();
    auto mobility = check_and_cast<StationaryMobility*> (getParentModule()->getSubmodule("mobility"));
    position = mobility->getCurrentPosition();

    udpApp = host->getSubmodule("app", 0); // TODO: handle more apps

    rank = INF_RANK - 1;
    detachedTimeoutEvent = new cMessage("", DETACHED_TIMEOUT);

    // Metrics updater timer:
    metric_updater_timer = new cMessage("time for update metrics", METRIC_TIMER);  //CL 2022-02-19
    scheduleAt(simTime() + 10, metric_updater_timer);
    //end

    if (isRoot && !par("disabled").boolValue()) {
        trickleTimer->start(pUseWarmup, par("numSkipTrickleIntervalUpdates").intValue());
        dodagColor = pickRandomColor();
        rank = ROOT_RANK;
        dodagVersion = DEFAULT_INIT_DODAG_VERSION;
        instanceId = RPL_DEFAULT_INSTANCE;
        dtsn = 0;
        storing = par("storing").boolValue();
        dodagId = getSelfAddress(); //CL 2021-11-25 to solve problems when sink receives DAO messages

        if (udpApp)
            udpApp->subscribe("packetReceived", this);
    }
}

void Rpl::refreshDisplay() const {
    if (isRoot) {
        host->getDisplayString().setTagArg("t", 0, std::string(" num rcvd: " + std::to_string(udpPacketsRecv)).c_str());
        host->getDisplayString().setTagArg("t", 1, "l"); // set display text position to 'left'
    }
}

void Rpl::stop()
{
    cancelAndDelete(detachedTimeoutEvent);
    //cancelAndDelete(metric_updater_timer);  //CL 2022-02-19

}

void Rpl::handleMessageWhenUp(cMessage *message)
{
    if (message->isSelfMessage())
        processSelfMessage(message);
    else
        processMessage(message);
}


void Rpl::processSelfMessage(cMessage *message)
{
    switch (message->getKind()) {
        case DETACHED_TIMEOUT: {
            floating = false;
            EV_DETAIL << "Detached state ended, processing new incoming RPL packets" << endl;
            break;
        }
        case RPL_START: {
            startDelay = 0;
            start();
            break;
        }
        case DAO_ACK_TIMEOUT: {
            Ipv6Address *advDest = (Ipv6Address*) message->getContextPointer();
            if (pendingDaoAcks.find(*advDest) == pendingDaoAcks.end())
                EV_WARN << "Received DAO_ACK timeout for deleted entry, ignoring" << endl;
            else
                retransmitDao(*advDest);
            break;
        }
        case METRIC_TIMER: {
            updateMetrics_frequently();
            break;
        }
        default: EV_WARN << "Unknown self-message received - " << message << endl;

    //CL 22-02-22
    //if (message == metric_updater_timer)
      //  updateMetrics_frequently();
    //end
    }
    delete message;
}

void Rpl::clearDaoAckTimer(Ipv6Address daoDest) {
    if (pendingDaoAcks.count(daoDest))
        if (pendingDaoAcks[daoDest].first && pendingDaoAcks[daoDest].first->isSelfMessage())
            cancelEvent(pendingDaoAcks[daoDest].first);
    pendingDaoAcks.erase(daoDest);
}

void Rpl::retransmitDao(Ipv6Address advDest) {
    EV_DETAIL << "DAO_ACK for " << advDest << " timed out, attempting retransmit" << endl;

    if (!preferredParent) {
        EV_WARN << "Preferred parent not set, cannot retransmit DAO"
                << "erasing entry from pendingDaoAcks " << endl;
        clearDaoAckTimer(advDest);
        return;
    }

    auto rtxCtn = pendingDaoAcks[advDest].second++;
    if (rtxCtn > daoRtxThresh) {
        EV_DETAIL << "Retransmission threshold (" << std::to_string(daoRtxThresh)
            << ") exceeded, erasing corresponding entry from pending ACKs" << endl;
        clearDaoAckTimer(advDest);
        numDaoDropped++;
        return;
    }

    EV_DETAIL << "(" << std::to_string(rtxCtn) << " attempt)" << endl;

    sendRplPacket(createDao(advDest), DAO, preferredParent->getSrcAddress(), daoDelay);
}

void Rpl::detachFromDodag() {
    /**
     * If parent set of a node turns empty, it is no longer associated
     * with a DODAG and should suppress previous RPL state info by
     * clearing dodagId, neighbor sets and setting it's rank to INFINITE_RANK [RFC6560, 8.2.2.1]
     */
    EV_DETAIL << "Candidate parent list empty, leaving DODAG" << endl;
    backupParents.erase(backupParents.begin(), backupParents.end());
    EV_DETAIL << "Backup parents list erased" << endl;
    /** Delete all routes associated with DAO destinations of the former DODAG */
    purgeDaoRoutes();
    rank = INF_RANK;
    trickleTimer->suspend();
    if (par("poisoning").boolValue())
        poisonSubDodag();
    dodagId = Ipv6Address::UNSPECIFIED_ADDRESS;
    floating = true;
    EV_DETAIL << "Detached state enabled, no RPL packets will be processed for "
            << (int)detachedTimeout << "s" << endl;
    cancelEvent(detachedTimeoutEvent);
    drawConnector(position, cFigure::BLACK);
    scheduleAt(simTime() + detachedTimeout, detachedTimeoutEvent);
}

void Rpl::purgeDaoRoutes() {
    std::list<Ipv6Route *> purgedRoutes;
    EV_DETAIL << "Purging DAO routes from the routing table: " << endl;
    auto numRoutes = routingTable->getNumRoutes();
    for (int i = 0; i < numRoutes; i++) {
        auto ri = routingTable->getRoute(i);
        auto routeData = dynamic_cast<RplRouteData *> (ri->getProtocolData());
        if (routeData && routeData->getDodagId() == dodagId && routeData->getInstanceId() == instanceId)
            purgedRoutes.push_front(ri);
    }
    if (!purgedRoutes.empty()) {
        for (auto route : purgedRoutes)
            EV_DETAIL << route->getDestPrefix() << " via " << route->getNextHop()
                    << " : " << boolStr(routingTable->deleteRoute(route), "Success", "Fail");
    } else
        EV_DETAIL << "No DAO-associated routes found" << endl;
}

void Rpl::poisonSubDodag() {
    ASSERT(rank == INF_RANK);
    EV_DETAIL << "Poisoning sub-dodag by advertising INF_RANK " << endl;
    sendRplPacket(createDio(), DIO, Ipv6Address::ALL_NODES_1, uniform(1, 2));
}

//
// Handling generic packets
//

void Rpl::processMessage(cMessage *message)
{
    if (!hasStarted || par("disabled").boolValue())
        return;
    std::string arrivalGateName = std::string(message->getArrivalGate()->getBaseName());
    if (arrivalGateName.compare("ttModule") == 0)
        processTrickleTimerMsg(message);
    else if (Packet *fp = dynamic_cast<Packet *>(message)) {
        try {
            processPacket(fp);
        }
        catch (std::exception &e) {
            EV_WARN << "Error occured during packet processing: " << e.what() << endl;
        }
    }
}

void Rpl::processTrickleTimerMsg(cMessage *message)
{
    /**
     * Process signal from trickle timer module,
     * indicating DIO broadcast event [RFC6560, 8.3]
     */
    EV_DETAIL << "Processing msg from trickle timer" << endl;
    switch (message->getKind()) {
        case TRICKLE_TRIGGER_EVENT: {
            /**
             * Broadcast DIO only if number of DIOs heard
             * from other nodes <= redundancyConstant (k) [RFC6206, 4.2]
             */
            if (trickleTimer->checkRedundancyConst()) {
                EV_DETAIL << "Redundancy OK, broadcasting DIO" << endl;
                sendRplPacket(createDio(), DIO, Ipv6Address::ALL_NODES_1, uniform(0, 1));
            }
            break;
        }
        default: throw cRuntimeError("Unknown TrickleTimer message");
    }
    delete message;
}

bool Rpl::isRplPacket(Packet *packet) {
    auto fullname = std::string(packet->getFullName());
    return !(fullname.find("DIO") == std::string::npos && fullname.find("DAO") == std::string::npos);
}

void Rpl::processPacket(Packet *packet)
{
    if (floating || !isRplPacket(packet)) {
        EV_DETAIL << "Not processing packet " << packet
                << "\n due to floating (detached) state or because of unsupported packet type" << endl;
        delete packet;
        return;
    }

    RplHeader* rplHeader;

    EV_DETAIL << "Processing packet -" << packet << endl;  //CL

    try {
        rplHeader = const_cast<RplHeader*> ((packet->popAtFront<RplHeader>()).get());
    } catch (...) {
        EV_WARN << "Error trying to pop RPL header from " << packet << endl;
        return;
    }

    EV_INFO <<"inside of Rpl::processPacket(Packet *packet)" << endl;  //CL

    EV_INFO <<"dodagId: " << dodagId << endl;

    // in non-storing mode check for RPL Target, Transit Information options
    if (!storing && dodagId != Ipv6Address::UNSPECIFIED_ADDRESS){
        EV_INFO <<"inside of  if (!storing && dodagId != Ipv6Address::UNSPECIFIED_ADDRESS)" << endl;
        extractSourceRoutingData(packet);
        EV_INFO <<"after:  extractSourceRoutingData(packet) " << endl;}
    auto rplBody = packet->peekData<RplPacket>();
    switch (rplHeader->getIcmpv6Code()) {
        case DIO: {
            //auto app = check_and_cast<UdpBasicApp*> (host->getSubmodule("app", 0)); //CL to test access to app module
            //EV_INFO <<"numSent = " << app->numSent;//CL to test access to app module //CL to test access to app module
            //pre_processDio
            processDio(dynamicPtrCast<const Dio>(rplBody));
            //processDio(dynamicPtrCast<Dio>(rplBody)); //CL 2022-09-13 it did not work.
            break;
        }
        case DAO: {
            EV_INFO << "I'm here" << endl;
            processDao(dynamicPtrCast<const Dao>(rplBody));
            break;
        }
        case DAO_ACK: {
            processDaoAck(dynamicPtrCast<const Dao>(rplBody));
            break;
        }
        default: EV_WARN << "Unknown Rpl packet" << endl;
    }

    delete packet;
}

int Rpl::getNumDownlinks() {
    EV_DETAIL << "Calculating number of downlinks" << endl;
    auto numRts = routingTable->getNumRoutes();
    int downlinkRoutes = 0;
    for (int i = 0; i < numRts; i++) {
        auto ri = routingTable->getRoute(i);
        auto dest = ri->getDestinationAsGeneric().toIpv6();

        if (dest.isUnicast() && ri->getPrefixLength() == prefixLength)
            downlinkRoutes++;
    }


    return downlinkRoutes - 1; // minus uplink route through the preferred parent
}

void Rpl::sendPacket(cPacket *packet, double delay)
{
    if (delay == 0)
        send(packet, "ipOut");
    else
        sendDelayed(packet, delay, "ipOut");
}

//
// Handling RPL packets
//

void Rpl::sendRplPacket(const Ptr<RplPacket>& body, RplPacketCode code,
        const L3Address& nextHop, double delay)
{
    sendRplPacket(body, code, nextHop, delay, Ipv6Address::UNSPECIFIED_ADDRESS, Ipv6Address::UNSPECIFIED_ADDRESS);
}

void Rpl::sendRplPacket(const Ptr<RplPacket> &body, RplPacketCode code,
        const L3Address& nextHop, double delay, const Ipv6Address &target, const Ipv6Address &transit)
{
    EV_INFO <<" I am inside: sendRplPacket" << endl;  //CL

    if (code == CROSS_LAYER_CTRL)
        EV_DETAIL << "Preparing to broadcast cross-layer control DIO " << endl;

    Packet *pkt = new Packet(std::string("inet::RplPacket::" + rplIcmpCodeToStr(code)).c_str());
    auto header = makeShared<RplHeader>();
    header->setIcmpv6Code(code);
    pkt->addTag<PacketProtocolTag>()->setProtocol(&Protocol::manet);
    pkt->addTag<DispatchProtocolReq>()->setProtocol(&Protocol::ipv6);
//    if (interfaceEntryPtr)
//        pkt->addTag<InterfaceReq>()->setInterfaceId(interfaceEntryPtr->getInterfaceId());
    auto addresses = pkt->addTag<L3AddressReq>();
    addresses->setSrcAddress(getSelfAddress());
    addresses->setDestAddress(nextHop);
    pkt->insertAtFront(header);
    pkt->insertAtBack(body);
    // append RPL Target + Transit option headers if corresponding addresses were provided (non-storing mode)
    if (target != Ipv6Address::UNSPECIFIED_ADDRESS && transit != Ipv6Address::UNSPECIFIED_ADDRESS)
        appendDaoTransitOptions(pkt, target, transit);

    if (code == DAO && ((dynamicPtrCast<Dao>) (body))->getDaoAckRequired()) {
        auto outgoingDao = (dynamicPtrCast<Dao>) (body);
        auto advertisedDest = outgoingDao->getReachableDest();
        auto timeout = simTime() + SimTime(daoAckTimeout, SIMTIME_S) * uniform(1, 3); // TODO: Magic numbers

        EV_DETAIL << "Scheduling DAO_ACK timeout at " << timeout << " for advertised dest "
                << advertisedDest << endl;

        cMessage *daoTimeoutMsg = new cMessage("", DAO_ACK_TIMEOUT);
        daoTimeoutMsg->setContextPointer(new Ipv6Address(advertisedDest));
        EV_DETAIL << "Created DAO_ACK timeout msg with context pointer - "
                << *((Ipv6Address *) daoTimeoutMsg->getContextPointer()) << endl;

        auto daoAckEntry = pendingDaoAcks.find(advertisedDest);
        if (daoAckEntry != pendingDaoAcks.end()) {
            pendingDaoAcks[advertisedDest].first = daoTimeoutMsg;
            EV_DETAIL << "Found existing entry in DAO_ACKs map for dest - " << advertisedDest
                    << " updating timeout event ptr" << endl;
        }
        else
            pendingDaoAcks[advertisedDest] = {daoTimeoutMsg, 0};

        EV_DETAIL << "Pending DAO_ACKs:" << endl;
        for (auto e : pendingDaoAcks)
            EV_DETAIL << e.first << " (" << std::to_string(e.second.second) << " attempts), timeout msg ptr - " << e.second.first << endl;
        scheduleAt(timeout, daoTimeoutMsg);
    }
    sendPacket(pkt, delay);

}

const Ptr<Dio> Rpl::createDio()
{
    auto dio = makeShared<Dio>();
    dio->setInstanceId(instanceId);
    dio->setChunkLength(getDioSize());
    dio->setStoring(storing);
    dio->setRank(rank);
    dio->setDtsn(dtsn);
    dio->setNodeId(selfId);
    dio->setDodagVersion(dodagVersion);
    dio->setDodagId(isRoot ? getSelfAddress() : dodagId);
    dio->setSrcAddress(getSelfAddress());
    dio->setPosition(position);
    if (isRoot)
        dio->setColor(dodagColor);
     else
        dio->setColor(preferredParent ? preferredParent->getColor() : cFigure::GREY);

    EV_DETAIL << "DIO created advertising DODAG - " << dio->getDodagId()
                << " and rank " << dio->getRank() << endl;

    numDIOSent++; //CL 2021-08-05
    DIOsent.record(numDIOSent); //CL 2021-08-05

    std::ofstream file;  //2022-05-12
    file.open("St_DIOsent.txt", std::ios_base::app);
    file << simTime() << " " << getParentModule()->getFullName() << endl ;
    file.close();

    dio->setNumDIO(numDIOSent);

    std::string OF = par("objectiveFunctionType").stdstringValue(); //CL 2021-11-02  0:ETX 1:HOP_COUNT 2:Energy
    EV_INFO << "I am using OF:" << OF << endl;
    if (OF == "ETX")
        dio->setOcp(ETX);
    else if (OF == "HOP_COUNT")
        dio->setOcp(HOP_COUNT);
    else if (OF == "HC_MOD")
        dio->setOcp(HC_MOD);
    else if (OF == "ML")
        dio->setOcp(ML);
    else if (OF=="RPL_ENH")
        dio->setOcp(RPL_ENH);
    else if (OF=="RPL_ENH2")
        dio->setOcp(RPL_ENH2);
    else
        dio->setOcp(HOP_COUNT);

    dio->setHC(hc);  //CL 2021-12-02
    dio->setETX(etx); //CL 2021-12-02
    //dio->setDROP(drop); //CL 2021-12-03  //it was using to test
    //15-02-2022
    //VERY IMPORTANT: the value that I put in my DIO is the value that I read from my MAC layer, but
    //the value that I save to make decisions respect to my preferred parent is the one that I receive from
    //other DIOs
    auto mac = check_and_cast<Ieee802154Mac *>(host->getSubmodule("wlan",0)->getSubmodule("mac"));
    //dio->setDropT(mac->nbDroppedFrames_copy);      //nbDroppedFrames in L2
    dio->setDropB(mac->framedropbycongestion_copy);      //Frames drop by Backoff (congestion)
    dio->setDropR(mac->framedropbyretry_limit_reached_copy);      //Frames drop by Retry (collisions or channel interferences)
    dio->setMissACK(mac->nbMissedAcks_copy); //nbMissed ACKs
    //dio->setTxF(mac->nbTxFrames_copy);         //nbTxFrames
    //dio->setRxF(mac->nbRxFrames_copy);         //nbRxFrames
    //dio->setBw(mac->channel_util);           //bandwidth, pending of implementing
    dio->setDen(getneighbors());
    dio->setQu(mac->qu);
    //dio->setFps((mac->nbTxFrames_copy + mac->nbRxFrames_copy)/simTime()); //Frames per seconds 2022-05-05

    //dio->setPath_cost(path_cost);

    if (isRoot)
        dio->setPath_cost(path_cost); //10/17/2022
    else{
        if(rank == 2)
            dio->setPath_cost(objectiveFunction->Path_Cost_Calculator(preferredParent)); //Calculate ranking with respect to the root
        else {
            //getting the path_cost through my best forwarding candidate at the time of sending a dio
            if (simTime() < 100)
                dio->setPath_cost(0);
                //dio->setPath_cost(hc + 1);
            else
                dio->setPath_cost(objectiveFunction->GetBestCandidatePATHCOST(candidateParents));
        }
    }


    dio->setSnr(snr); //For SNR the analysis is different. The DIO carries any (in this case zero) value of SNR, once
                      // receiving the DIO the receiver update the SNR at the receiver.

    dio->setLast_update(simTime()) ; //11/03/2022 ... I want to know how out of date is my DIO information later

    auto radio = check_and_cast<Radio *>(host->getSubmodule("wlan",0)->getSubmodule("radio")); //CL: 2022-09-13
    //EV_INFO << " SNR at receiving this sender: " << radio->snr_L1 << endl;
    dio->setRx_un_suc(radio->rx_unsuccessful);  //value read from my radio layer
    double fail_retry_current_value;
    double fail_cong_current_value;
    double rx_frame_rate;
    double current_rx_frame_rate = mac->nbRxFrames_copy;

    if (simTime() - mac->last_reading > 300){
        double beta = 0.9;
        dio->setRx_suc_rate(beta*(radio->rx_successful_rate) + (1-beta)*radio->rx_successful_rate_copy);
        if (mac->nbTxFrames_copy + mac->txAttempts_copy == 0)
            fail_retry_current_value = 0;
        else
            fail_retry_current_value = (mac->framedropbyretry_limit_reached + mac->txAttempts_copy)/(mac->nbTxFrames_copy + mac->txAttempts_copy);

        dio->setTxF(beta*fail_retry_current_value + (1-beta)*mac->fail_rate_retry);

        if (mac->nbTxFrames_copy + mac->txAttempts_copy + mac->NB_copy == 0)
            fail_cong_current_value = 0;
        else
            fail_cong_current_value = (mac->framedropbycongestion + mac->txAttempts_copy + mac->NB_copy)/(mac->nbTxFrames_copy + mac->txAttempts_copy + mac->NB_copy);

        dio->setRxF(beta*fail_cong_current_value + (1-beta)*mac->fail_rate_cong);

        rx_frame_rate = beta*current_rx_frame_rate + (1-beta)*mac->rx_frames_rate ;  //check this!!!!!!!!

    } else {
        double beta = 0.6;
        dio->setRx_suc_rate(beta*(radio->rx_successful_rate) + (1-beta)*radio->rx_successful_rate_copy);
        EV_INFO << "rx_successful_rate: " << radio->rx_successful_rate   << endl;
        EV_INFO << "rx_successful_rate_copy: " << radio->rx_successful_rate_copy  << endl;
        if (mac->nbTxFrames_copy + mac->txAttempts_copy == 0)
            fail_retry_current_value = 0;
        else
            fail_retry_current_value = (mac->framedropbyretry_limit_reached + mac->txAttempts_copy)/(mac->nbTxFrames_copy + mac->txAttempts_copy);

        dio->setTxF(beta*fail_retry_current_value + (1-beta)*mac->fail_rate_retry);

        if (mac->nbTxFrames_copy + mac->txAttempts_copy + mac->NB_copy == 0)
            fail_cong_current_value = 0;
        else
            fail_cong_current_value = (mac->framedropbycongestion + mac->txAttempts_copy + mac->NB_copy)/(mac->nbTxFrames_copy + mac->txAttempts_copy + mac->NB_copy);

        dio->setRxF(beta*fail_cong_current_value + (1-beta)*mac->fail_rate_cong);

        rx_frame_rate = beta*current_rx_frame_rate + (1-beta)*mac->rx_frames_rate ;  //check this!!!!!!!!
    }

    dio->setFps (rx_frame_rate); //2022-11-06

    //To do the same with the ch utilization
            double current_ch_util;
            if (mac->busy + mac->idle == 0)
                current_ch_util = 0.5;
            else
                current_ch_util = mac->busy/(mac->busy + mac->idle);

            double my_ch_util = 0;

            if (simTime() - mac->last_ch_uti_reset > 300){
                double beta = 0.9;
                my_ch_util = beta*current_ch_util + (1-beta)*mac->channel_util ;
            } else {
                double beta = 0.6;
                my_ch_util = beta*current_ch_util + (1-beta)*mac->channel_util ;
            }

            dio->setBw(my_ch_util);

    return dio;
}

const Ptr<Dio> Rpl::createDio_Unique()
{
    auto dio = makeShared<Dio>();
    dio->setInstanceId(instanceId);
    dio->setChunkLength(getDioSize());
    dio->setStoring(storing);
    dio->setRank(rank);
    dio->setDtsn(dtsn);
    dio->setNodeId(selfId);
    dio->setDodagVersion(dodagVersion);
    dio->setDodagId(isRoot ? getSelfAddress() : dodagId);
    dio->setSrcAddress(getSelfAddress());
    dio->setPosition(position);
    if (isRoot)
        dio->setColor(dodagColor);
     else
        dio->setColor(preferredParent ? preferredParent->getColor() : cFigure::GREY);

    EV_DETAIL << "DIO created advertising DODAG - " << dio->getDodagId()
                << " and rank " << dio->getRank() << endl;

    numDIOSent++; //CL 2021-08-05
    DIOsent.record(numDIOSent); //CL 2021-08-05

    std::ofstream file;  //2022-05-12
    file.open("St_DIOsent.txt", std::ios_base::app);
    file << simTime() << " " << getParentModule()->getFullName() << endl ;
    file.close();

    dio->setNumDIO(numDIOSent);

    std::string OF = par("objectiveFunctionType").stdstringValue(); //CL 2021-11-02  0:ETX 1:HOP_COUNT 2:Energy
    EV_INFO << "I am using OF:" << OF << endl;
    if (OF == "ETX")
        dio->setOcp(ETX);
    else if (OF == "HOP_COUNT")
        dio->setOcp(HOP_COUNT);
    else if (OF == "HC_MOD")
        dio->setOcp(HC_MOD);
    else if (OF == "ML")
        dio->setOcp(ML);
    else if (OF=="RPL_ENH")
        dio->setOcp(RPL_ENH);
    else if (OF=="RPL_ENH2")
        dio->setOcp(RPL_ENH2);
    else
        dio->setOcp(HOP_COUNT);

    dio->setHC(hc);  //CL 2021-12-02
    dio->setETX(etx); //CL 2021-12-02
    //dio->setDROP(drop); //CL 2021-12-03  //it was using to test
    //15-02-2022
    //VERY IMPORTANT: the value that I put in my DIO is the value that I read from my MAC layer, but
    //the value that I save to make decisions respect to my preferred parent is the one that I receive from
    //other DIOs
    auto mac = check_and_cast<Ieee802154Mac *>(host->getSubmodule("wlan",0)->getSubmodule("mac"));
    //dio->setDropT(mac->nbDroppedFrames_copy);      //nbDroppedFrames in L2
    dio->setDropB(mac->framedropbycongestion_copy);      //Frames drop by Backoff (congestion)
    dio->setDropR(mac->framedropbyretry_limit_reached_copy);      //Frames drop by Retry (collisions or channel interferences)
    dio->setMissACK(mac->nbMissedAcks_copy); //nbMissed ACKs
    //dio->setTxF(mac->nbTxFrames_copy);         //nbTxFrames
    //dio->setRxF(mac->nbRxFrames_copy);         //nbRxFrames
    //dio->setBw(mac->channel_util);           //bandwidth, pending of implementing
    dio->setDen(getneighbors());
    dio->setQu(mac->qu);
    //dio->setFps((mac->nbTxFrames_copy + mac->nbRxFrames_copy)/simTime()); //Frames per seconds 2022-05-05

    //dio->setPath_cost(path_cost);

    if (isRoot)
        dio->setPath_cost(path_cost); //10/17/2022
    else{
        if(rank == 2)
            dio->setPath_cost(objectiveFunction->Path_Cost_Calculator(preferredParent)); //Calculate ranking with respect to the root
        else {
            //getting the path_cost through my best forwarding candidate at the time of sending a dio
            if (simTime() < 100)
                dio->setPath_cost(0);
                //dio->setPath_cost(hc + 1);
            else
                dio->setPath_cost(objectiveFunction->GetBestCandidatePATHCOST(candidateParents));
        }
    }


    dio->setSnr(snr); //For SNR the analysis is different. The DIO carries any (in this case zero) value of SNR, once
                      // receiving the DIO the receiver update the SNR at the receiver.

    dio->setLast_update(simTime()) ; //11/03/2022 ... I want to know how out of date is my DIO information later

    auto radio = check_and_cast<Radio *>(host->getSubmodule("wlan",0)->getSubmodule("radio")); //CL: 2022-09-13
    //EV_INFO << " SNR at receiving this sender: " << radio->snr_L1 << endl;
    dio->setRx_un_suc(radio->rx_unsuccessful);  //value read from my radio layer
    double fail_retry_current_value;
    double fail_cong_current_value;
    double rx_frame_rate;
    double current_rx_frame_rate = mac->nbRxFrames_copy;

    if (simTime() - mac->last_reading > 300){
        double beta = 0.9;
        dio->setRx_suc_rate(beta*(radio->rx_successful_rate) + (1-beta)*radio->rx_successful_rate_copy);
        if (mac->nbTxFrames_copy + mac->txAttempts_copy == 0)
            fail_retry_current_value = 0;
        else
            fail_retry_current_value = (mac->framedropbyretry_limit_reached + mac->txAttempts_copy)/(mac->nbTxFrames_copy + mac->txAttempts_copy);

        dio->setTxF(beta*fail_retry_current_value + (1-beta)*mac->fail_rate_retry);

        if (mac->nbTxFrames_copy + mac->txAttempts_copy + mac->NB_copy == 0)
            fail_cong_current_value = 0;
        else
            fail_cong_current_value = (mac->framedropbycongestion + mac->txAttempts_copy + mac->NB_copy)/(mac->nbTxFrames_copy + mac->txAttempts_copy + mac->NB_copy);

        dio->setRxF(beta*fail_cong_current_value + (1-beta)*mac->fail_rate_cong);

        rx_frame_rate = beta*current_rx_frame_rate + (1-beta)*mac->rx_frames_rate ;  //check this!!!!!!!!

    } else {
        double beta = 0.6;
        dio->setRx_suc_rate(beta*(radio->rx_successful_rate) + (1-beta)*radio->rx_successful_rate_copy);
        EV_INFO << "rx_successful_rate: " << radio->rx_successful_rate   << endl;
        EV_INFO << "rx_successful_rate_copy: " << radio->rx_successful_rate_copy  << endl;
        if (mac->nbTxFrames_copy + mac->txAttempts_copy == 0)
            fail_retry_current_value = 0;
        else
            fail_retry_current_value = (mac->framedropbyretry_limit_reached + mac->txAttempts_copy)/(mac->nbTxFrames_copy + mac->txAttempts_copy);

        dio->setTxF(beta*fail_retry_current_value + (1-beta)*mac->fail_rate_retry);

        if (mac->nbTxFrames_copy + mac->txAttempts_copy + mac->NB_copy == 0)
            fail_cong_current_value = 0;
        else
            fail_cong_current_value = (mac->framedropbycongestion + mac->txAttempts_copy + mac->NB_copy)/(mac->nbTxFrames_copy + mac->txAttempts_copy + mac->NB_copy);

        dio->setRxF(beta*fail_cong_current_value + (1-beta)*mac->fail_rate_cong);

        rx_frame_rate = beta*current_rx_frame_rate + (1-beta)*mac->rx_frames_rate ;  //check this!!!!!!!!
    }

    dio->setFps (rx_frame_rate); //2022-11-06

    //To do the same with the ch utilization
            double current_ch_util;
            if (mac->busy + mac->idle == 0)
                current_ch_util = 0.5;
            else
                current_ch_util = mac->busy/(mac->busy + mac->idle);

            double my_ch_util = 0;

            if (simTime() - mac->last_ch_uti_reset > 300){
                double beta = 0.9;
                my_ch_util = beta*current_ch_util + (1-beta)*mac->channel_util ;
            } else {
                double beta = 0.6;
                my_ch_util = beta*current_ch_util + (1-beta)*mac->channel_util ;
            }

            dio->setBw(my_ch_util);

    return dio;
}

// @p channelOffset - cross-layer specific parameter, useless in default RPL
const Ptr<Dao> Rpl::createDao(const Ipv6Address &reachableDest, uint8_t channelOffset)
{
    EV_INFO << "I'm using createDao(const Ipv6Address &reachableDest, uint8_t channelOffset)" << endl;
    auto dao = makeShared<Dao>();
    dao->setInstanceId(instanceId);
    dao->setChunkLength(b(64));
    dao->setChOffset(channelOffset);
    dao->setSrcAddress(getSelfAddress());
    dao->setReachableDest(reachableDest);
    dao->setSeqNum(daoSeqNum++);
    dao->setNodeId(selfId);
    dao->setDaoAckRequired(pDaoAckEnabled);
    EV_DETAIL << "Created DAO with seqNum = " << std::to_string(dao->getSeqNum()) << " advertising " << reachableDest << endl;

    numDAOSent++; //CL 2021-08-05
    DAOsent.record(numDAOSent); //CL 2021-08-05

    return dao;
}

const Ptr<Dao> Rpl::createDao(const Ipv6Address &reachableDest, bool ackRequired)
{
    EV_INFO << "I'm using createDao(const Ipv6Address &reachableDest, bool ackRequired)" << endl;
    auto dao = makeShared<Dao>();
    dao->setInstanceId(instanceId);
    dao->setChunkLength(b(64));
    //dao->setChunkLength(B(40)); //CL
    dao->setChOffset(UNDEFINED_CH_OFFSET);
    dao->setSrcAddress(getSelfAddress());
    dao->setReachableDest(reachableDest);
    dao->setSeqNum(daoSeqNum++);
    dao->setNodeId(selfId);
    dao->setDaoAckRequired(ackRequired);
    EV_DETAIL << "Created DAO with seqNum = " << std::to_string(dao->getSeqNum()) << " advertising " << reachableDest << endl;

    numDAOSent++; //CL 2021-08-05
    DAOsent.record(numDAOSent); //CL 2021-08-05

    return dao;
}

bool Rpl::isUdpSink() {
    if (udpApp)
        try {
            auto udpSink = check_and_cast<UdpSink*> (udpApp);
            return true;
        }
        catch (...) {}

    return false;
}

void Rpl::processDio(const Ptr<const Dio>& dio1)
//void Rpl::processDio(Ptr<Dio>& dio) //CL: I removed the const to be able to add a new field into the DIO
{
    if (isRoot){
        countNeighbours(dio1);
        return;
    }

    EV_DETAIL << "Processing DIO from: " << dio1->getSrcAddress() << endl; //CL
    EV_DETAIL << "DIO sender Id: " << dio1->getNodeId() << endl;
    EV_DETAIL << "Num of DIO from this sender: " << dio1->getNumDIO() << endl; //CL:to check the parameter value
    //updateCounterCache(dio); //CL
    EV_INFO << "HC value of this sender: " << dio1->getHC() << endl;   //CL: 2021-12-02
    EV_INFO << "ETX value of this sender: " <<dio1->getETX() << endl;
    //EV_INFO << "frame dropped by this sender: " <<dio->getDROP() << endl; //CL: 2021-12-02
    //EV_INFO << "total frames dropped by this sender: " <<dio->getDropT() << endl; //CL: 2022-02-16
    EV_INFO << "frame dropped(backoff) by this sender: " <<dio1->getDropB() << endl; //CL: 2022-02-16
    EV_INFO << "frame dropped(retry) by this sender: " <<dio1->getDropR() << endl; //CL: 2022-02-16
    EV_INFO << "total ACKs missed by this sender: " <<dio1->getMissACK() << endl; //CL: 2022-02-16
    EV_INFO << "total frames transmitted by this sender: " <<dio1->getTxF() << endl; //CL: 2022-02-16
    EV_INFO << "total frames received by this sender: " <<dio1->getRxF() << endl; //CL: 2022-02-16
    EV_INFO << "frame per seconds of this sender: " << dio1->getFps() << endl; //2022-05-05
    EV_INFO << "channel utilization of this sender: " << dio1->getBw() << endl;
    EV_INFO << "SNR that comes with this sender: " << dio1->getSnr() << endl; //2022-09-14
    EV_INFO << "Unsuccessful packets that this sender has received: " << dio1->getRx_un_suc() << endl; //2022-09-29
    EV_INFO << "Packet successful rate of this sender: " << dio1->getRx_suc_rate() << endl; //2022-09-29
    EV_INFO << "Path cost that is advertised by this sender: " << dio1->getPath_cost() << endl; //2022-10-17

    //Here I have to get the value of the SNR of the dio I just received. 2022-09-12
    //auto radio = check_and_cast<Radio *>(host->getSubmodule("wlan",0)->getSubmodule("radio")); //CL: 2022-09-13
    //EV_INFO << " SNR at receiving this sender: " << radio->snr_L1 << endl;
    //Here, I changed this part 10/30/2022, because I want to get the average SNR of this sender.
    macModule = getParentModule()->getSubmodule("wlan",0)->getSubmodule("mac");
    mac = check_and_cast<Ieee802154Mac *>(macModule);
    //uint64_t nodeId = dio->getNodeId();
    double ave_snr = mac->getSNRave(dio1->getNodeId()); //I am going to call a method defined in mac layer to get the ave SNR.
    EV_INFO << " AVE SNR at receiving this sender: " << ave_snr << endl;

    //I should add to this DIO a new field to put in the SNR value:
    Dio *dio_copy;
    dio_copy = dio1->dup();
    EV_INFO << "SNR from dio_copy: " << dio_copy->getSnr() << endl ;
    //dio_copy->setSnr(radio->snr_L1);
    dio_copy->setSnr(ave_snr);
    EV_INFO << "SNR from dio_copy after modifying SNR: " << dio_copy->getSnr() << endl ;
    auto dio = makeShared<Dio>();
    dio = dio_copy->dup();
    EV_INFO << "AVE SNR after changing the value in the original DIO: " << dio->getSnr() << endl ;

    //method to count DIOs from different nodes to know how many neighbours a node has
    countNeighbours(dio);

    emit(dioReceivedSignal, dio->dup());

    // If node's not a part of any DODAG, join the first one advertised
    if (dodagId == Ipv6Address::UNSPECIFIED_ADDRESS && dio->getRank() != INF_RANK)
    {
        dodagId = dio->getDodagId();
        dodagVersion = dio->getDodagVersion();
        instanceId = dio->getInstanceId();
        storing = dio->getStoring();
        dtsn = dio->getDtsn();
        lastTarget = new Ipv6Address(getSelfAddress());
        selfAddr = getSelfAddress();
        dodagColor = dio->getColor();
        EV_DETAIL << "Joined DODAG with id - " << dodagId << endl;
        purgeRoutingTable();
        updateMetrics_fromDIO(dio);  //CL 2021-12-02
        preferredParent = dio->dup(); // 2022-04-29: if node is not part of the DODAG, the sender of the first DIO is
                                      // is going to be the preferred parent. Also to avoid the error regarding to
                                      // updatePreferredParent method when the preferred parent is empty at the
                                      // begining
        // Start broadcasting DIOs, diffusing DODAG control data, TODO: refactor TT lifecycle
        if (trickleTimer->hasStarted())
            trickleTimer->reset();
        else
            trickleTimer->start(false, par("numSkipTrickleIntervalUpdates").intValue());
        //if (udpApp && !isUdpSink() && udpApp->par("destAddresses").str().empty()) //commented by CL 2021-11-19
            //udpApp->par("destAddresses") = dio->getDodagId().str();               //commented by CL 2021-11-19
    }
    else {
        if (!allowDodagSwitching && dio->getDodagId() != dodagId) {
            EV_DETAIL << "Node already joined a DODAG, skipping DIO advertising other ones" << endl;
            return;
        }
        /**
         * If INFINITE_RANK is advertised from a preferred parent (route poisoning),
         * delete preferred parent and remove it from candidate neighbors
         */
        if (checkPoisonedParent(dio)) {
            EV_DETAIL << "Received poisoned DIO from preferred parent - "
                    << preferredParent->getSrcAddress() << endl;
            deletePrefParent(true);
            updatePreferredParent();
            return;
        }
    }
    //trickleTimer->ctrlMsgReceived();     //2022-05-13 Here is where redundancy constant is counted, and
                                         // in RplDefs.h is the value of the constant, 10 according to RFC
    EV_INFO << "ctrlMsgReceived: " << (trickleTimer->getCtrlMsgReceived()) + 0 << endl;  //2022-05-12

//    // Do not process DIO from unknown DAG/RPL instance, TODO: check with RFC
//    if (checkUnknownDio(dio)) {
//        EV_DETAIL << "Unknown DODAG/InstanceId, or receiver is root - discarding DIO" << endl;
//        return;
//    }
    auto dioSender = dio->getSrcAddress();

    //to test python embbeded
    //Py_Initialize();
       //PyRun_SimpleString("print('Hello World from Embedded Python!!!')");
    //Py_Finalize();

    //connecting(); //2022-07-06: This method was created for testing socket programing only

//1. DIO sender is in the parent list?
    dio_received = dio_received + 1 ; //to count DIOs received
    EV_INFO << "Checking if DIO sender is in the parent list: " << endl;
    if (candidateParents.find(dioSender) != candidateParents.end()){
        //EV_DETAIL << "Candidate parent entry updated - " << dioSender << endl; //verify how the entry is updated
        EV_INFO << "DIO sender is in the parent list: " << endl;
        temp_rank = objectiveFunction->calcTemp_Rank(dio);
        EV_INFO << "my temp rank is:" << temp_rank << endl;
        //2. Is the dio sender the preferred parent?
        //if (!(checkPrefParentChanged(dio->getSrcAddress()))){
        if (checkPrefParentChanged(dio->getSrcAddress(),preferredParent->getSrcAddress())){
            EV_INFO << "DIO sender is my preferred parent: " << endl;
            if (temp_rank > rank){
                //EV_INFO << "Preferred parent advertising a worse rank, forward this DIO" << endl;
                EV_INFO << "My rank respect to my Preferred parent has changed (it is worse now)" << endl;  //CL 2022-02-02
                //This could happened due to a change in my parent rank, a change in the link conditions or both at the same time
                c = c + 1;  //CL 2022-01-31
                updateNeighbour(dio); // CL 2022-02-02
                updatePreferredParent();
                //updateMetrics   I should pass to this function the current preferred parent
                updateMetrics_fromPrefParent(preferredParent);
            }else if (temp_rank == rank){
                //EV_INFO << "Preferred parent advertising the same rank, discarding DIO" << endl;
                EV_INFO << "My rank respect to my Preferred parent has not changed" << endl;  //CL 2022-02-02
                c1 = c1 + 1;  //CL 2022-01-31
                //2022-03-08: I need to add this b/c the DIO is carried dynamic metrics.
                updateNeighbour(dio);
                preferredParent = dio->dup(); //Just copy the DIO
                //updateMetrics_fromDIO(dio); //this works hear, and also in the next else, update directly from DIO b/c is my pref parent
                updateMetrics_fromPrefParent(preferredParent);
                updatePreferredParent(); //2022-10-17: now the pref parent is related to the path cost, so even if the
                //rank did not change, the path cost could have deteriorated.
                //EV_INFO << "txF of my pref parent: " << preferredParent->getTxF() << endl;
                return;
            }else{
                //EV_INFO << "Preferred parent advertising a better rank, forward this DIO" << endl;
                EV_INFO << "My rank respect to my Preferred parent has changed (it is better now)" << endl;  //CL 2022-02-02
                updateNeighbour(dio); // CL 2022-02-02
                preferredParent = dio->dup(); //Just copy the DIO
                //updateMetrics
                updateMetrics_fromPrefParent(preferredParent);
                c2 = c2 + 1;  //CL 2022-01-31
            }
        }else{
            EV_INFO << "DIO sender is in the parent list, but it is not my preferred parent: " << endl;
            if ( temp_rank > rank ){
                //EV_INFO << "A node in the parent list advertising a worse rank, just update the parent list" << endl;
                EV_INFO << "My rank respect to a node in the parent list has changed (it is worse)" << endl;  //CL 2022-02-02
                c3 = c3 + 1;  //CL 2022-01-31
                updateNeighbour(dio); // CL 2022-02-02
            }else if (temp_rank == rank ){
                //EV_INFO << "A node in the parent list advertising a rank equal to it was, nothing to do" << endl;
                EV_INFO << "My rank respect to a node in the parent list is the same as it was, nothing to do" << endl;  //CL 2022-02-02
                //But update info of this node anyway:
                updateNeighbour(dio);
                c4 = c4 + 1;  //CL 2022-01-31
                //But for the new OF this node can be the new father, I need to check it
                //I should check which OF I am using
                std::string OF = par("objectiveFunctionType").stdstringValue(); //CL 2021-11-02  0:ETX 1:HOP_COUNT 2:Energy
                if (OF == "HC_MOD" or OF == "RPL_ENH"){
                    updatePreferredParent();  //2022-05-04
                    updateMetrics_fromPrefParent(preferredParent); //2022-05-04
                    return;
                }
                return;
            }else{
                //EV_INFO << "A node in the parent list advertising a better rank, check if it can be the new preferred parent" << endl;
                EV_INFO << "My rank respect to a node in the parent list has changed (it is better)" << endl;  //CL 2022-02-02
                updateNeighbour(dio); // CL 2022-02-02
                updatePreferredParent();
                updateMetrics_fromPrefParent(preferredParent); //here i need that works well
                c5 = c5 + 1;  //CL 2022-01-31
            }
        }


    }else{
        //Dio sender is not in parent list
        EV_INFO << "Dio sender is not in parent list " << endl;
        temp_rank = objectiveFunction->calcTemp_Rank(dio);
        EV_INFO << "temporal rank based on this dio sender: "<< temp_rank << endl;
        EV_INFO << "current rank: " << rank << endl;
        if ((temp_rank)>rank){
            EV_DETAIL << "Higher rank advertised, discarding DIO" << endl;
            c7 = c7 + 1;
            return;
        }else{
            EV_INFO << "I'll add a new neighbor as a candidate parent" << endl ;
            c6 = c6 + 1 ;
            addNeighbour(dio);  //When I do this I have to save all the metrics this sender is advertising
        }
    }
//    if (dio->getRank() > rank) {   //for ETX OF I have to jump this or do this: >rank-1 & first time received otherwise no bc could be the pref parent or another node already in that worst
//        EV_DETAIL << "Higher rank advertised, discarding DIO" << endl;
//        return;
//    }

//    addNeighbour(dio);
//    updatePreferredParent();

}

void Rpl::purgeRoutingTable() {
    auto numRoutes = routingTable->getNumRoutes();
    for (auto i = 0; i < numRoutes; i++)
        routingTable->deleteRoute(routingTable->getRoute(i));
}

bool Rpl::checkPoisonedParent(const Ptr<const Dio>& dio) {
    return preferredParent && dio->getRank() == INF_RANK && preferredParent->getSrcAddress() == dio->getSrcAddress();
}

void Rpl::processDao(const Ptr<const Dao>& dao) {
    if (!daoEnabled) {
        EV_WARN << "DAO support not enabled, discarding packet" << endl;
        return;
    }

    emit(daoReceivedSignal, dao->dup());
    auto daoSender = dao->getSrcAddress();
    auto advertisedDest = dao->getReachableDest();
    EV_DETAIL << "Processing DAO with seq num " << std::to_string(dao->getSeqNum()) << " from "
            << daoSender << " advertising " << advertisedDest << endl;

    if (dao->getDaoAckRequired()) {
        sendRplPacket(createDao(advertisedDest), DAO_ACK, daoSender, uniform(1, 3));
        EV_DETAIL << "DAO_ACK sent to " << daoSender
                << " acknowledging advertised dest - " << advertisedDest << endl;
    }

    /**
     * If a node is root or operates in storing mode
     * update routing table with destinations from DAO [RFC6560, 3.3].
     * TODO: Implement DAO aggregation!
     */
    if (storing || isRoot) {
        if (!checkDestKnown(daoSender, advertisedDest)) {
            updateRoutingTable(daoSender, advertisedDest, prepRouteData(dao.get()));

            EV_DETAIL << "Destination learned from DAO - " << advertisedDest
                    << " reachable via " << daoSender << endl;
        }
        else
            return;
    }
    /**
     * Forward DAO 'upwards' via preferred parent advertising destination to the root [RFC6560, 6.4]
     */
    if (!isRoot && preferredParent) {
        if (!storing)
            sendRplPacket(createDao(advertisedDest), DAO,
                preferredParent->getSrcAddress(), daoDelay * uniform(1, 2), *lastTarget, *lastTransit);
        else
            sendRplPacket(createDao(advertisedDest), DAO,
                preferredParent->getSrcAddress(), daoDelay * uniform(1, 2));

        numDaoForwarded++;
        EV_DETAIL << "Forwarding DAO to " << preferredParent->getSrcAddress()
                << " advertising " << advertisedDest << " reachability" << endl;
    }
}

std::vector<Ipv6Address> Rpl::getNearestChildren() {
    auto prefParentAddr = preferredParent ? preferredParent->getSrcAddress() : Ipv6Address::UNSPECIFIED_ADDRESS;
    std::vector<Ipv6Address> neighbrs = {};
    for (auto i = 0; i < routingTable->getNumRoutes(); i++) {
        auto rt = routingTable->getRoute(i);
        auto dest = rt->getDestPrefix();
        auto nextHop = rt->getNextHop();

        if (dest == nextHop && dest != prefParentAddr)
            neighbrs.push_back(nextHop);
    }

    EV_DETAIL << "Found 1-hop neighbors (" << neighbrs.size() << ") - " << neighbrs << endl;
    return neighbrs;
}

void Rpl::processDaoAck(const Ptr<const Dao>& daoAck) {
    auto advDest = daoAck->getReachableDest();

    EV_INFO << "Received DAO_ACK from " << daoAck->getSrcAddress()
            << " for advertised dest - "  << advDest << endl;

    if (pendingDaoAcks.empty()) {
        EV_DETAIL << "No DAO_ACKs were expected!" << endl;
        return;
    }

    clearDaoAckTimer(advDest);

    EV_DETAIL << "Cancelled timeout event and erased entry in the pendingDaoAcks, remaining: " << endl;

    for (auto e : pendingDaoAcks)
        EV_DETAIL << e.first << " rtxs: " << std::to_string(e.second.second)
            << ", msg ptr - " << e.second.first << endl;

}

void Rpl::drawConnector(Coord target, cFigure::Color col) {
    // (0, 0) corresponds to default Coord constructor, meaning no target position was provided
    if ((!target.x && !target.y) || !par("drawConnectors").boolValue())
        return;

    cCanvas *canvas = getParentModule()->getParentModule()->getCanvas();
    if (prefParentConnector) {
        prefParentConnector->setLineColor(col);
        prefParentConnector->setEnd(cFigure::Point(target.x, target.y));
        return;
    }

    prefParentConnector = new cLineFigure("preferredParentConnector");
    prefParentConnector->setStart(cFigure::Point(position.x, position.y));
    prefParentConnector->setEnd(cFigure::Point(target.x, target.y));
    prefParentConnector->setLineWidth(2);
    prefParentConnector->setLineColor(col);
    prefParentConnector->setLineOpacity(0.6);
    prefParentConnector->setEndArrowhead(cFigure::ARROW_BARBED);
    prefParentConnector->setVisible(true);
    canvas->addFigure(prefParentConnector);
}

void Rpl::updatePreferredParent()
{
    //2022-04-11: To be used later to see if the rank changed
    double current_rank = rank;

    Dio *newPrefParent;
    EV_DETAIL << "Choosing preferred parent from "
            << boolStr(candidateParents.empty() && par("useBackupAsPreferred").boolValue(),
                    "backup", "candidate") << " parent set" << endl;
    /**
     * Choose parent from candidate neighbor set. If it's empty, leave DODAG.
     */
    if (candidateParents.empty())
        if (par("useBackupAsPreferred").boolValue())
            newPrefParent = objectiveFunction->getPreferredParent(backupParents, preferredParent); //back to this on 22-04-29
            //newPrefParent = objectiveFunction->getPreferredParent(backupParents, preferredParent); //2022-03-08
        else {
            detachFromDodag();
            return;
        }
    else
        //newPrefParent = objectiveFunction->getPreferredParent(candidateParents);
        newPrefParent = objectiveFunction->getPreferredParent(candidateParents, preferredParent); //2022-03-08...2022-04-29

    auto newPrefParentAddr = newPrefParent->getSrcAddress();
    /**
     * If a better preferred parent is discovered (based on the objective function metric),
     * update default route to DODAG sink with the new nextHop
     */
    if (!(checkPrefParentChanged(newPrefParentAddr,previous_PrefParentAddr))) {  //2022-03-08

        //CL: if it is here is because the preferred parent changed otherwise it was only updated
        std::ofstream file;  // CL 2022-02-02
        file.open("St_PrefParentChanges.txt", std::ios_base::app);
        file << simTime() <<" " << getParentModule()->getFullName() << "The preferred parent is another node" << endl;
        //

        auto newPrefParentDodagId = newPrefParent->getDodagId();

        /** Silently join new DODAG and update dest address for application, TODO: Check with RFC */
        dodagId = newPrefParentDodagId;
        //if (udpApp && !isUdpSink() && udpApp->par("destAddresses").str().empty()) //commented by CL 2021-11-19
            //udpApp->par("destAddresses") = newPrefParentDodagId.str();            //commented by CL

        daoSeqNum = 0;
        clearParentRoutes();
        drawConnector(newPrefParent->getPosition(), newPrefParent->getColor());
        updateRoutingTable(newPrefParentAddr, dodagId, nullptr, true);

        // required for proper nextHop address resolution
        if (newPrefParentAddr != dodagId)
            updateRoutingTable(newPrefParentAddr, newPrefParentAddr, nullptr, false);

        lastTransit = new Ipv6Address(newPrefParentAddr);
        EV_DETAIL << "Updated preferred parent to - " << newPrefParentAddr << endl;
        numParentUpdates++;
        /**
         * Reset trickle timer due to inconsistency (preferred parent changed) detected, thus
         * maintaining higher topology reactivity and convergence rate [RFC 6550, 8.3]
         */

        //trickleTimer->reset();  //2022-05-15

        /**
         * OF based on ETX needs advertise the change of preferred parent because the rank of the other nodes depends on that, so there is a need to reset the trickleTimer
         * to advertise the change. For RPL_ENH, if the node does not change the rank value and just changes to a pref parent with the same ranking, but better path_cost, I am thinking
         * to avoid resetting the trickleTimer to reduce the amount of DIO sent.         *
         */
        std::string OF = par("objectiveFunctionType").stdstringValue(); //CL 2022-11-04
        //EV_INFO << "I am using OF:" << OF << endl;
        if (OF == "ETX")
            trickleTimer->reset();
        else if (OF == "HOP_COUNT")
            trickleTimer->reset();
        else if (OF == "HC_MOD")
            trickleTimer->reset();
        else if (OF == "ML"){
            if(simTime()<1740)
                trickleTimer->reset();
            else
                sendRplPacket(createDio_Unique(), DIO, Ipv6Address::ALL_NODES_1, uniform(0, 1));  //trickleTimer->reset();
        }else if (OF=="RPL_ENH"){
            if (newPrefParent->getRank() + 1 == current_rank)
                EV_INFO << "do nothing if the rank of my pref parent did not change, and only changed the path_cost" << endl;
            else
                trickleTimer->reset();
        }else
            trickleTimer->reset();


        if (daoEnabled) {
            if (storing)
                // TODO: magic numbers
                sendRplPacket(createDao(), DAO, newPrefParentAddr, daoDelay * uniform(1, 7));
            else
                sendRplPacket(createDao(), DAO, newPrefParentAddr, daoDelay * uniform(1, 7),
                            getSelfAddress(), newPrefParentAddr);

            EV_DETAIL << "Sending DAO to new pref. parent - " << newPrefParentAddr
                    << " advertising " << getSelfAddress() << " reachability" << endl;
        }
    }
    preferredParent = newPrefParent->dup();
    previous_PrefParentAddr = preferredParent->getSrcAddress();  //2022-03-08

    /** Recalculate rank based on the objective function */
    rank = objectiveFunction->calcRank(preferredParent);
    EV_DETAIL << "My current Rank: " << rank << endl;
    //EV_DETAIL << "My HC value: " << getHC() << endl;
    //EV_DETAIL << "My ETX value: " << getETX() << endl;
}

//bool Rpl::checkPrefParentChanged(const Ipv6Address &newPrefParentAddr)  2022-03-08
bool Rpl::checkPrefParentChanged(const Ipv6Address &newPrefParentAddr , const Ipv6Address &previous_PrefParentAddr)
{
    //return !preferredParent || preferredParent->getSrcAddress() != newPrefParentAddr;
    if (newPrefParentAddr == previous_PrefParentAddr){
        EV_INFO << "preferred parent did not change" << endl;
        return true;
    }else
        EV_INFO << "preferred parent changed" << endl;
        return false;
}

//
// Handling routing data    previous_PrefParentAddr
//

RplRouteData* Rpl::prepRouteData(const Dao *dao) {
    auto routeData = new RplRouteData();
    routeData->setDodagId(dao->getDodagId());
    routeData->setInstanceId(dao->getInstanceId());
    routeData->setDtsn(dao->getSeqNum());
    routeData->setExpirationTime(-1);
    return routeData;
}

void Rpl::updateRoutingTable(const Ipv6Address &nextHop, const Ipv6Address &dest, RplRouteData *routeData, bool defaultRoute)
{
    auto route = routingTable->createRoute();
    route->setSourceType(IRoute::MANET);
    route->setPrefixLength(isRoot ? 128 : prefixLength);
    route->setInterface(interfaceEntryPtr);
    route->setDestination(dest);
    route->setNextHop(nextHop);
    /**
     * If a route through preferred parent is being added
     * (i.e. not downward route, learned from DAO), set it as default route
     */
    if (defaultRoute) {
        routingTable->addDefaultRoute(nextHop, interfaceEntryPtr->getInterfaceId(), DEFAULT_PARENT_LIFETIME);
        EV_DETAIL << "Adding default route via " << nextHop << endl;
    }
    if (routeData)
        route->setProtocolData(routeData);

    if (!checkDuplicateRoute((Ipv6Route*) route))
        routingTable->addRoute(route);
}

bool Rpl::checkDuplicateRoute(Ipv6Route *route) {
    for (auto i = 0; i < routingTable->getNumRoutes(); i++) {
        auto rt = routingTable->getRoute(i);
        auto rtdest = rt->getDestinationAsGeneric().toIpv6();
        auto dest = route->getDestinationAsGeneric().toIpv6();
        if (dest == rtdest) {
            if (rt->getNextHop() != route->getNextHop()) {
                rt->setNextHop(route->getNextHop());
                EV_DETAIL << "Duplicate route, updated next hop to " << rt->getNextHop() << " for dest " << dest << endl;
                if (route->getProtocolData())
                    rt->setProtocolData(route->getProtocolData());
            }
            return true;
        }
    }
    return false;
}

void Rpl::appendRplPacketInfo(Packet *datagram) {
    auto rpi = makeShared<RplPacketInfo>();
    rpi->setChunkLength(B(4));
    rpi->setDown(isRoot || packetTravelsDown(datagram));
    rpi->setRankError(false);
    rpi->setFwdError(false);
    rpi->setInstanceId(instanceId);
    rpi->setSenderRank(rank);
    datagram->insertAtBack(rpi);
    EV_INFO << "Appended RPL Packet Information: \n" << printHeader(rpi.get())
            << "\n to UDP datagram: " << datagram << endl;
}

bool Rpl::packetTravelsDown(Packet *datagram) {
    auto networkProtocolHeader = findNetworkProtocolHeader(datagram);
    auto dest = networkProtocolHeader->getDestinationAddress().toIpv6();
    EV_DETAIL << "Determining packet forwarding direction:\n destination - " << dest << endl;
    auto ri = routingTable->doLongestPrefixMatch(dest);
    if (ri == nullptr) {
        auto errorMsg = std::string("Error while determining packet forwarding direction"
                + std::string(", couldn't find route to ") + dest.str());
        throw cRuntimeError(errorMsg.c_str());
    }

    EV_DETAIL << " next hop - " << ri->getNextHop() << endl;
    bool res = sourceRouted(datagram)
            || !(ri->getNextHop().matches(preferredParent->getSrcAddress(), prefixLength));
    EV_DETAIL << " Packet travels " << boolStr(res, "downwards", "upwards");
    return res;
}


bool Rpl::sourceRouted(Packet *pkt) {
    EV_DETAIL << "Checking if packet is source-routed" << endl;;
    try {
        auto srh = pkt->popAtBack<SourceRoutingHeader>(B(64));
        EV_DETAIL << "Retrieved source-routing header - " << srh << endl;
        return true;
    }
    catch (std::exception &e) { }

    return false;
}

B Rpl::getDaoLength() {
    return B(8);
}


void Rpl::appendDaoTransitOptions(Packet *pkt) {
    appendDaoTransitOptions(pkt, getSelfAddress(), preferredParent->getSrcAddress());
}

void Rpl::appendDaoTransitOptions(Packet *pkt, const Ipv6Address &target, const Ipv6Address &transit) {
    EV_DETAIL << "Appending target, transit options to DAO: " << pkt << endl;
    auto rplTarget = makeShared<RplTargetInfo>();
    auto rplTransit = makeShared<RplTransitInfo>();
    rplTarget->setChunkLength(getTransitOptionsLength());
    rplTransit->setChunkLength(getTransitOptionsLength());
    rplTarget->setTarget(target);
    rplTransit->setTransit(transit);
    pkt->insertAtBack(rplTarget);
    pkt->insertAtBack(rplTransit);
    EV_DETAIL << "transit => target headers appended: "
            << rplTransit->getTransit() << " => " << rplTarget->getTarget() << endl;
}


bool Rpl::checkRplRouteInfo(Packet *datagram) {
    auto dest = findNetworkProtocolHeader(datagram)->getDestinationAddress().toIpv6();
    if (dest.matches(getSelfAddress(), prefixLength)) {
        EV_DETAIL << "Packet reached its destination, no RPI header checking needed" << endl;
        return true;
    }
    EV_DETAIL << "Checking RPI header for packet " << datagram
            << " \n coming from "
            << findNetworkProtocolHeader(datagram)->getSourceAddress().toIpv6()
            << endl;
    RplPacketInfo *rpi;
    try {
        rpi = const_cast<RplPacketInfo *> (datagram->popAtBack<RplPacketInfo>(B(4)).get());
    }
    catch (std::exception &e) {
       EV_WARN << "No RPL Packet Information present in UDP datagram, appending" << endl;
       try {
           appendRplPacketInfo(datagram);
           return true;
       }
       catch (...) {
           return false;
       }
    }
    if (isRoot)
        return true;
    // check for rank inconsistencies
    EV_DETAIL << "Rank error before checking - " << rpi->getRankError() << endl;
    bool rankInconsistency = checkRankError(rpi);
    if (rpi->getRankError() && rankInconsistency) {
        EV_WARN << "Repeated rank error detected for packet "
                << datagram << "\n dropping..." << endl;
        return false;
    }
    if (rankInconsistency)
        rpi->setRankError(rankInconsistency);
    EV_DETAIL << "Rank error after check - " << rpi->getRankError() << endl;
    /**
     * If there's a forwarding error, packet should be returned to parent
     * with 'F' flag set to clear outdated DAO routes if DAO inconsistency detection
     * is enabled [RFC 6550, 11.2.2.3]
     */
    rpi->setFwdError(!isRoot && checkForwardingError(rpi, dest));
    if (rpi->getFwdError()) {
        EV_WARN << "Forwarding error detected for packet " << datagram
                << "\n destined to " << dest << ", dropping" << endl;
        return false;
    }
    // update packet forwarding direction if storing mode is in use
    // e.g. if unicast P2P packet reaches sub-dodag root with 'O' flag cleared,
    // and this root can route packet downwards to destination, 'O' flag has to be set.
    if (storing)
        rpi->setDown(isRoot || packetTravelsDown(datagram));
    // try make new shared ptr chunk RPI, to be refactored into reusing existing RPI object
    auto rpiCopy = makeShared<RplPacketInfo>();
    rpiCopy->setChunkLength(getRpiHeaderLength());
    rpiCopy->setDown(rpi->getDown());
    rpiCopy->setRankError(rpi->getRankError());
    rpiCopy->setFwdError(rpi->getFwdError());
    rpiCopy->setInstanceId(instanceId);
    rpiCopy->setSenderRank(rank);
    datagram->insertAtBack(rpiCopy);
    return true;
}

B Rpl::getTransitOptionsLength() {
    return B(16);  //B(16)
}

B Rpl::getRpiHeaderLength() {
    return B(16); //B(4)
}


void Rpl::extractSourceRoutingData(Packet *pkt) {
    try {
        lastTransit = new Ipv6Address(pkt->popAtBack<RplTransitInfo>(getTransitOptionsLength()).get()->getTransit());
        lastTarget = new Ipv6Address(pkt->popAtBack<RplTargetInfo>(getTransitOptionsLength()).get()->getTarget());
        if (!isRoot)
            return;

        sourceRoutingTable.insert( std::pair<Ipv6Address, Ipv6Address>(*lastTarget, *lastTransit) );
        EV_DETAIL << "Source routing table updated with new:\n"
                << "target: " << lastTarget << "\n transit: " << lastTransit << "\n"
                << printMap(sourceRoutingTable) << endl;
    }
    catch (std::exception &e) {
        EV_WARN << "Couldn't pop RPL Target, Transit Information options from packet: "
                << pkt << endl;
    }
}

INetfilter::IHook::Result Rpl::checkRplHeaders(Packet *datagram) {
    // skip further checks if node doesn't belong to a DODAG
    auto datagramName = std::string(datagram->getFullName());
    EV_INFO << "packet fullname " << datagramName << endl;
    if (!isRoot && (preferredParent == nullptr || dodagId == Ipv6Address::UNSPECIFIED_ADDRESS))
    {
        EV_DETAIL << "Node is detached from a DODAG, " <<
                " no forwarding/rank error checks will be performed" << endl;
        return ACCEPT;
    }
    EV_INFO << "before of: if(isUdp(datagram)) "<< endl;
    if (isUdp(datagram) or isPing(datagram)) {
        // in non-storing MOP source routing header is needed for downwards traffic
        EV_INFO << "before of: if (!storing)) "<< endl;
        if (!storing) {
            // generate one if the sender is root
            if (isRoot) {
                if (selfGeneratedPkt(datagram)) {
                    appendSrcRoutingHeader(datagram);
                    return ACCEPT;
                }
                else
                    EV_DETAIL << "P2P source-routing not yet supported" << endl;
            }
            // or forward packet further using the routing header
            else {
                if (!destIsRoot(datagram))
                    forwardSourceRoutedPacket(datagram);
                return ACCEPT;
            }
        }

        // check for loops
//        return checkRplRouteInfo(datagram) ? ACCEPT : DROP;
    }
    else{
       EV_INFO << "I didn't enter in: if(isUdp(datagram)) "<< endl;
    }

    return ACCEPT;
}


bool Rpl::destIsRoot(Packet *datagram) {
    return findNetworkProtocolHeader(datagram).get()->getDestinationAddress().toIpv6().matches(dodagId, prefixLength);
}

void Rpl::saveDaoTransitOptions(Packet *dao) {
    try {
        lastTransit = new Ipv6Address(dao->popAtBack<RplTransitInfo>(getTransitOptionsLength()).get()->getTransit());
        lastTarget = new Ipv6Address(dao->popAtBack<RplTargetInfo>(getTransitOptionsLength()).get()->getTarget());
        EV_DETAIL << "Updated lastTransit => lastTarget to: " << *lastTransit << " => " << *lastTarget << endl;
    }
    catch (std::exception &e) {
        EV_DETAIL << "No Target, Transit headers found on packet:\n " << *dao << endl;
        return;
    }
}

void Rpl::constructSrcRoutingHeader(std::deque<Ipv6Address> &addressList, Ipv6Address dest)
{
    Ipv6Address nextHop = sourceRoutingTable[dest];
    addressList.push_front(dest);
    addressList.push_front(nextHop);
    EV_DETAIL << "Constructing routing header for dest - " << dest << "  next hop: " << nextHop << endl;

    // Create sequence of 'next hop' addresses recursively using the source routing table learned from DAOs
    while (sourceRoutingTable.find(nextHop) != sourceRoutingTable.end())
    {
        nextHop = sourceRoutingTable[nextHop];
        addressList.push_front(nextHop);
    }
    // pop redundant link-local next hop, which is already known from the routing table
    addressList.pop_front();
}


void Rpl::appendSrcRoutingHeader(Packet *datagram) {
    Ipv6Address dest = findNetworkProtocolHeader(datagram)->getDestinationAddress().toIpv6();
    std::deque<Ipv6Address> srhAddresses;
    EV_DETAIL << "Appending routing header to datagram " << datagram << endl;
    if ( sourceRoutingTable.find(dest) == sourceRoutingTable.end() ) {
        EV_WARN << "Required destination " << dest << " not yet present in source-routing table: \n"
                << printMap(sourceRoutingTable) << endl;
        return;
    }

    constructSrcRoutingHeader(srhAddresses, dest);

    EV_DETAIL << "Source routing header constructed : " << srhAddresses << endl;

    auto srh = makeShared<SourceRoutingHeader>();
    srh->setAddresses(srhAddresses);
    srh->setChunkLength(getSrhSize());
    datagram->insertAtBack(srh);
}

void Rpl::forwardSourceRoutedPacket(Packet *datagram) {
    EV_DETAIL << "processing source-routed datagram - " << datagram
            << "\n with routing header: " << endl;
    auto srh = const_cast<SourceRoutingHeader *> (datagram->popAtBack<SourceRoutingHeader>(getSrhSize()).get());
    EV_INFO << "I'm here" << endl;
    auto srhAddresses = srh->getAddresses();

    if (srhAddresses.back() == getSelfAddress()) {
        EV_DETAIL << "Source-routed destination reached" << endl;
        return;
    }

    for (auto addr : srhAddresses)
        EV_DETAIL << addr << " => ";

    Ipv6Address nextHop;
    for (auto it = srhAddresses.cbegin(); it != srhAddresses.cend(); it++) {
        if (it->matches(getSelfAddress(), prefixLength)) {
            nextHop = *(++it);
            break;
        }
    }
    EV_DETAIL << "\n Forwarding source-routed datagram to " << nextHop << endl;

    updateRoutingTable(nextHop, nextHop, nullptr, false);

    srhAddresses.pop_front();
//
    // re-insert updated routing header
    auto updatedSrh =  makeShared<SourceRoutingHeader>();
    updatedSrh->setChunkLength(getSrhSize());
    updatedSrh->setAddresses(srhAddresses);
    datagram->insertAtBack(updatedSrh);
    (const_cast<NetworkHeaderBase *>(findNetworkProtocolHeader(datagram).get()))->setDestinationAddress(nextHop);
}

bool Rpl::selfGeneratedPkt(Packet *pkt) {
    bool res;
    if (isRoot) {
        auto dest = findNetworkProtocolHeader(pkt)->getDestinationAddress().toIpv6();
        res = !(dest.matches(getSelfAddress(), prefixLength));
        EV_DETAIL << "Checking if packet is self generated by dest: " << dest << endl;
    }
    else {
        auto srcAddr = findNetworkProtocolHeader(pkt)->getSourceAddress().toIpv6();
        res = srcAddr.matches(getSelfAddress(), prefixLength);
        EV_DETAIL << "Checking if packet is self generated by source address: "
                << srcAddr << "; is self-generated: " << boolStr(res) << endl;
    }
    return res;
}

std::string Rpl::rplIcmpCodeToStr(RplPacketCode code) {
    switch (code) {
        case 0:
            return std::string("DIS");
        case 1:
            return std::string("DIO");
        case 2:
            return std::string("DAO");
        case 3:
            return std::string("DAO_ACK");
        default:
            return std::string("Unknown");
    }
}


bool Rpl::checkRankError(RplPacketInfo *rpi) {
    auto senderRank = rpi->getSenderRank();
    EV_DETAIL << "Checking rank consistency: "
            << "\n direction - " << boolStr(rpi->getDown(), "down", "up")
            << "\n senderRank - " << senderRank << "; own rank - " << rank << endl;
    bool res = (!(rpi->getDown()) && (senderRank <= rank))
                    || (rpi->getDown() && (senderRank >= rank));
    EV_DETAIL << "Rank consistency check " << boolStr(res, "failed", "passed") << endl;
    return res;
}

bool Rpl::checkForwardingError(RplPacketInfo *rpi, Ipv6Address &dest) {
    EV_DETAIL << "Checking forwarding error: \n MOP - "
            << boolStr(storing, "storing", "non-storing")
            << "\n dest - " << dest
            << "\n direction - " << boolStr(rpi->getDown(), "down", "up") << endl;
    auto route = routingTable->doLongestPrefixMatch(dest);
    auto parentAddr = preferredParent != nullptr ? preferredParent->getSrcAddress() : Ipv6Address::UNSPECIFIED_ADDRESS;
    bool res = storing && rpi->getDown()
                    && (route == nullptr || route->getNextHop().matches(parentAddr, prefixLength));
    EV_DETAIL << "Forwarding " << boolStr(res, "error detected", "OK") << endl;
    return res;
}

void Rpl::printTags(Packet *packet) {
    EV_DETAIL << "Packet " << packet->str() << "\n Tags: " << endl;
    for (int i = 0; i < packet->getNumTags(); i++)
        EV_DETAIL << "classname: " << packet->getTag(i)->getClassName() << endl;
}

std::string Rpl::printHeader(RplPacketInfo *rpi) {
    std::ostringstream out;
    out << " direction: " << boolStr(rpi->getDown(), "down", "up")
        << "\n senderRank:  " << rpi->getSenderRank()
        << "\n rankError: " << boolStr(rpi->getRankError())
        << "\n forwardingError: " << boolStr(rpi->getFwdError());
    return out.str();
}


void Rpl::deletePrefParent(bool poisoned)
{
    if (!preferredParent) {
        EV_WARN << "Cannot delete preferred parent, it's nullptr" << endl;
        return;
    }

    auto prefParentAddr = preferredParent->getSrcAddress();
    EV_DETAIL << "Preferred parent " << prefParentAddr
            << boolStr(poisoned, " detachment", " unreachability") << "detected" << endl;
    emit(parentUnreachableSignal, preferredParent);
    clearParentRoutes();
    candidateParents.erase(prefParentAddr);
    preferredParent = nullptr;
    EV_DETAIL << "Erased preferred parent from candidate parent set" << endl;
}

void Rpl::clearParentRoutes() {
    if (!preferredParent) {
        EV_WARN << "Pref. parent not set, cannot delete associate routes from routing table " << endl;
        return;
    }

    Ipv6Route *routeToDelete;

    EV_INFO << "interface before deleting: " << interfaceEntryPtr->getInterfaceId() << endl;
    //routingTable->printRoutingTable();
    EV_INFO << "Num routes: " << routingTable->getNumRoutes() << endl;

    routingTable->deleteDefaultRoutes(interfaceEntryPtr->getInterfaceId());
    EV_DETAIL << "Deleted default route through preferred parent " << endl;
    auto totalRoutes = routingTable->getNumRoutes();
    for (int i = 0; i < totalRoutes; i++) {
        auto ri = routingTable->getRoute(i);
        if (ri->getNextHop() == preferredParent->getSrcAddress())
            routeToDelete = ri;
    }
    if (routeToDelete)
        routingTable->deleteRoute(routeToDelete);
    EV_DETAIL << "Deleted non-default route through preferred parent " << endl;
    routingTable->purgeDestCache();
}

//
// Utility methods
//

std::string Rpl::boolStr(bool cond, std::string positive, std::string negative) {
    return cond ? positive : negative;
}

template<typename Map>
std::string Rpl::printMap(const Map& map) {
    std::ostringstream out;
    for (const auto& p : map)
        out << p.first << " => " << p.second << endl;
    return out.str();
}

Ipv6Address Rpl::getSelfAddress() {
    return interfaceEntryPtr->getNetworkAddress().toIpv6();
}

bool Rpl::checkDestKnown(const Ipv6Address &nextHop, const Ipv6Address &dest) {
    Ipv6Route *outdatedRoute = nullptr;
    for (int i = 0; i < routingTable->getNumRoutes(); i++) {
        auto ri = routingTable->getRoute(i);
        if (ri->getDestPrefix() == dest) {
            EV_DETAIL << "Destination " << ri->getDestPrefix() << " already known, ";
            if (ri->getNextHop() == nextHop) {
                EV_DETAIL << "reachable via " << ri->getNextHop() << endl;
                return true;
            }
            else {
                EV_DETAIL << "but next hop has changed to "
                        << nextHop << ", routing table to be updated" << endl;
                outdatedRoute = ri;
                break;
            }
        }
    }
    try {
        if (outdatedRoute)
            EV_DETAIL << "Deleting outdated route to dest " << dest
                    << " via " << outdatedRoute->getNextHop()
                    << " : \n "
                    << boolStr(routingTable->deleteRoute(outdatedRoute), "Success", "Fail");
    }
    catch (std::exception &e) {
        EV_WARN << "Exception while deleting outdated route " << e.what() << endl;
    }

    return false;
}

bool Rpl::checkUnknownDio(const Ptr<const Dio>& dio)
{
    return dio->getDodagId() != dodagId || dio->getInstanceId() != instanceId;
}

void Rpl::addNeighbour(const Ptr<const Dio>& dio)
{
    /**
     * Maintain following sets of link-local nodes:
     *  - backup parents
     *  - candidate parents
     * where preferred parent is chosen from the candidate parent set [RFC6560, 8.2.1]
     *
     * In current implementation, neighbor data is represented by most recent
     * DIO packet received from it.
     */
    auto dioCopy = dio->dup();
    auto dioSender = dio->getSrcAddress();
//NO    /** If DIO sender has equal rank, consider this node as a backup parent */
//    EV_INFO <<"dio Ranking = "<<dio->getRank()<<" current node ranking: "<<rank << endl;
//    if (dio->getRank() == rank) {
//        if (backupParents.find(dioSender) != backupParents.end())
//            EV_DETAIL << "Backup parent entry updated - " << dioSender << endl;
//        else
//            EV_DETAIL << "New backup parent added - " << dioSender << endl;
//        backupParents[dioSender] = dioCopy;
//    }
    /** If node's new rank based on DIO sender is going to be equal to the current rank that this node has, just add this node as a candidate parent */
//    if ((temp_rank)==rank)
//        candidateParents[dioSender] = dioCopy;
//    else  //no need to check if temp_rank is less than current rank
        /** If DIO sender has lower rank, consider this node as a candidate parent */
        candidateParents[dioSender] = dioCopy;
        updatePreferredParent();
        updateMetrics_fromPrefParent(preferredParent);

//     if (dio->getRank() < rank) {
//        if (candidateParents.find(dioSender) != candidateParents.end())
//            EV_DETAIL << "Candidate parent entry updated - " << dioSender << endl;
//        else
//            EV_DETAIL << "New candidate parent added - " << dioSender << endl;
//        candidateParents[dioSender] = dioCopy;
//    }
}

//
// Notification handling
//

void Rpl::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{

//    Enter_Method("receiveChangeNotification");
//
    Enter_Method_Silent();

    EV_DETAIL << "Processing signal - " << signalID << endl;
    if (signalID == packetReceivedSignal)
        udpPacketsRecv++;

    /**
     * Upon receiving broken link signal from MAC layer, check whether
     * preferred parent is unreachable
     */
    if (signalID == linkBrokenSignal) {
        EV_WARN << "Received link break" << endl;
        Packet *datagram = check_and_cast<Packet *>(obj);
        EV_DETAIL << "Packet " << datagram->str() << " lost?" << endl;
        const auto& networkHeader = findNetworkProtocolHeader(datagram);
        if (networkHeader != nullptr) {
            const Ipv6Address& destination = networkHeader->getDestinationAddress().toIpv6();
            EV_DETAIL << "Connection with destination " << destination << " broken?" << endl;

            /**
             * If preferred parent unreachability detected, remove route with it as a
             * next hop from the routing table and select new preferred parent from the
             * candidate set.
             *
             * If candidate parent set is empty, leave current DODAG
             * (becoming either floating DODAG or poison child routes altogether)
             */
            if (preferredParent && destination == preferredParent->getSrcAddress()
                    && par("unreachabilityDetectionEnabled").boolValue())
            {
                deletePrefParent();
                updatePreferredParent();
            }
        }
    }
}

///// CL methods  /////////////

void Rpl::finish(){

    auto radio = check_and_cast<Radio *>(host->getSubmodule("wlan",0)->getSubmodule("radio")); //CL: 2022-09-13

    //CL, to save the ranking of each node at the end of the simulation.
    std::ofstream file;
    file.open("St_Ranking.txt", std::ios_base::app);
    // meter[] rank hc etx dio_received c c1 c2 c3 c4 c5 c6 c7
    //file << getParentModule()->getFullName() << " " << rank << " " <<"path_C: " << path_cost << " "<< hc << " " << etx << " " << "dioRx: " << dio_received <<
    //        " " << c << " " << c1 << " " << c2 << " " << c3 << " " << c4 << " " << c5 << " " << c6 <<
    //        " " << c7 << " rx_uns= " << radio->rx_unsuccessful << " rx_suc_rate= " << radio->rx_successful_rate << endl;
    file << getParentModule()->getFullName() << " " << rank << " " <<"path_C: " << path_cost << " HC: " << hc << " ETX: " << etx << " " << "dioRx: " << dio_received <<
            " " << "parents-updated: " << numParentUpdates << " neighbors: " << neighbors << endl;
    //file << getParentModule()->getFullName() << "rank: "<< rank << " hc: " << hc<< " etx: " << etx << endl;
    //file << "My rank respect to my Preferred parent has changed (it is worse now): " << c << endl;
    //file << "My rank respect to my Preferred parent has not changed: " << c1 << endl;
    //file << "My rank respect to my Preferred parent has changed (it is better now): " << c2 << endl;
    //file << "My rank respect to a node in the parent list has changed (it is worse): " << c3 << endl;
    //file << "My rank respect to a node in the parent list is the same as it was, nothing to do: " << c4 << endl;
    //file << "My rank respect to a node in the parent list has changed (it is better) " << c5 << endl;
    //file << "I'll add a new neighbor as a candidate parent: " << c6 << endl;
    //file << "Neighbors: " << den << endl;

    //macModule = getParentModule()->getSubmodule("wlan",0)->getSubmodule("mac");
    //mac = check_and_cast<Ieee802154Mac *>(macModule);

    //file << "channel utilization: " << mac->channel_util  <<endl;
    //file << "queue utilization: " << mac->qu << endl;
    file.close();
}

double Rpl::getRank(){
    return rank;
}

double Rpl::getETX(){
    return etx;
}

int Rpl::getHC(){
    return hc;
}

void Rpl::setETX(double ETX){
    etx = ETX;
}

void Rpl::setHC(int HC){
    hc = HC;
}

/*
int Rpl::getdropT() {
    return dropT;
}
*/
int Rpl::getdropB() {
    return dropB;
}

int Rpl::getdropR() {
    return dropR;
}

int Rpl::getmissACK(){
    return missACK;
}

double Rpl::gettxF(){
    return txF;
}

double Rpl::getrxF(){
    return rxF;
}

double Rpl::getbw(){
    return bw;
}

int Rpl::getden(){
    return den;
}

double Rpl::getqu(){
    return qu;
}

int Rpl::getneighbors(){
    return neighbors;
}

double Rpl::getetx_int(){
    return etx_inst;
}

double Rpl::getsnr_inst(){
    return snr_inst;
}

double Rpl::getfps(){
    return fps;
}

double Rpl::getSNR(){
    return snr;
}

int Rpl::getRXuns(){
    return rxuns;
}

double Rpl::getRXsucrate(){
    return rxsucrate;
}

simtime_t Rpl::getlastupdate(){
    return last_update;
}

void Rpl::updateMetrics_fromDIO (const Ptr<const Dio>& dio){

    EV_INFO << "Updating metrics from DIO: " << endl;
    hc = dio->getHC() + 1;
    EV_INFO << "hc: " << hc << endl;
    etx = dio->getETX() + objectiveFunction->ETX_Calculator_onLink(dio);
    EV_INFO << "etx: " << etx << endl;

    //auto app = check_and_cast<UdpBasicApp*> (host->getSubmodule("app", 0)); //CL to test access to app module
    //EV_INFO <<"numSent = " << app->numSent;//CL to test access to app module //CL to test access to app module

    //macModule = getParentModule()->getSubmodule("wlan",0)->getSubmodule("mac");
    //mac = check_and_cast<Ieee802154Mac *>(macModule);

    //In case, I want to use accumulative metrics
    //auto mac = check_and_cast<Ieee802154Mac *>(host->getSubmodule("wlan",0)->getSubmodule("mac"));
    //drop = dio->getDROP() + mac->nbDroppedFrames_copy;  //drop was used as an example
    //EV_INFO << "my accumulative drop value: " << drop << endl;

    //EV_INFO << "Updating metrics from DIO: " << endl;
    //dropT = dio->getDropT();      //nbDroppedFrames in L2
    dropB = dio->getDropB();      //Frames drop by Backoff (congestion)
    dropR = dio->getDropR();      //Frames drop by Retry (collisions or channel interferences)
    missACK = dio->getMissACK();  //nbMissed ACKs
    txF = dio->getTxF() ;            //nbTxFrames
    rxF = dio->getRxF();             //nbRxFrames
    bw = dio->getBw();                       //bandwidth, pending of implementing (channel utilization)
    den = dio->getDen();
    qu = dio->getQu();
    fps = dio->getFps();
    snr = dio->getSnr(); //2022-09-14
    rxuns = dio->getRx_un_suc();
    rxsucrate = dio->getRx_suc_rate();

    //EV_INFO << "dropT metric value is: " << dropT << endl;
    EV_INFO << "dropB metric value is: " << dropB << endl;
    EV_INFO << "dropR metric value is: " << dropR << endl;
    EV_INFO << "missACK metric value is: " << missACK << endl;
    EV_INFO << "txF  metric value is: " << txF  << endl;
    EV_INFO << "rxF metric value is: " << rxF << endl;
    EV_INFO << "bw metric value is: " << bw << endl;
    EV_INFO << "den metric value is: " << den << endl;
    EV_INFO << "QU metric value is: " << qu << endl;
    EV_INFO << "fps metric value is: " << fps << endl;
    EV_INFO << "SNR metric value is: " << snr << endl;
    EV_INFO << "Unsuccessful metric value is: " << rxuns << endl;
    EV_INFO << "Successful rate metric vlues is: " << rxsucrate << endl;

     //dropT = mac->nbDroppedFrames_copy;
     //dropB = mac->framedropbycongestion;
     //dropR = mac->framedropbyretry_limit_reached;
     //missACK = mac->nbMissedAcks_copy;
     //txF = mac->nbTxFrames_copy ;
     //rxF = mac->nbRxFrames_copy;
     //bw = 0; //bandwidth, pending of implementing

    path_cost = dio->getPath_cost(); //+ objectiveFunction->Path_Cost_Calculator_ptr(dio);
    //EV_INFO << "dio->getPath_cost() = " << dio->getPath_cost() << endl;
    //EV_INFO << "objectiveFunction->Path_Cost_Calculator_ptr(dio) = " << objectiveFunction->Path_Cost_Calculator_ptr(dio) << endl;
    EV_INFO << "Path Cost of this DIO = " << path_cost << endl;

    last_update = simTime();
    EV_INFO << "This dio was updated at: " << last_update << endl;

}
void Rpl::updateMetrics_fromPrefParent (Dio* preferredParent){

    EV_INFO << "Updating metrics from Pref. Parent: " << endl;
    hc = preferredParent->getHC() + 1;
    EV_INFO << "hc: " << hc << endl;
    etx = preferredParent->getETX() + objectiveFunction->ETX_Calculator_onLink_calcRank(preferredParent);
    EV_INFO << "etx: " << etx << endl;

    //auto app = check_and_cast<UdpBasicApp*> (host->getSubmodule("app", 0)); //CL to test access to app module
    //EV_INFO <<"numSent = " << app->numSent;//CL to test access to app module //CL to test access to app module

    //macModule = getParentModule()->getSubmodule("wlan",0)->getSubmodule("mac");
    //mac = check_and_cast<Ieee802154Mac *>(macModule);

     //auto mac = check_and_cast<Ieee802154Mac *>(host->getSubmodule("wlan",0)->getSubmodule("mac"));
     //drop = dio->getDROP() + mac->nbDroppedFrames_copy;
     //EV_INFO << "my accumulative drop value: " << drop << endl;

    //Added on Feb 16-2022

    //dropT = preferredParent->getDropT();      //nbDroppedFrames in L2
    dropB = preferredParent->getDropB();      //Frames drop by Backoff (congestion)
    dropR = preferredParent->getDropR();      //Frames drop by Retry (collisions or channel interferences)
    missACK = preferredParent->getMissACK();  //nbMissed ACKs
    txF = preferredParent->getTxF() ;            //nbTxFrames
    rxF = preferredParent->getRxF();             //nbRxFrames
    bw = preferredParent->getBw();              //bandwidth, pending of implementing
    den = preferredParent->getDen();
    qu = preferredParent->getQu();
    fps = preferredParent->getFps();
    last_update = simTime();  //2022-05-13
    snr = preferredParent->getSnr(); //2022-09-14
    rxuns = preferredParent->getRx_un_suc();
    rxsucrate = preferredParent->getRx_suc_rate();

    //EV_INFO << "Updating metrics from Pref. Parent: " << endl;
    //EV_INFO << "dropT metric value is: " << dropT << endl;
    EV_INFO << "dropB metric value is: " << dropB << endl;
    EV_INFO << "dropR metric value is: " << dropR << endl;
    EV_INFO << "missACK metric value is: " << missACK << endl;
    EV_INFO << "txF  metric value is: " << txF  << endl;
    EV_INFO << "rxF metric value is: " << rxF << endl;
    EV_INFO << "bw metric value is: " << bw << endl;
    EV_INFO << "den metric value is: " << den << endl;
    EV_INFO << "QU metric value is: " << qu << endl;
    EV_INFO << "Fps metric value is: " << fps << endl;
    EV_INFO << "SNR metric value is: " << snr << endl;
    EV_INFO << "rxuns = " << rxuns << endl;
    EV_INFO << "rxsucrate = " << rxsucrate << endl;

    path_cost = preferredParent->getPath_cost(); // + objectiveFunction->Path_Cost_Calculator(preferredParent);
    EV_INFO << "Path Cost of my pref parent = " << path_cost << endl;
}

//CL 02-02-2022
void Rpl::updateNeighbour(const Ptr<const Dio>& dio)
{
    /**
     * Based on addNeighbour
     */
    auto dioCopy = dio->dup();
    auto dioSender = dio->getSrcAddress();
//NO    /** If DIO sender has equal rank, consider this node as a backup parent */
//    EV_INFO <<"dio Ranking = "<<dio->getRank()<<" current node ranking: "<<rank << endl;
//    if (dio->getRank() == rank) {
//        if (backupParents.find(dioSender) != backupParents.end())
//            EV_DETAIL << "Backup parent entry updated - " << dioSender << endl;
//        else
//            EV_DETAIL << "New backup parent added - " << dioSender << endl;
//        backupParents[dioSender] = dioCopy;
//    }
    /** If node's new rank based on DIO sender is going to be equal to the current rank that this node has, just add this node as a candidate parent */
//    if ((temp_rank)==rank)
//        candidateParents[dioSender] = dioCopy;
//    else  //no need to check if temp_rank is less than current rank
        /** If DIO sender has lower rank, consider this node as a candidate parent */


    candidateParents[dioSender] = dioCopy;
    //updatePreferredParent();

        //     if (dio->getRank() < rank) {
//        if (candidateParents.find(dioSender) != candidateParents.end())
//            EV_DETAIL << "Candidate parent entry updated - " << dioSender << endl;
//        else
//            EV_DETAIL << "New candidate parent added - " << dioSender << endl;
//        candidateParents[dioSender] = dioCopy;
//    }
}

void Rpl::countNeighbours(const Ptr<const Dio>& dio){
    std::vector<ParentStructure*>::iterator it;
        for(it = counterNeighbors.begin(); it != counterNeighbors.end(); it++)
        {
            if(dio->getSrcAddress() == (*it)->SenderIpAddr){
                EV_INFO <<" I already count this sender " <<endl;
                return;
            }
        }
        if (it == counterNeighbors.end()) {
            ParentStructure* rcv = new ParentStructure();
            rcv->SenderIpAddr = dio->getSrcAddress();
            counterNeighbors.push_back(rcv);
            neighbors = neighbors + 1;
            EV_INFO <<"this node has " << den << " neighbors" <<endl;
        }

        return;
}

void Rpl::updateMetrics_frequently () {

    if (isRoot){
        EV_INFO << "yes, it is root" << endl;
        return;
    }

    metric_updater_timer = new cMessage("time for update metrics", METRIC_TIMER);
    scheduleAt(simTime() + 5, metric_updater_timer);

    //An error is launched if there is no preferred parent
    if (neighbors ==0){
        EV_INFO << "I have zero neighbors" << endl;
        return;
    }

    macModule = getParentModule()->getSubmodule("wlan",0)->getSubmodule("mac");
    mac = check_and_cast<Ieee802154Mac *>(macModule);

    uint64_t nodeId = preferredParent->getNodeId();

    if (mac->getACKrcv(nodeId)==0 & mac->getACKmissed(nodeId) ==0){
        etx_inst = 1;
        EV_INFO << "updating etx_int of my pref parent: " << etx_inst << endl;
        EV_INFO << "my pref parent is: " << nodeId << endl;
        //for SNR
        snr_inst = mac->getSNRave(nodeId);
        EV_INFO << "updating snr_int of my pref parent: " << snr_inst << endl;
        EV_INFO << "my pref parent is: " << nodeId << endl;
        return ;
    }
    if (mac->getACKrcv(nodeId) ==0) {
        etx_inst = 100; //a very high ETX value
        EV_INFO << "updating etx_int of my pref parent: " << etx_inst << endl;
        EV_INFO << "my pref parent is: " << nodeId << endl;
        //for SNR
        snr_inst = mac->getSNRave(nodeId);
        EV_INFO << "updating snr_int of my pref parent: " << snr_inst << endl;
        EV_INFO << "my pref parent is: " << nodeId << endl;
        return ;
    }

    etx_inst = (mac->getACKmissed(nodeId) + mac->getACKrcv(nodeId))/mac->getACKrcv(nodeId);

    EV_INFO << "updating etx_int of my pref parent: " << etx_inst << endl;

    //for SNR
    snr_inst = mac->getSNRave(nodeId);
    EV_INFO << "updating snr_int of my pref parent: " << snr_inst << endl;
    EV_INFO << "my pref parent is: " << nodeId << endl;

    return;

}

void Rpl::connecting()
{
    EV_INFO << "connecting" << endl;

    /*
        Create a TCP socket
    */


    #pragma comment(lib,"ws2_32.lib") //Winsock Library

    //int main(int argc , char *argv[])
    //{
        WSADATA wsa;
        SOCKET s;
        struct sockaddr_in server;
        const char *message ;
        char *space = " " ;
        char server_reply [2000];
        int recv_size;

        printf("\nInitialising Winsock...");

        if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
        {
            //EV_INFO << "Failed. Error Code : %d", WSAGetLastError() << endl;
            //return 1;
        }

        EV_INFO << "Initialised" << endl ;

        //Create a socket
        if((s = socket(AF_INET , SOCK_STREAM , 0 )) == INVALID_SOCKET)
        {
            //EV_INFO << "Could not create socket : %d" , WSAGetLastError() << endl;
        }

        EV_INFO << "Socket created" << endl ;


        server.sin_addr.s_addr = inet_addr("127.0.0.1");
        server.sin_family = AF_INET;
        server.sin_port = htons( 8080 );

        //Connect to remote server
        if (connect(s , (struct sockaddr *)&server , sizeof(server)) < 0)
        {
            puts("connect error");
           // return 1;
        }

        puts("Connected");
        EV_INFO <<"conected"<< endl;
        //return 0;

        //Send some data
        //message = sprintf(2.53) ;//"GET / HTTP/1.1\r\n\r\n";
        double num = 2.53;
        //std::stringstream ss;
        //ss << i;
        //message = ss.str();
        //message = ss.str();
        //char ch = static_cast<char>(i);
        //send(socketClient, (void*)&userDataBuffer[i], sizeof(double), 0);
        //if(sendto(s , message , strlen(message) , 0 ,0,0) < 0)
       // std::string st = std::to_string(num);
       // message = st.c_str();

        //Custom precision
        std::ostringstream streamObj;
        streamObj << std::fixed;
        streamObj << std::setprecision(2);
        streamObj << num;
        std::string strObj = streamObj.str() + " " + streamObj.str() +  " " + streamObj.str() + " " + streamObj.str()
                + " " + streamObj.str() + " " + streamObj.str() ;
        message = strObj.c_str();

        sendto(s , message , strlen(message) , 0 , 0, 0 );
    /*    sendto(s , space , strlen(space) , 0 , 0, 0 );
        sendto(s , message , strlen(message) , 0 , 0, 0 );
        sendto(s , space , strlen(space) , 0 , 0, 0 );
        sendto(s , message , strlen(message) , 0 , 0, 0 );
        sendto(s , space , strlen(space) , 0 , 0, 0 );
        sendto(s , message , strlen(message) , 0 , 0, 0 ); */
    /*        {
                puts("Send failed");
                //return 1;
            }
            puts("Data Send\n"); */

            //return 0;

            //Receive a reply from the server
                if((recv_size = recv(s , server_reply , 2000 , 0)) == SOCKET_ERROR)
                {
                    //puts("recv failed");
                    EV_INFO << "rec failed" << endl ;
                }

                //puts("Reply received\n");
                //EV_INFO << "rec failed" << endl ;

                //Add a NULL terminating character to make it a proper string before printing
                //server_reply[recv_size] = '\0';
                //puts(server_reply);

                // convert string to float
                //float num_float = std::stof(server_reply);

                // convert string to double
                double num_double = std::stod(server_reply);
                EV_INFO << "server reply: " << num_double << endl;

                std::ofstream file;
                file.open("St_RankingCalcReplayfromML.txt", std::ios_base::app);
                file << getParentModule()->getFullName() << " received: " << num_double << endl;
                file.close();


            //recv_size = recv(s , server_reply , 2000 , 0)

            closesocket(s);
            WSACleanup();
    //}
}

void Rpl::updateBestCandidate()
{
    //2022-04-11: To be used later to see if the rank changed
    //double current_rank = rank;

    //Dio *newPrefParent;
    Dio *newBestForwardingCandidate;
    EV_DETAIL << "Choosing the best forwarding candidate from "
            << boolStr(candidateParents.empty() && par("useBackupAsPreferred").boolValue(),
                    "backup", "candidate") << " parent set" << endl;
    /**
     * Choose parent from candidate neighbor set. If it's empty, leave DODAG.
     */

    if (candidateParents.empty())
        if (par("useBackupAsPreferred").boolValue())
            //newPrefParent = objectiveFunction->getPreferredParent(backupParents, preferredParent); //back to this on 22-04-29
            //newPrefParent = objectiveFunction->getPreferredParent(backupParents, preferredParent); //2022-03-08
            EV_INFO << "nothing to do" << endl;
        else {
            detachFromDodag();
            return;
        }
    else
        //newPrefParent = objectiveFunction->getPreferredParent(candidateParents);
        //newPrefParent = objectiveFunction->getPreferredParent(candidateParents, preferredParent); //2022-03-08...2022-04-29
        newBestForwardingCandidate = objectiveFunction->GetBestCandidate(candidateParents); //2022-11

    //auto newPrefParentAddr = newPrefParent->getSrcAddress();
    auto newBestForwardingCandidateAddr = newBestForwardingCandidate->getSrcAddress();
    /**
     * If a better preferred parent is discovered (based on the objective function metric),
     * update default route to DODAG sink with the new nextHop
     */

//    if (!(checkPrefParentChanged(newPrefParentAddr,previous_PrefParentAddr))) {  //2022-03-08

        //CL: if it is here is because the preferred parent changed otherwise it was only updated
        std::ofstream file;  // CL 2022-02-02
        file.open("St_BestForwardingCandidatesChanges.txt", std::ios_base::app);
        file << simTime() <<" " << getParentModule()->getFullName() << " The best forwarding candidate is: " << newBestForwardingCandidateAddr << endl;
        //

        //auto newPrefParentDodagId = newPrefParent->getDodagId();
        auto newPrefParentDodagId = newBestForwardingCandidate->getDodagId();

        /** Silently join new DODAG and update dest address for application, TODO: Check with RFC */
        dodagId = newPrefParentDodagId;
        //if (udpApp && !isUdpSink() && udpApp->par("destAddresses").str().empty()) //commented by CL 2021-11-19
            //udpApp->par("destAddresses") = newPrefParentDodagId.str();            //commented by CL

        daoSeqNum = 0;
        clearParentRoutes();
        //drawConnector(newPrefParent->getPosition(), newPrefParent->getColor());
        drawConnector(newBestForwardingCandidate->getPosition(), newBestForwardingCandidate->getColor());
        //updateRoutingTable(newPrefParentAddr, dodagId, nullptr, true);
        updateRoutingTable(newBestForwardingCandidateAddr, dodagId, nullptr, true);

        // required for proper nextHop address resolution
/*
        if (newPrefParentAddr != dodagId)
            updateRoutingTable(newPrefParentAddr, newPrefParentAddr, nullptr, false);

        lastTransit = new Ipv6Address(newPrefParentAddr);
        EV_DETAIL << "Updated preferred parent to - " << newPrefParentAddr << endl;
 */
        if (newBestForwardingCandidateAddr != dodagId)
            updateRoutingTable(newBestForwardingCandidateAddr, newBestForwardingCandidateAddr, nullptr, false);

        lastTransit = new Ipv6Address(newBestForwardingCandidateAddr);
        EV_DETAIL << "Updated best forwarding candidate - " << newBestForwardingCandidate << endl;
        //preferredParent = newBestForwardingCandidate->dup(); //I included this line after getting an error

        //numParentUpdates++;
        /**
         * Reset trickle timer due to inconsistency (preferred parent changed) detected, thus
         * maintaining higher topology reactivity and convergence rate [RFC 6550, 8.3]
         */

        //trickleTimer->reset();  //2022-05-15

        /**
         * OF based on ETX needs advertise the change of preferred parent because the rank of the other nodes depends on that, so there is a need to reset the trickleTimer
         * to advertise the change. For RPL_ENH, if the node does not change the rank value and just changes to a pref parent with the same ranking, but better path_cost, I am thinking
         * to avoid resetting the trickleTimer to reduce the amount of DIO sent.         *
         */
/*
        std::string OF = par("objectiveFunctionType").stdstringValue(); //CL 2022-11-04
        //EV_INFO << "I am using OF:" << OF << endl;
        if (OF == "ETX")
            trickleTimer->reset();
        else if (OF == "HOP_COUNT")
            trickleTimer->reset();
        else if (OF == "HC_MOD")
            trickleTimer->reset();
        else if (OF == "ML")
            trickleTimer->reset();
        else if (OF=="RPL_ENH"){
            if (newPrefParent->getRank() + 1 == current_rank)
                EV_INFO << "do nothing if the rank of my pref parent did not change, and only changed the path_cost" << endl;
            else
                trickleTimer->reset();
        }else
            trickleTimer->reset();

//TODO: How to handle the DAOs should be fixed
        if (daoEnabled) {
            if (storing)
                // TODO: magic numbers
                sendRplPacket(createDao(), DAO, newPrefParentAddr, daoDelay * uniform(1, 7));
            else
                sendRplPacket(createDao(), DAO, newPrefParentAddr, daoDelay * uniform(1, 7),
                            getSelfAddress(), newPrefParentAddr);

            EV_DETAIL << "Sending DAO to new pref. parent - " << newPrefParentAddr
                    << " advertising " << getSelfAddress() << " reachability" << endl;
        }
    }
    preferredParent = newPrefParent->dup();
    previous_PrefParentAddr = preferredParent->getSrcAddress();  //2022-03-08
*/
    /** Recalculate rank based on the objective function */
//    rank = objectiveFunction->calcRank(preferredParent);
//    EV_DETAIL << "My current Rank: " << rank << endl;
    //EV_DETAIL << "My HC value: " << getHC() << endl;
    //EV_DETAIL << "My ETX value: " << getETX() << endl;

}

//
// Notification handling

/*void Rpl::updateCudpAppounterCache(const Ptr<const Dio>& dio){

    std::vector<ParentStructure*>::iterator it;
    for(it = countercache.begin(); it != countercache.end(); it++)
    {
        if(dio->getSrcAddress() == (*it)->SenderIpAddr){
            (*it)->Rcvdcounter = (*it)->Rcvdcounter + 1;
            EV_INFO <<"Number of DIOs received from this sender: " << (*it)->Rcvdcounter <<endl;
            EV_INFO <<"Number of DIOs sent by this sender = "<< dio->getNumDIO()  <<endl;
            return;
        }
    }
    if (it == countercache.end()) {
        ParentStructure* rcv = new ParentStructure();
        rcv->SenderIpAddr = dio->getSrcAddress();
        rcv->Rcvdcounter = 1;
        countercache.push_back(rcv);
        EV_INFO <<"first DIO received from this sender " <<endl;
    }

    return;
}*/

} // namespace inet



