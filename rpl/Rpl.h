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

#ifndef _RPL_H
#define _RPL_H

#include "inet/routing/rpl/TrickleTimer.h"
#include "inet/routing/rpl/RplRouteData.h"
#include "inet/applications/udpapp/UdpBasicApp.h"
#include "inet/applications/udpapp/UdpSink.h"
#include "inet/common/packet/dissector/PacketDissector.h"
#include "inet/common/packet/dissector/ProtocolDissector.h"
#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/common/ModuleAccess.h"
#include "inet/mobility/static/StationaryMobility.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/common/L3Tools.h"

#include "inet/linklayer/ieee802154/Ieee802154Mac.h"  //CL  2021-12-03: to access L2 layer
//#include <Python.h>
//#include <pyembed.h>
//#include "C:/Users/carlo/Anaconda3/Python.h"
#include<stdio.h>
#include <winsock.h>
#include "inet/physicallayer/common/packetlevel/Radio.h"  //CL 2022-09-12
//The next 3 lines were added by CL so that Radio Module can be recognized
#ifdef WITH_RADIO
using namespace inet::physicallayer;
#endif

namespace inet {

class RplRouteData;  //CL 2021-12-03: to avoid circular dependency
class ObjectiveFunction;  // 2022-04-28: to avoid circular dependency between Rpl.h and ObjectiveFunction.h

class Rpl : public RoutingProtocolBase, public cListener, public NetfilterBase::HookBase
{
  private:

    /** Environment */
    IInterfaceTable *interfaceTable;
    Ipv6RoutingTable *routingTable;
    Ipv6NeighbourDiscovery *nd;
    InterfaceEntry *interfaceEntryPtr;
    INetfilter *networkProtocol;
    ObjectiveFunction *objectiveFunction;
    TrickleTimer *trickleTimer;
    cModule *host;
    cModule *udpApp;

    /** RPL configuration parameters and state management */
    uint8_t dodagVersion;
    Ipv6Address dodagId;
    Ipv6Address selfAddr;
    Ipv6Address *lastTarget;
    Ipv6Address *lastTransit;
    uint8_t instanceId;
    double daoDelay;
    double daoAckTimeout;
    double clKickoffTimeout; // timeout for auto-triggering phase II of CL SF
    uint8_t daoRtxCtn;
    uint8_t daoRtxThresh;
    bool isRoot;
    bool daoEnabled;
    bool storing;
    bool pDaoAckEnabled;
    bool hasStarted;
    bool allowDodagSwitching;
    //uint16_t rank;
    double rank; //CL
    //uint16_t temp_rank; //CL
    double temp_rank; //CL
    uint8_t dtsn;
    uint32_t branchChOffset;
    uint16_t branchSize;
    int daoSeqNum;
    Dio *preferredParent;
    Ipv6Address previous_PrefParentAddr ;
    std::string objectiveFunctionType;
    std::map<Ipv6Address, Dio *> backupParents;
    std::map<Ipv6Address, Dio *> candidateParents;
    std::map<Ipv6Address, Ipv6Address> sourceRoutingTable;
    std::map<Ipv6Address, std::pair<cMessage *, uint8_t>> pendingDaoAcks;

    /** Statistics collection */
    simsignal_t dioReceivedSignal;
    simsignal_t daoReceivedSignal;
    simsignal_t parentChangedSignal;
    simsignal_t parentUnreachableSignal;

    /*************CL*******************/
    cOutVector DIOsent;
    cOutVector DAOsent;
    int numDIOSent=0;
    int numDAOSent=0;

    //metric value:
    //Important reminder: these metrics are the ones that I save from my PrefParent, received from DIOs messages
    double etx=0;
    int hc=0;
    //int dropT=0;   //nbDroppedFrames in L2      //no used it anymore
    int dropB=0;   //Frames drop by Backoff (congestion)
    int dropR=0;   //Frames drop by Retry (collisions or channel interferences)
    int missACK=0; //nbMissed ACKs
    double txF=0 ;    // it was before nbTxFrames. I am using the field now for fail_rate_retry
    double rxF=0;     //it was before nbRxFrames. I am using the fiel now for fail_rate_cong
    double bw = 0; //bandwidth, pending of implementing  (channel utilization)
    int den = 0;   //amount of neighbors of my pref parent
    double qu = 0 ; //queue occupation
    double fps = 0; //frame per seconds 2022-05-05
    simtime_t last_update = 0; //to know how out of date are my metrics 2022-05-13

    //these ones have nothing to do with DIOs
    double etx_inst = 0 ;  //it is the etx instantaneous. Only measured respect to the preferred parent. It is not
                           // gotten from DIO
    double snr_inst = 0 ; //same idea as etx_inst

    int neighbors = 0; //my neighbors
    double snr = 0; //2022-09-12: Value of the SNR of the DIO received.

    int rxuns = 0;        //2022-09-29 value from dio received
    double rxsucrate = 0;

    double path_cost = 0; //2022-10-17

    //counters
    int c = 0; //2022-01-31
    int c1 = 0; //2022-01-31
    int c2 = 0; //2022-01-31
    int c3 = 0; //2022-01-31
    int c4 = 0; //2022-01-31
    int c5 = 0; //2022-01-31
    int c6 = 0;
    int c7 = 0;
    //int dio_received = 0 ;

    //double thre = 0.5 ;

    //To get variables from L2   CL 2021-12-10
    cModule *macModule = nullptr;
    Ieee802154Mac *mac = nullptr;

    //To get variables form Radio module CL 2022-09-12
    cModule *radioModule = nullptr;
    Radio *radio = nullptr;

    //struct ParentStructure {          //I moved this part to the ObjectiveFunction.h
        //Ipv6Address SenderIpAddr;
        //double Rcvdcounter = 0;       //it is necessary for of_etx to keep record of DIO received
        //std::string ParentName;
    //};

    struct ParentStructure {
       Ipv6Address SenderIpAddr;
    };
    typedef std::vector<ParentStructure*> CounterCache;
    CounterCache counterNeighbors;

   /*************end CL****************/

    bool pJoinAtSinkAllowed;
    int numDaoDropped;

    /** Misc */
    bool floating;
    bool verbose;
    bool pUseWarmup;
    bool pLockParent;
    bool isLeaf;
    uint8_t detachedTimeout; // temporary variable to suppress msg processing after just leaving the DODAG
    cMessage *detachedTimeoutEvent; // temporary msg corresponding to triggering above functionality
    cMessage *daoAckTimeoutEvent; // temporary msg corresponding to triggering above functionality
    uint8_t prefixLength;
    Coord position;
    uint64_t selfId;    // Primary IE MAC address in decimal
    std::vector<cFigure::Color> colorPalette;
    int udpPacketsRecv;
    cFigure::Color dodagColor;

  public:
    Rpl();
    ~Rpl();

    //cModule *appModule; //CL to test from here to the app module

    //To get variables from L2   CL 2021-11-09
    //cModule *macModule = nullptr;


    /** Conveniently display boolean variable with custom true / false format */
    static std::string boolStr(bool cond, std::string positive, std::string negative);
    static std::string boolStr(bool cond) { return boolStr(cond, "true", "false"); }

    /** Search for a submodule of @param host by its name @param sname */
    static cModule* findSubmodule(std::string sname, cModule *host);

    /** Randomly pick @param numRequested elements from a [0..@param total] array*/
    static std::vector<int> pickRandomly(int total, int numRequested);

    int numParentUpdates;
    int numDaoForwarded;

    int dio_received = 0 ;

    //Added by CL to cache the count the rcvd msg from specific src Addr
    //typedef std::vector<ParentStructure*> CounterCache;
    //CounterCache countercache;

    //void updateCounterCache(const Ptr<const Dio>& dio);  //CL: to handle the CounterCache

    double getRank (); //CL
    double getETX();   //CL
    int getHC();       //CL
    void setETX(double ETX);  //CL
    void setHC(int HC);      //CL

   //int getdropT();   //nbDroppedFrames in L2
   int getdropB();   //Frames drop by Backoff (congestion)
   int getdropR();   //Frames drop by Retry (collisions or channel interferences)
   int getmissACK(); //nbMissed ACKs
   double gettxF() ;    //nbTxFrames
   double getrxF();     //nbRxFrames
   double getbw(); //bandwidth, pending of implementing
   int getden();
   double getqu();
   double getfps();
   simtime_t getlastupdate();

   double getSNR(); //2022-09-14

   int getneighbors();
   double getetx_int();
   double getsnr_inst();

    void updateMetrics_fromDIO (const Ptr<const Dio>& dio);  //CL 2021-12-02
    void updateMetrics_fromPrefParent (Dio* preferredParent); //CL 2022-01-29

    cMessage *metric_updater_timer ;   //CL 2022-02-22
    void updateMetrics_frequently ();  //CL 2022-02-22

    int getRXuns();   //CL 2022-09-29
    double getRXsucrate();

    void updateBestCandidate (); //2022-11-08

  protected:
    /** module interface */
    void initialize(int stage) override;
    void handleMessageWhenUp(cMessage *message) override;
    virtual void refreshDisplay() const override;

  private:
    void processSelfMessage(cMessage *message);
    void processMessage(cMessage *message);

    /************ Handling generic packets *************/

    /**
     * Send packet via 'ipOut' gate with specified delay
     *
     * @param packet packet object to be sent out
     * @param delay transmission delay
     */
    void sendPacket(cPacket *packet, double delay);
    void processPacket(Packet *packet);

    /**
     * Process message from trickle timer to start DIO broadcast
     *
     * @param message notification message with kind assigned from enum
     */
    void processTrickleTimerMsg(cMessage *message);

    /************ Handling RPL packets *************/

    /**
     * Process DIO packet by inspecting it's source @see checkUnknownDio(),
     * joining a DODAG if not yet a part of one, (re)starting trickle timer,
     * and updating neighbor sets correspondingly @see updatePreferredParent(), addNeighbour()
     *
     * @param dio DIO packet object for processing
     */
    void processDio(const Ptr<const Dio>& dio);
    //void pre_processDio(Ptr<Dio>& dio);

    void processCrossLayerMsg(const Ptr<const Dio>& dio);

    /**
     * Process DAO packet advertising node's reachability,
     * update routing table if destination advertised was unknown and
     * forward DAO further upwards until root is reached to enable P2P and P2MP communication
     *
     * @param dao DAO packet object for processing
     */
    void processDao(const Ptr<const Dao>& dao);
//    void retransmitDao(Dao *dao);
    void retransmitDao(Ipv6Address advDest);

    /**
     * Process DAO_ACK packet if daoAckRequried flag is set
     *
     * @param daoAck decapsulated DAO_ACK packet for processing
     */
    void processDaoAck(const Ptr<const Dao>& daoAck);
    void saveDaoTransitOptions(Packet *dao);

    /**
     * Send RPL packet (@see createDao(), createDio(), createDis()) via 'ipOut'
     *
     * @param packet packet object encapsulating control and payload data
     * @param code icmpv6-based control code used for RPL packets, [RFC 6550, 6]
     * @param nextHop next hop for the RPL packet to be sent out to (unicast DAO, broadcast DIO, DIS)
     * @param delay transmission delay before sending packet from outgoing gate
     */
    void sendRplPacket(const Ptr<RplPacket>& body, RplPacketCode code, const L3Address& nextHop, double delay, const Ipv6Address &target, const Ipv6Address &transit);
    void sendRplPacket(const Ptr<RplPacket>& body, RplPacketCode code, const L3Address& nextHop, double delay);

    /**
     * Create DIO packet to broadcast DODAG info and configuration parameters
     *
     * @return initialized DIO packet object
     */
    const Ptr<Dio> createDio();
    B getDioSize() { return b(128); }

    const Ptr<Dio> createDio_Unique();  //CL 11/16/2022

    /**
     * Create DAO packet advertising destination reachability
     *
     * @param reachableDest reachable destination, may be own address or forwarded from sub-dodag
     * @return initialized DAO packet object
     */
    const Ptr<Dao> createDao(const Ipv6Address &reachableDest, uint8_t channelOffset);
    const Ptr<Dao> createDao(const Ipv6Address &reachableDest, bool ackRequired);
    const Ptr<Dao> createDao() {return createDao(getSelfAddress()); };
    const Ptr<Dao> createDao(const Ipv6Address &reachableDest) {
        return createDao(reachableDest, (uint8_t) UNDEFINED_CH_OFFSET);
    }

    /**
     * Update routing table with new route to destination reachable via next hop
     *
     * @param nextHop next hop address to reach the destination for findBestMatchingRoute()
     * @param dest discovered destination address being added to the routing table
     */
    void updateRoutingTable(const Ipv6Address &nextHop, const Ipv6Address &dest, RplRouteData *routeData, bool defaultRoute);
    void updateRoutingTable(const Ipv6Address &nextHop, const Ipv6Address &dest, RplRouteData *routeData) { updateRoutingTable(nextHop, dest, routeData, false); };
//    void updateRoutingTable(const Dao *dao);
    RplRouteData* prepRouteData(const Dao *dao);

    void purgeDaoRoutes();

    /**
     * Add node (represented by most recent DIO packet) to one of the neighboring sets:
     *  - backup parents
     *  - candidate parents
     *
     * @param dio DIO packet received recently
     */
    void addNeighbour(const Ptr<const Dio>& dio);

    /**
     * Delete preferred parent and related info:
     *  - route with it as a next-hop from the routing table
     *  - erase corresponding entry from the candidate parent list
     */
    void deletePrefParent() { deletePrefParent(false); };
    void deletePrefParent(bool poisoned);
    void clearParentRoutes();

    /**
     * Update preferred parent based on the current best candidate
     * determined by the objective function from candidate neighbors
     * If change in preferred parent detected, restart Trickle timer [RFC 6550, 8.3]
     */
    void updatePreferredParent();

    /************ Lifecycle ****************/

    virtual void handleStartOperation(LifecycleOperation *operation) override { start(); }
    virtual void handleStopOperation(LifecycleOperation *operation) override { stop(); }
    virtual void handleCrashOperation(LifecycleOperation *operation) override  { stop(); }
    void start();
    void stop();

    /************ Utility *****************/

    /**
     * Check whether DIO comes from an uknown DODAG or has different
     * RPL instance id
     *
     * @param dio recently received DIO packet
     * @return true if DIO sender stems from an unknown DODAG
     */
    bool checkUnknownDio(const Ptr<const Dio>& dio);

    /**
     * Get address of the default network interface
     *
     * @return Ipv6 address set by autoconfigurator
     */
    Ipv6Address getSelfAddress();

    /**
     * Check if node's preferred parent has changed after recalculating via
     * objective function to determine whether routing table updates are necessary
     * as well display corresponding log information
     *
     * @param newPrefParent DIO packet received from node chosen as preferred
     * parent by objective function
     * @return True if new preferred parent address differs from the one currently stored,
     * false otherwise
     */
    //bool checkPrefParentChanged(const Ipv6Address &newPrefParentAddr); //2022-03-08
    bool checkPrefParentChanged(const Ipv6Address &newPrefParentAddr , const Ipv6Address &previous_PrefParentAddr);

    template<typename Map>
    std::string printMap(const Map& map);

    bool selfGeneratedPkt(Packet *pkt);
    bool isUdpSink();
    void purgeRoutingTable();

    /**
     * Check if destination advertised in DAO is already stored in
     * the routing table with the same nextHop address
     *
     * @param dest address of the advertised destination from DAO
     * @param nextHop address of the next hop to this destination
     * @return
     */
    bool checkDestKnown(const Ipv6Address &nextHop, const Ipv6Address &dest);

    /**
     * Detach from a DODAG in case preferred parent has been detected unreachable
     * and candidate parent set is empty. Clear RPL-related state including:
     *  - rank
     *  - dodagId
     *  - RPL instance
     *  - neighbor sets
     * [RFC 6550, 8.2.2.1]
     */
    void detachFromDodag();

    /**
     * Poison node's sub-dodag by advertising rank of INF_RANK in case
     * connection to the DODAG is lost. Child nodes upon hearing this message
     * remove poisoning node from their parent set [RFC 6550, 8.2.2.5]
     */
    void poisonSubDodag();

    /**
     * Check if INF_RANK is advertised in DIO and whether it comes from preferred parent
     */
    bool checkPoisonedParent(const Ptr<const Dio>& dio);

    /**
     * Print all packet tags' classnames
     *
     * @param packet packet to print tags from
     */
    void printTags(Packet *packet);

    /**
     * Print detailed info about RPL Packet Information header
     *
     * @param rpi RPL Packet Information object
     * @return string containing packet info
     */
    std::string printHeader(RplPacketInfo *rpi);

    /** Loop detection */

    /**
     * Check if a rank inconsistency happens during packet forwarding
     * for detailed procedure description [RFC 6550, 11.2.2.2]
     *
     * @param rpi RPL Route Infomration header to check for
     * @return true if ther's mismatched rank relationship, false otherwise
     */
    bool checkRankError(RplPacketInfo *rpi);

    /**
     * Check RPL Packet Information header to spot a forwarding error in storing mode
     * (packet expected to progress down, but no route found) [RFC 6550, 11.2.2.3]
     *
     * @param rpi RPL Route Infomration header to check for
     * @param dest destination address retrieved from packet IP header
     * @return true if forwarding error detected, false otherwise
     */
    bool checkForwardingError(RplPacketInfo *rpi, Ipv6Address &dest);

    /**
     * Get default length of Target/Transit option headers
     * @return predefined default byte value
     */
    B getTransitOptionsLength();

    /**
     * Get default length of Target/Transit option headers
     *
     * @return predefined default byte value
     */
    B getRpiHeaderLength();
    B getDaoLength();

    // TODO: replace by dynamic calculation based on the number of addresses in source routing header
    // + additional field specifying length of this header to allow proper decapsulation
    B getSrhSize() { return B(64); }

    bool isDao(Packet *pkt) { return std::string(pkt->getFullName()).find("Dao") != std::string::npos; }
    bool isUdp(Packet *datagram) { return std::string(datagram->getFullName()).find("Udp") != std::string::npos; }

    //CL to allow to use SRH with others applications
    //bool isPing(Packet *datagram) { return std::string(datagram->getFullName()).find("ping") != std::string::npos; }
    bool isPing(Packet *datagram) { return std::string(datagram->getFullName()).find("dADR") != std::string::npos; }
    //modification in isPing because I changed the name of the application 2022/01/19

    /**
     * Used by sink to collect Transit -> Target reachability information
     * for source-routing purposes [RFC6550, 9.7]
     *
     * @param dao
     */
    void extractSourceRoutingData(Packet *dao);

    /**
     * Determine packet forwarding direction - 'up' or 'down'
     *
     * @param datagram UDP packet to check for
     * @return true if packet travels down, false otherwise
     */
    bool packetTravelsDown(Packet *datagram);

    /**
     * Append RPL Packet Information header to outgoing packet,
     * captured by Netfilter hook
     *
     * @param datagram outgoing UDP packet
     */
    void appendRplPacketInfo(Packet *datagram);

    /**
     * Check RPL Packet Information header on rank consistency,
     * forwarding errors [RFC 6550, 11.2]
     *
     * @param datagram packet coming from upper or lower layers catched by Netfilter hook
     * @return INetfilter interface result specifying further actions with a packet
     */
    Result checkRplHeaders(Packet *datagram);

    /**
     * Append Target and Transit option headers representing
     * child->parent relationship, required for source-routing
     *
     * @param pkt DAO packet to insert option headers [RFC6550, 6.7.7-6.7.8]
     */
    void appendDaoTransitOptions(Packet *pkt, const Ipv6Address &target, const Ipv6Address &transit);
    void appendDaoTransitOptions(Packet *pkt);

    /**
     * Check RPL Packet Information header for loop detection purposes [RFC6550, 11.2]
     *
     * @param datagram application data to check RPL headers for
     * @return
     */
    bool checkRplRouteInfo(Packet *datagram);

    bool checkDuplicateRoute(Ipv6Route *route);

    /**
     * Check if packet has source-routing header (SRH) present
     * @param pkt packet to check for
     * @return true on SRH presence, false otherwise
     */
    bool sourceRouted(Packet *pkt);
    B getDaoFrontOffset();
    std::string rplIcmpCodeToStr(RplPacketCode code);

    /**
     * Manually forward application packet to next hop using the
     * info provided in SRH
     *
     * @param datagram
     */
    void forwardSourceRoutedPacket(Packet *datagram);

    /**
     * At sink append SRH for packets going downwards
     * @param datagram
     */
    void appendSrcRoutingHeader(Packet *datagram);

    /** Netfilter hooks */
    // catching incoming packet
    virtual Result datagramPreRoutingHook(Packet *datagram) override { Enter_Method("datagramPreRoutingHook"); return checkRplHeaders(datagram); }
    virtual Result datagramForwardHook(Packet *datagram) override { return ACCEPT; }
    virtual Result datagramPostRoutingHook(Packet *datagram) override { return ACCEPT; }
    virtual Result datagramLocalInHook(Packet *datagram) override { return ACCEPT; }
    // catching outgoing packet
    virtual Result datagramLocalOutHook(Packet *datagram) override { Enter_Method("datagramLocalOutHook"); return checkRplHeaders(datagram); }

    /** Source-routing methods */
    void constructSrcRoutingHeader(std::deque<Ipv6Address> &addressList, Ipv6Address dest);
    bool destIsRoot(Packet *datagram);

    /**
     * Handle signals by implementing @see cListener interface to
     * get notified when MAC layer reports link break
     *
     * @param source
     * @param signalID
     * @param obj
     * @param details
     */
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;

    std::vector<uint16_t> getNodesPerHopDistance();
    bool isRplPacket(Packet *packet);
    void clearDaoAckTimer(Ipv6Address daoDest);

    std::vector<Ipv6Address> getNearestChildren();
    int getNumDownlinks();

    /** Misc */
    void drawConnector(Coord target, cFigure::Color col);
    static int getNodeId(std::string nodeName);

    cLineFigure *prefParentConnector;

    double startDelay;

    /** Pick random color for parent-child connector drawing (if node's sink) */
    cFigure::Color pickRandomColor();

    void finish() override; //CL

    void updateNeighbour(const Ptr<const Dio>& dio); //CL 2022-02-02

    void countNeighbours(const Ptr<const Dio>& dio); //CL 2022-02-19

    void connecting();

    //void updateBestCandidate (); //2022-11-08

};

} // namespace inet

#endif

