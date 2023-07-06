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

#ifndef __INET_UDPBASICAPP_H
#define __INET_UDPBASICAPP_H

#include <vector>

#include "inet/common/INETDefs.h"

#include "inet/applications/base/ApplicationBase.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"
#include "inet/linklayer/ieee802154/Ieee802154Mac.h"  //CL
#include "inet/routing/rpl/Rpl.h"
#include "inet/physicallayer/common/packetlevel/Radio.h"  //CL 2022-09-12
#include "inet/physicallayer/contract/packetlevel/IRadio.h"


namespace inet {

//The next 3 lines were added by CL so that Radio Module can be recognized
#ifdef WITH_RADIO
using namespace inet::physicallayer;
#endif

/**
 * UDP application. See NED for more info.
 */

class Rpl; //CL to avoid circular dependency
//class Radio; // CL to avoid circular dependency

class INET_API UdpBasicApp : public ApplicationBase, public UdpSocket::ICallback
{
  protected:
    enum SelfMsgKinds { START = 1, SEND, STOP };

    // parameters
    std::vector<L3Address> destAddresses;
    std::vector<std::string> destAddressStr;
    int localPort = -1, destPort = -1;
    simtime_t startTime;
    simtime_t stopTime;
    bool dontFragment = false;
    const char *packetName = nullptr;

    // state
    UdpSocket socket;
    cMessage *selfMsg = nullptr;

    // statistics
    int numSent = 0;
    int numReceived = 0;

    //To get variables from L2   CL 2021-11-09
    cModule *macModule = nullptr;
    Ieee802154Mac *mac = nullptr;

    //To get variables from Rpl module   CL 2021-12-10
    cModule *rplModule = nullptr;
    Rpl *rpl = nullptr;

    //To get variables form Radio module CL 2022-09-12
    //cModule *radioModule = nullptr;
    //Radio *radio = nullptr;


  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void finish() override;
    virtual void refreshDisplay() const override;

    // chooses random destination address
    virtual L3Address chooseDestAddr();
    virtual void sendPacket();
    virtual void processPacket(Packet *msg);
    virtual void setSocketOptions();

    virtual void processStart();
    virtual void processSend();
    virtual void processStop();

    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

    virtual void socketDataArrived(UdpSocket *socket, Packet *packet) override;
    virtual void socketErrorArrived(UdpSocket *socket, Indication *indication) override;
    virtual void socketClosed(UdpSocket *socket) override;

  public:
    UdpBasicApp() {}
    ~UdpBasicApp();

};

} // namespace inet

#endif // ifndef __INET_UDPBASICAPP_H

