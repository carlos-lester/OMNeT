//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004,2011 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/applications/base/ApplicationPacket_m.h"
#include "inet/applications/udpapp/UdpBasicApp.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/TagBase_m.h"
#include "inet/common/TimeTag_m.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/FragmentationTag_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/transportlayer/contract/udp/UdpControlInfo_m.h"
//#include "C:/omnetpp-5.6.2/MyWorkspaces/ComNestHH_CLv3/rpl/src/Rpl.h"

#include <iostream>
#include <string>
#include <fstream>

namespace inet {

Define_Module(UdpBasicApp);

UdpBasicApp::~UdpBasicApp()
{
    cancelAndDelete(selfMsg);
}

void UdpBasicApp::initialize(int stage)
{
    ApplicationBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        numSent = 0;
        numReceived = 0;
        WATCH(numSent);
        WATCH(numReceived);

        localPort = par("localPort");
        destPort = par("destPort");
        startTime = par("startTime");
        stopTime = par("stopTime");
        packetName = par("packetName");
        dontFragment = par("dontFragment");
        if (stopTime >= SIMTIME_ZERO && stopTime < startTime)
            throw cRuntimeError("Invalid startTime/stopTime parameters");
        selfMsg = new cMessage("sendTimer");
    }
}

void UdpBasicApp::finish()
{
    recordScalar("packets sent", numSent);
    recordScalar("packets received", numReceived);
    ApplicationBase::finish();
}

void UdpBasicApp::setSocketOptions()
{
    int timeToLive = par("timeToLive");
    if (timeToLive != -1)
        socket.setTimeToLive(timeToLive);

    int dscp = par("dscp");
    if (dscp != -1)
        socket.setDscp(dscp);

    int tos = par("tos");
    if (tos != -1)
        socket.setTos(tos);

    const char *multicastInterface = par("multicastInterface");
    if (multicastInterface[0]) {
        IInterfaceTable *ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        InterfaceEntry *ie = ift->findInterfaceByName(multicastInterface);
        if (!ie)
            throw cRuntimeError("Wrong multicastInterface setting: no interface named \"%s\"", multicastInterface);
        socket.setMulticastOutputInterface(ie->getInterfaceId());
    }

    bool receiveBroadcast = par("receiveBroadcast");
    if (receiveBroadcast)
        socket.setBroadcast(true);

    bool joinLocalMulticastGroups = par("joinLocalMulticastGroups");
    if (joinLocalMulticastGroups) {
        MulticastGroupList mgl = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this)->collectMulticastGroups();
        socket.joinLocalMulticastGroups(mgl);
    }
    socket.setCallback(this);
}

L3Address UdpBasicApp::chooseDestAddr()
{
    int k = intrand(destAddresses.size());
    if (destAddresses[k].isUnspecified() || destAddresses[k].isLinkLocal()) {
        L3AddressResolver().tryResolve(destAddressStr[k].c_str(), destAddresses[k]);
    }
    return destAddresses[k];
}

void UdpBasicApp::sendPacket()
{
    std::ostringstream str;
    //str << packetName << "-" << numSent;
    str << packetName << "-" << getParentModule()->getFullName() << "-" << numSent ; //CL
    Packet *packet = new Packet(str.str().c_str());
    if(dontFragment)
        packet->addTag<FragmentationReq>()->setDontFragment(true);
    const auto& payload = makeShared<ApplicationPacket>();
    payload->setChunkLength(B(par("messageLength")));
    payload->setSequenceNumber(numSent);
    payload->addTag<CreationTimeTag>()->setCreationTime(simTime());
    packet->insertAtBack(payload);
    L3Address destAddr = chooseDestAddr();
    emit(packetSentSignal, packet);
    socket.sendTo(packet, destAddr, destPort);
    numSent++;

    //CL: 2021-06-16
    //To register when the packet was sent.
    std:: ofstream file;
    file.open("St_packet-tracer.txt", std::ios_base::app);
    std::string packetName = packet->getName();

    int packetSize = par("messageLength");
    //EV_INFO << "packet size: " << packetSize << endl;

    //To get variables from other places
    macModule = getParentModule()->getSubmodule("wlan",0)->getSubmodule("mac");
    mac = check_and_cast<Ieee802154Mac *>(macModule);

    rplModule = getParentModule()->getSubmodule("rpl");
    //EV_INFO << rplModule << endl;
    rpl = check_and_cast<Rpl *>(rplModule);
    //EV_INFO << rpl->getRank() << endl;

    //To get the best candidate to send the packet before sending
    //if there is no prefparent yet, skip this:
/*    if(rpl->getneighbors()==0 or rpl->getRank()==2)
        EV_INFO << "skip rpl->updateBestCandidate()...no neighbors yet " << endl;
    else
        rpl->updateBestCandidate();
*/
    //radioModule = getParentModule()->getSubmodule("wlan",0)->getSubmodule("radio");
    //radio = check_and_cast<Radio *>(radioModule);
    //EV_INFO << "SNR: " << radio->snr_L1 << endl ;

    //radioModule = getParentModule()->getSubmodule("wlan",0)->getSubmodule("mac");

/*    file << simTime() <<" " <<getParentModule()->getFullName() << " pkt_sent " << packetName <<" " << packetSize <<" "<<
         mac->nbTxFrames_copy<< " "<< mac->nbRxFrames_copy << " " << mac->retry_limit_reached_copy << " " <<
         mac->framedropbycongestion_copy << " " << mac->nbMissedAcks_copy << " " << mac->nbDroppedFrames_copy <<
         " " << rpl->getRank() <<endl;
*/
    //file << simTime() <<" " <<getParentModule()->getFullName() << " pkt_sent " << packetName << endl;
    double fail_retry = 0;
    double fail_cong = 0;
    double fail_retry_current_value;
    double fail_cong_current_value;
    double rx_frame_rate;
    double current_rx_frame_rate = mac->nbRxFrames_copy;

    if (simTime() - mac->last_reading > 300){
        double beta = 0.9;
            //dio->setRx_suc_rate(beta*(radio->rx_successful_rate) + (1-beta)*radio->rx_successful_rate_copy);
        if (mac->nbTxFrames_copy + mac->txAttempts_copy == 0)
            fail_retry_current_value = 0;
        else
            fail_retry_current_value = (mac->framedropbyretry_limit_reached + mac->txAttempts_copy)/(mac->nbTxFrames_copy + mac->txAttempts_copy);

        fail_retry = beta*fail_retry_current_value + (1-beta)*mac->fail_rate_retry ;

        if (mac->nbTxFrames_copy + mac->txAttempts_copy + mac->NB_copy == 0)
            fail_cong_current_value = 0;
        else
            fail_cong_current_value = (mac->framedropbycongestion + mac->txAttempts_copy + mac->NB_copy)/(mac->nbTxFrames_copy + mac->txAttempts_copy + mac->NB_copy);

        fail_cong = beta*fail_cong_current_value + (1-beta)*mac->fail_rate_cong ;

        rx_frame_rate = beta*current_rx_frame_rate + (1-beta)*mac->rx_frames_rate ;  //check this!!!!!!!!

    } else {
        double beta = 0.6;
        if (mac->nbTxFrames_copy + mac->txAttempts_copy == 0)
            fail_retry_current_value = 0;
        else
            fail_retry_current_value = (mac->framedropbyretry_limit_reached + mac->txAttempts_copy)/(mac->nbTxFrames_copy + mac->txAttempts_copy);

        fail_retry = beta*fail_retry_current_value + (1-beta)*mac->fail_rate_retry ;

        if (mac->nbTxFrames_copy + mac->txAttempts_copy + mac->NB_copy == 0)
            fail_cong_current_value = 0;
        else
            fail_cong_current_value = (mac->framedropbycongestion + mac->txAttempts_copy + mac->NB_copy)/(mac->nbTxFrames_copy + mac->txAttempts_copy + mac->NB_copy);

        fail_cong = beta*fail_cong_current_value + (1-beta)*mac->fail_rate_cong ;

        rx_frame_rate = beta*current_rx_frame_rate + (1-beta)*mac->rx_frames_rate ;  //check this!!!!!!!!
    }

    //To do the same with the ch utilization
    double current_ch_util;
    if ((mac->busy + mac->idle) == 0)
        current_ch_util = 0;
    else
        current_ch_util =mac->busy/(mac->busy + mac->idle);

    double my_ch_util = 0;

    if (simTime() - mac->last_ch_uti_reset > 300){
        double beta = 0.9;
        my_ch_util = beta*current_ch_util + (1-beta)*mac->channel_util ;
    } else {
        double beta = 0.6;
        my_ch_util = beta*current_ch_util + (1-beta)*mac->channel_util ;
    }


/*
    file << simTime() <<" " << getParentModule()->getFullName() << " pkt_sent " << packetName <<" " <<
         rpl->getHC() << " " << round(rpl->getETX()*100.0)/100.0 << " " << rpl->getdropB() << " " <<
         rpl->getdropR() << " " << round(rpl->gettxF()*100.0)/100.0 << " " << round(rpl->getrxF()*100.0)/100.0 << " " <<
         round(rpl->getbw()*100.0)/100.0 << " " << rpl->getden() << " " << round(rpl->getqu()*100.0)/100.0 << " " <<
         round(my_ch_util*100.0)/100.0 << " " << round(mac->qu*100.0)/100.0 << " " <<
         round(fail_retry*100.0)/100.0 << " " << round(fail_cong*100.0)/100.0 << " " <<
         rpl->getneighbors() << " " << round(rpl->getetx_int()*100.0)/100.0 << " " <<
         round(rpl->getfps()*100.0)/100.0 << " " << rpl->getlastupdate() << " " << rpl->getSNR() << " " <<
         round(rpl->getsnr_inst()*100)/100.0 << " " << round(rpl->getRXsucrate()*100.0)/100.0 << " " << endl ;
*/

    file << simTime() <<" " << getParentModule()->getFullName() << " pkt_sent " << packetName <<" " <<
                rpl->getHC() << " " << round(rpl->getETX()*100.0)/100.0 << " " <<  rpl->getdropB() << " " <<
                rpl->getdropR() << " " << round(rpl->gettxF()*100.0)/100.0 << " " << round(rpl->getrxF()*100.0)/100.0 << " " <<
                round(rpl->getbw()*100.0)/100.0 << " " << rpl->getden() << " " << round(rpl->getqu()*100.0)/100.0 << " " <<
                round(my_ch_util*100.0)/100.0 << " " << round(mac->qu*100.0)/100.0 << " " <<
                round(fail_retry*100.0)/100.0 << " " << round(fail_cong*100.0)/100.0 << " " <<
                rpl->getneighbors() << " " << round(rpl->getetx_int()*100.0)/100.0 << " " <<
                round(rpl->getfps()*100.0)/100.0 << " " << rpl->getlastupdate()  << " " << rpl->getSNR() <<
                " " << round(rpl->getsnr_inst()*100)/100.0 << " " << round(rx_frame_rate*100.0)/100.0 << endl ;

    file.close();

}

void UdpBasicApp::processStart()
{
    socket.setOutputGate(gate("socketOut"));
    const char *localAddress = par("localAddress");
    socket.bind(*localAddress ? L3AddressResolver().resolve(localAddress) : L3Address(), localPort);
    setSocketOptions();

    const char *destAddrs = par("destAddresses");
    cStringTokenizer tokenizer(destAddrs);
    const char *token;

    while ((token = tokenizer.nextToken()) != nullptr) {
        destAddressStr.push_back(token);
        L3Address result;
        L3AddressResolver().tryResolve(token, result);
        if (result.isUnspecified())
            EV_ERROR << "cannot resolve destination address: " << token << endl;
        destAddresses.push_back(result);
    }

    if (!destAddresses.empty()) {
        selfMsg->setKind(SEND);
        processSend();
    }
    else {
        if (stopTime >= SIMTIME_ZERO) {
            selfMsg->setKind(STOP);
            scheduleAt(stopTime, selfMsg);
        }
    }
}

void UdpBasicApp::processSend()
{
    sendPacket();
    simtime_t d = simTime() + par("sendInterval");
    if (stopTime < SIMTIME_ZERO || d < stopTime) {
        selfMsg->setKind(SEND);
        scheduleAt(d, selfMsg);
    }
    else {
        selfMsg->setKind(STOP);
        scheduleAt(stopTime, selfMsg);
    }
}

void UdpBasicApp::processStop()
{
    socket.close();
}

void UdpBasicApp::handleMessageWhenUp(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        ASSERT(msg == selfMsg);
        switch (selfMsg->getKind()) {
            case START:
                processStart();
                break;

            case SEND:
                processSend();
                break;

            case STOP:
                processStop();
                break;

            default:
                throw cRuntimeError("Invalid kind %d in self message", (int)selfMsg->getKind());
        }
    }
    else
        socket.processMessage(msg);
}

void UdpBasicApp::socketDataArrived(UdpSocket *socket, Packet *packet)
{
    // process incoming packet
    processPacket(packet);
}

void UdpBasicApp::socketErrorArrived(UdpSocket *socket, Indication *indication)
{
    EV_WARN << "Ignoring UDP error report " << indication->getName() << endl;
    delete indication;
}

void UdpBasicApp::socketClosed(UdpSocket *socket)
{
    if (operationalState == State::STOPPING_OPERATION)
        startActiveOperationExtraTimeOrFinish(par("stopOperationExtraTime"));
}

void UdpBasicApp::refreshDisplay() const
{
    ApplicationBase::refreshDisplay();

    char buf[100];
    sprintf(buf, "rcvd: %d pks\nsent: %d pks", numReceived, numSent);
    getDisplayString().setTagArg("t", 0, buf);
}

void UdpBasicApp::processPacket(Packet *pk)
{
    emit(packetReceivedSignal, pk);
    EV_INFO << "Received packet: " << UdpSocket::getReceivedPacketInfo(pk) << endl;
    delete pk;
    numReceived++;
}

void UdpBasicApp::handleStartOperation(LifecycleOperation *operation)
{
    simtime_t start = std::max(startTime, simTime());
    if ((stopTime < SIMTIME_ZERO) || (start < stopTime) || (start == stopTime && startTime == stopTime)) {
        selfMsg->setKind(START);
        scheduleAt(start, selfMsg);
    }
}

void UdpBasicApp::handleStopOperation(LifecycleOperation *operation)
{
    cancelEvent(selfMsg);
    socket.close();
    delayActiveOperationFinish(par("stopOperationTimeout"));
}

void UdpBasicApp::handleCrashOperation(LifecycleOperation *operation)
{
    cancelEvent(selfMsg);
    socket.destroy();         //TODO  in real operating systems, program crash detected by OS and OS closes sockets of crashed programs.
}

} // namespace inet

