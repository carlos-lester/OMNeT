/* -*- mode:c++ -*- ********************************************************
 * file:        Ieee802154Mac.h
 *
 * author:     Jerome Rousselot, Marcel Steine, Amre El-Hoiydi,
 *                Marc Loebbers, Yosia Hadisusanto
 *
 * copyright:    (C) 2007-2009 CSEM SA
 *              (C) 2009 T.U. Eindhoven
 *                (C) 2004 Telecommunication Networks Group (TKN) at
 *              Technische Universitaet Berlin, Germany.
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 *
 * Funding: This work was partially financed by the European Commission under the
 * Framework 6 IST Project "Wirelessly Accessible Sensor Populations"
 * (WASP) under contract IST-034963.
 ***************************************************************************
 * part of:    Modifications to the MF-2 framework by CSEM
 **************************************************************************/

#ifndef __INET_IEEE802154MAC_H
#define __INET_IEEE802154MAC_H

#include "inet/queueing/contract/IPacketQueue.h"
#include "inet/linklayer/base/MacProtocolBase.h"
#include "inet/linklayer/common/MacAddress.h"
#include "inet/linklayer/contract/IMacProtocol.h"
#include "inet/physicallayer/contract/packetlevel/IRadio.h"

#include "inet/physicallayer/common/packetlevel/Radio.h"  //added by CL to read in radio layer
//The next 3 lines were added by CL so that Radio Module can be recognized
#ifdef WITH_RADIO
using namespace inet::physicallayer;
#endif

namespace inet {

/**
 * @brief Generic CSMA Mac-Layer.
 *
 * Supports constant, linear and exponential backoffs as well as
 * MAC ACKs.
 *
 * @author Jerome Rousselot, Amre El-Hoiydi, Marc Loebbers, Yosia Hadisusanto, Andreas Koepke
 * @author Karl Wessel (port for MiXiM)
 *
 * \image html csmaFSM.png "CSMA Mac-Layer - finite state machine"
 */
class INET_API Ieee802154Mac : public MacProtocolBase, public IMacProtocol
{
  public:
    Ieee802154Mac()
        : MacProtocolBase()
        , nbTxFrames(0)
        , nbRxFrames(0)
        , nbMissedAcks(0)
        , nbRecvdAcks(0)
        , nbDroppedFrames(0)
        , nbTxAcks(0)
        , nbDuplicates(0)
        , nbBackoffs(0)
        , backoffValues(0)
        , backoffTimer(nullptr), ccaTimer(nullptr), sifsTimer(nullptr), rxAckTimer(nullptr)
        , macState(IDLE_1)
        , status(STATUS_OK)
        , radio(nullptr)
        , transmissionState(physicallayer::IRadio::TRANSMISSION_STATE_UNDEFINED)
        , sifs()
        , macAckWaitDuration()
        , headerLength(0)
        , transmissionAttemptInterruptedByRx(false)
        , ccaDetectionTime()
        , rxSetupTime()
        , aTurnaroundTime()
        , macMaxCSMABackoffs(0)
        , macMaxFrameRetries(0)
        , aUnitBackoffPeriod()
        , useMACAcks(false)
        , backoffMethod(CONSTANT)
        , macMinBE(0)
        , macMaxBE(0)
        , initialCW(0)
        , txPower(0)
        , NB(0)
        , txAttempts(0)
        , bitrate(0)
        , ackLength(0)
        , ackMessage(nullptr)
        , SeqNrParent()
        , SeqNrChild()
    {}

    //CL 2021-09-11
        struct ParentStructure {         //it is necessary for ETX to keep record of received ack from every neighbor
               Ipv6Address SenderIpAddr;
               MacAddress SenderMacAddr;
               uint64_t nodeId;  //Node's MAC converted in decimal
               double Rcvdcounter = 0;  //ACKs received from a specific sender
               double ACKMissedcounter = 0; //ACKs missed from a specific sender
               //The idea is that the amount of ACKs transmitted by a specific sender is: Rcvdcounter + ACKMissedcounter
               double Rcvdcounter_all = 0;
               double snr_rssi = 0; //accumulative
               double ave_snr_rssi = 0; //to average the snr from ACK sender
               };
        typedef std::vector<ParentStructure*> CounterCache;
                CounterCache countercache_L2;

       //Method to get into ParentStructure from OF
       double getACKrcv (uint64_t nodeId);
       double getACKmissed (uint64_t nodeId);
       virtual void printCacheL2(); //2022-01-31

       double getSNRave (uint64_t nodeId) ; //2022-10-30

       //Variables that I want to have access from other layers

        int nbTxFrames_copy = 0;
        int nbRxFrames_copy = 0;
        int nbMissedAcks_copy = 0;
       // int nbRecvdAcks_copy = 0;
        int nbDroppedFrames_copy = 0;
       // int nbTxAcks_copy = 0;
       // int nbDuplicates_copy = 0;
       // int nbBackoffs_copy = 0;
       // int backoffValues_copy = 0;
        double rx_frames_rate = 0;

        int framedropbyretry_limit_reached = 0; //means the pkt was trasnmitted but the ACK never was received
        int framedropbycongestion = 0; //means the channel was busy at the time of transmitting. It is incremented exactly
                                       // where nbDroppedFrames is incremented
        int framedropbyretry_limit_reached_copy = 0;
        int framedropbycongestion_copy = 0;

        double channel_util = 0;    //Channel utilization metric.   2022-02-19
        virtual void channel_utilization();
        cMessage *channel_util_mess ;
        double qu = 0; //queue utilization. I'll update it together with channel util to avoid complexity
                       //in fact, this metric according the way I am calculating it, is more "pkt/s in the queue"

        double busy = 0; //to count busy and idle stage  22-10-20
        double idle = 0;

        uint64_t IPfromUpperLayers(MacAddress macAddr);

        // 2022-10-03
        double fail_rate_retry = 0;
        double fail_rate_cong = 0;
        virtual void fail_rate();
        cMessage *fail_rate_mess ;
        double txAttempts_copy = 0;
        double NB_copy = 0;
        simtime_t last_reading = 0;
        simtime_t last_ch_uti_reset = 0;
        //double rx_suc_rate = 0; //try to get it from the radio layer

        //To get variables from L1   CL 2022-10-29
        cModule *radioModule = nullptr;
        Radio *phy = nullptr;
        //cModule *host;
/*
        struct ParentStructure2 {         //it is necessary for ETX to keep record of received ack from every neighbor
           //Ipv6Address SenderIpAddr;
           MacAddress SenderMacAddr;
           uint64_t nodeId;  //Node's MAC converted in decimal
           double Rcvdcounter = 0;  //ACKs received from a specific sender
           //double ACKMissedcounter = 0; //ACKs missed from a specific sender
           double snr_rssi = 0; //accumulative
           double ave_snr_rssi = 0; //to average the snr from ACK sender
        };
        typedef std::vector<ParentStructure2*> CounterCache2;
        CounterCache2 countercache_L2_signal;
*/
        // end CL


    virtual ~Ieee802154Mac();

    /** @brief Initialization of the module and some variables*/
    virtual void initialize(int) override;

    /** @brief Delete all dynamically allocated objects of the module*/
    virtual void finish() override;

    /** @brief Handle messages from lower layer */
    virtual void handleLowerPacket(Packet *packet) override;

    /** @brief Handle messages from upper layer */
    virtual void handleUpperPacket(Packet *packet) override;

    /** @brief Handle self messages such as timers */
    virtual void handleSelfMessage(cMessage *) override;

    /** @brief Handle control messages from lower layer */
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, intval_t value, cObject *details) override;

  protected:
    /** @name Different tracked statistics.*/
    /*@{*/
    long nbTxFrames;
    long nbRxFrames;
    long nbMissedAcks;
    long nbRecvdAcks;
    long nbDroppedFrames;
    long nbTxAcks;
    long nbDuplicates;
    long nbBackoffs;
    double backoffValues;

    //CL  2020-11-17
        //cOutVector congestion; //this has to do with packets losses due to they never come out b/c the medium is busy.
        //cOutVector linkBroken; //this has to do with packets losses due to they never receive the ACK confirming the packet reaches the destination,
                               //and after certain numbers of attempts the pkt is deleted.
        //cOutVector reception_errors;
        //int retry_limit_reached;
        //int framedropbycongestion;
    //end CL
    /*@}*/

    /** @brief MAC states
     * see states diagram.
     */
    enum t_mac_states {
        IDLE_1 = 1,
        BACKOFF_2,
        CCA_3,
        TRANSMITFRAME_4,
        WAITACK_5,
        WAITSIFS_6,
        TRANSMITACK_7
    };

    /*************************************************************/
    /****************** TYPES ************************************/
    /*************************************************************/

    /** @brief Kinds for timer messages.*/
    enum t_mac_timer {
        TIMER_NULL = 0,
        TIMER_BACKOFF,
        TIMER_CCA,
        TIMER_SIFS,
        TIMER_RX_ACK,
    };

    /** @name Pointer for timer messages.*/
    /*@{*/
    cMessage *backoffTimer, *ccaTimer, *sifsTimer, *rxAckTimer;
    /*@}*/

    /** @brief MAC state machine events.
     * See state diagram.*/
    enum t_mac_event {
        EV_SEND_REQUEST = 1,    // 1, 11, 20, 21, 22
        EV_TIMER_BACKOFF,    // 2, 7, 14, 15
        EV_FRAME_TRANSMITTED,    // 4, 19
        EV_ACK_RECEIVED,    // 5
        EV_ACK_TIMEOUT,    // 12
        EV_FRAME_RECEIVED,    // 15, 26
        EV_DUPLICATE_RECEIVED,
        EV_TIMER_SIFS,    // 17
        EV_BROADCAST_RECEIVED,    // 23, 24
        EV_TIMER_CCA
    };

    /** @brief Types for frames sent by the CSMA.*/
    enum t_csma_frame_types {
        DATA,
        ACK
    };

    enum t_mac_carrier_sensed {
        CHANNEL_BUSY = 1,
        CHANNEL_FREE
    };

    enum t_mac_status {
        STATUS_OK = 1,
        STATUS_ERROR,
        STATUS_RX_ERROR,
        STATUS_RX_TIMEOUT,
        STATUS_FRAME_TO_PROCESS,
        STATUS_NO_FRAME_TO_PROCESS,
        STATUS_FRAME_TRANSMITTED
    };

    /** @brief The different back-off methods.*/
    enum backoff_methods {
        /** @brief Constant back-off time.*/
        CONSTANT = 0,
        /** @brief Linear increasing back-off time.*/
        LINEAR,
        /** @brief Exponentially increasing back-off time.*/
        EXPONENTIAL,
    };

    /** @brief keep track of MAC state */
    t_mac_states macState;
    t_mac_status status;

    /** @brief The radio. */
    physicallayer::IRadio *radio;
    physicallayer::IRadio::TransmissionState transmissionState;

    /** @brief Maximum time between a packet and its ACK
     *
     * Usually this is slightly more then the tx-rx turnaround time
     * The channel should stay clear within this period of time.
     */
    simtime_t sifs;

    /** @brief The amount of time the MAC waits for the ACK of a packet.*/
    simtime_t macAckWaitDuration;

    /** @brief Length of the header*/
    int headerLength;

    bool transmissionAttemptInterruptedByRx;
    /** @brief CCA detection time */
    simtime_t ccaDetectionTime;
    /** @brief Time to setup radio from sleep to Rx state */
    simtime_t rxSetupTime;
    /** @brief Time to switch radio from Rx to Tx state */
    simtime_t aTurnaroundTime;
    /** @brief maximum number of extra backoffs (excluding the first unconditional one) before frame drop */
    int macMaxCSMABackoffs;
    /** @brief maximum number of frame retransmissions without ack */
    unsigned int macMaxFrameRetries;
    /** @brief base time unit for calculating backoff durations */
    simtime_t aUnitBackoffPeriod;
    /** @brief Stores if the MAC expects Acks for Unicast packets.*/
    bool useMACAcks;

    /** @brief Defines the backoff method to be used.*/
    backoff_methods backoffMethod;

    /**
     * @brief Minimum backoff exponent.
     * Only used for exponential backoff method.
     */
    int macMinBE;
    /**
     * @brief Maximum backoff exponent.
     * Only used for exponential backoff method.
     */
    int macMaxBE;

    /** @brief initial contention window size
     * Only used for linear and constant backoff method.*/
    int initialCW;

    /** @brief The power (in mW) to transmit with.*/
    double txPower;

    /** @brief number of backoff performed until now for current frame */
    int NB;

    /** @brief count the number of tx attempts
     *
     * This holds the number of transmission attempts for the current frame.
     */
    unsigned int txAttempts;

    /** @brief the bit rate at which we transmit */
    double bitrate;

    /** @brief The bit length of the ACK packet.*/
    int ackLength;

  protected:
    /** @brief Generate new interface address*/
    virtual void configureInterfaceEntry() override;
    virtual void handleCommand(cMessage *msg) {}

    // FSM functions
    void fsmError(t_mac_event event, cMessage *msg);
    void executeMac(t_mac_event event, cMessage *msg);
    void updateStatusIdle(t_mac_event event, cMessage *msg);
    void updateStatusBackoff(t_mac_event event, cMessage *msg);
    void updateStatusCCA(t_mac_event event, cMessage *msg);
    void updateStatusTransmitFrame(t_mac_event event, cMessage *msg);
    void updateStatusWaitAck(t_mac_event event, cMessage *msg);
    void updateStatusSIFS(t_mac_event event, cMessage *msg);
    void updateStatusTransmitAck(t_mac_event event, cMessage *msg);
    void updateStatusNotIdle(cMessage *msg);
    void manageQueue();
    void updateMacState(t_mac_states newMacState);

    void attachSignal(Packet *mac, simtime_t_cref startTime);
    void manageMissingAck(t_mac_event event, cMessage *msg);
    void startTimer(t_mac_timer timer);

    virtual simtime_t scheduleBackoff();

    virtual void decapsulate(Packet *packet);

    Packet *ackMessage;

    //sequence number for sending, map for the general case with more senders
    //also in initialisation phase multiple potential parents
    std::map<MacAddress, unsigned long> SeqNrParent;    //parent -> sequence number

    //sequence numbers for receiving
    std::map<MacAddress, unsigned long> SeqNrChild;    //child -> sequence number

  private:
    /** @brief Copy constructor is not allowed.
     */
    Ieee802154Mac(const Ieee802154Mac&);
    /** @brief Assignment operator is not allowed.
     */
    Ieee802154Mac& operator=(const Ieee802154Mac&);
};

} // namespace inet

#endif // ifndef __INET_IEEE802154MAC_H

