//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
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

#include "inet/applications/udpapp/UdpSink.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/transportlayer/contract/udp/UdpControlInfo_m.h"

#include <iostream>
#include <string>
#include <fstream>
#include "inet/networklayer/common/HopLimitTag_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"

namespace inet {

Define_Module(UdpSink);

UdpSink::~UdpSink()
{
    cancelAndDelete(selfMsg);
}

void UdpSink::initialize(int stage)
{
    ApplicationBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        numReceived = 0;
        WATCH(numReceived);

        localPort = par("localPort");
        startTime = par("startTime");
        stopTime = par("stopTime");
        if (stopTime >= SIMTIME_ZERO && stopTime < startTime)
            throw cRuntimeError("Invalid startTime/stopTime parameters");
        selfMsg = new cMessage("UDPSinkTimer");
    }
}

void UdpSink::handleMessageWhenUp(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        ASSERT(msg == selfMsg);
        switch (selfMsg->getKind()) {
            case START:
                processStart();
                break;

            case STOP:
                processStop();
                break;

            default:
                throw cRuntimeError("Invalid kind %d in self message", (int)selfMsg->getKind());
        }
    }
    else if (msg->arrivedOn("socketIn"))
        socket.processMessage(msg);
    else
        throw cRuntimeError("Unknown incoming gate: '%s'", msg->getArrivalGate()->getFullName());
}

void UdpSink::socketDataArrived(UdpSocket *socket, Packet *packet)
{
    // process incoming packet
    processPacket(packet);
}

void UdpSink::socketErrorArrived(UdpSocket *socket, Indication *indication)
{
    EV_WARN << "Ignoring UDP error report " << indication->getName() << endl;
    delete indication;
}

void UdpSink::socketClosed(UdpSocket *socket)
{
    if (operationalState == State::STOPPING_OPERATION)
        startActiveOperationExtraTimeOrFinish(par("stopOperationExtraTime"));
}

void UdpSink::refreshDisplay() const
{
    ApplicationBase::refreshDisplay();

    char buf[50];
    sprintf(buf, "rcvd: %d pks", numReceived);
    getDisplayString().setTagArg("t", 0, buf);
}

void UdpSink::finish()
{
    ApplicationBase::finish();
    EV_INFO << getFullPath() << ": received " << numReceived << " packets\n";
}

void UdpSink::setSocketOptions()
{
    bool receiveBroadcast = par("receiveBroadcast");
    if (receiveBroadcast)
        socket.setBroadcast(true);

    MulticastGroupList mgl = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this)->collectMulticastGroups();
    socket.joinLocalMulticastGroups(mgl);

    // join multicastGroup
    const char *groupAddr = par("multicastGroup");
    multicastGroup = L3AddressResolver().resolve(groupAddr);
    if (!multicastGroup.isUnspecified()) {
        if (!multicastGroup.isMulticast())
            throw cRuntimeError("Wrong multicastGroup setting: not a multicast address: %s", groupAddr);
        socket.joinMulticastGroup(multicastGroup);
    }
    socket.setCallback(this);
}

void UdpSink::processStart()
{
    socket.setOutputGate(gate("socketOut"));
    socket.bind(localPort);
    setSocketOptions();

    if (stopTime >= SIMTIME_ZERO) {
        selfMsg->setKind(STOP);
        scheduleAt(stopTime, selfMsg);
    }
}

void UdpSink::processStop()
{
    if (!multicastGroup.isUnspecified())
        socket.leaveMulticastGroup(multicastGroup); // FIXME should be done by socket.close()
    socket.close();
}

void UdpSink::processPacket(Packet *pk)
{
    EV_INFO << "Received packet: " << UdpSocket::getReceivedPacketInfo(pk) << endl;
    emit(packetReceivedSignal, pk);

    //CL code
    //Record IP address and amount of hops when a pkt reaches the destination
        L3Address src = pk->getTag<L3AddressInd>()->getSrcAddress();
        int hop = pk->getTag<HopLimitInd>()->getHopLimit();
        hop = 32 - hop + 1; //Based on initial TTL=32. Also, +1 b/c IP does not decrement in the first hop, but does not forward with TTL =1
        std::ofstream file;
        file.open("St_deviceID.txt", std::ios_base::app);
        file << src << " "<< hop << endl;
        file.close();
    //Record the packet when arrives to the destination
        file.open("St_packet-tracer.txt", std::ios_base::app);
        std::string packetName = pk->getName();
        int packetSize = pk->getByteLength();

        //To get variables from other places
        //macModule = getParentModule()->getSubmodule("wlan",0)->getSubmodule("mac");
        //mac = check_and_cast<Ieee802154Mac *>(macModule);

        //file << simTime() <<" " <<getParentModule()->getFullName() << " pkt_rcv " << packetName << " "<< packetSize <<endl;
        //file.close();

 /*       file << simTime() <<" " <<getParentModule()->getFullName() << " pkt_rcv " << packetName <<" " << packetSize <<" "<<
        mac->nbTxFrames_copy<< " "<< mac->nbRxFrames_copy << " " << mac->retry_limit_reached_copy << " " <<
        mac->framedropbycongestion_copy << " " << mac->nbMissedAcks_copy << " " << mac->nbDroppedFrames_copy << " " << "1" <<endl;
 */
   //     file << simTime() <<" " <<getParentModule()->getFullName() << " pkt_rcv " << packetName << endl;

        file << simTime() <<" " << getParentModule()->getFullName() << " pkt_rcv " << packetName <<" " <<
                 "0" << " " << "0" << " " << "0"<< " " << "0" << " " <<
                 "0" << " " << "0" << " " << "0" << " " << "0" << " " <<
                 "0" << " " << "0" << " " << "0" << " " << "0" << " " <<
                 "0" << " " << "0" << " " << "0" << " " << "0" << " " << "0" << " " << "0" << " "
                 "0" << " " << "0" << endl ;

        file.close();

    delete pk;

    numReceived++;
}

void UdpSink::handleStartOperation(LifecycleOperation *operation)
{
    simtime_t start = std::max(startTime, simTime());
    if ((stopTime < SIMTIME_ZERO) || (start < stopTime) || (start == stopTime && startTime == stopTime)) {
        selfMsg->setKind(START);
        scheduleAt(start, selfMsg);
    }
}

void UdpSink::handleStopOperation(LifecycleOperation *operation)
{
    cancelEvent(selfMsg);
    if (!multicastGroup.isUnspecified())
        socket.leaveMulticastGroup(multicastGroup); // FIXME should be done by socket.close()
    socket.close();
    delayActiveOperationFinish(par("stopOperationTimeout"));
}

void UdpSink::handleCrashOperation(LifecycleOperation *operation)
{
    cancelEvent(selfMsg);
    if (operation->getRootModule() != getContainingNode(this)) {     // closes socket when the application crashed only
        if (!multicastGroup.isUnspecified())
            socket.leaveMulticastGroup(multicastGroup); // FIXME should be done by socket.close()
        socket.destroy();    //TODO  in real operating systems, program crash detected by OS and OS closes sockets of crashed programs.
    }
}

} // namespace inet

