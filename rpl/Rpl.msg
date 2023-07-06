//
//  Simulation model for RPL (Routing Protocol for Low-Power and Lossy Networks)
// 
//  Copyright (C) 2021  Institute of Communication Networks (ComNets),
//                      Hamburg University of Technology (TUHH)
//            (C) 2021  Yevhenii Shudrenko
// 
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
// 
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
// 
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;
import inet.networklayer.common.L3Address;
import inet.networklayer.contract.ipv6.Ipv6Address;
import inet.networklayer.icmpv6.Icmpv6Header;

import inet.common.geometry.Geometry; //CL: for the reason below

cplusplus {{
	#include "inet/routing/rpl/RplDefs.h"
   // #include "inet/common/geometry/common/Coord.h"  //CL:because it was creating conflict with geometric.h, after I moved the code to inet folder
	#include <deque>
	#include <stdint.h>
}}
	
namespace inet;

//class Coord { @existingClass;}    //CL: same reason as above
class cFigure::Color { @existingClass;}

// Objective Code Point - defines objective function of RPL instance
enum Ocp {
    ETX = 0;
    HOP_COUNT = 1;
    ENERGY = 2;
    HC_MOD = 3;
    ML = 4;
    RPL_ENH = 5;
    RPL_ENH2 = 6;
};

enum RplPacketCode {
    DIS = 0;
    DIO = 1;
    DAO = 2;
    DAO_ACK = 3;
    PING = 4;
    PING_ACK = 5;
    CROSS_LAYER_CTRL = 6;
};


//
// RPL packets
//

class RplHeader extends Icmpv6Header {
	RplPacketCode icmpv6Code;
	const uint8_t icmpv6Type = 155;
}

class RplPacket extends FieldsChunk {
    Ipv6Address destAddress;   		
    Ipv6Address srcAddress;
    uint8_t instanceId;        	  	// RPL instance ID
    Ipv6Address dodagId;            // IPv6 address of the DODAG root
	uint64_t nodeId; 				// Node's MAC (temp. workaround for cross-layer 6TiSCH)
}

// DODAG Information Option [RFC 6550, 6.3]
class Dio extends RplPacket {
    // DODAG version number, may be incremented by the root to trigger topology refresh 
    uint8_t dodagVersion;       	
	//uint16_t rank;            	 	// Node's rank within the DODAG 
	double rank;            	 	// CL 2022-01-31
    bool storing;					// Mode of operation: storing / non-storing                
    bool grounded;              	// DODAG grounded flag, indicates whether sink is connected to backbone	   
    
    // Destination Advertisement Trigger Sequence Number, 
    // may be incremented to trigger downward routes update
    uint8_t dtsn;
    uint8_t dioId;  // temporary field for debugging purposes   
   
	// Trickle timer parameters (DAG Configuration Option)
    int minInterval;           
    int dioRedundancyConst;              
    int dioNumDoublings;						
    Ocp ocp;                
    
    // Non-RFC fields, misc
    Coord position; // DIO sender node location to draw directed parent-child connectors on canvas
    cFigure::Color color; // Color of the parent-child connector line, per DODAG
	int colorId; // DODAG color id (from static palette) for multi-GW scenario
	
	// Created by CL
	int numDIO;
    // add here other metrics
    int HC;
    double ETX;   //put double on 2022-01-31
    //int DROP;  //total frame dropped: it was used for testing 
    //add any other from L2  (15-02-2022)
    //int dropT;   //nbDroppedFrames in L2
    int dropB;   //Frames drop by Backoff (congestion)
    int dropR;   //Frames drop by Retry (collisions or channel interferences)
    int missACK; //nbMissed ACKs
    double txF ;    //mac-fail-rate-retry. It was before: nbTxFrames  
    double rxF;     //mac-fail-rate by congestio. It was before nbRxFrames
    double bw; //bandwidth, pending of implementing //channel utilization 2022-02-23
    int den;
    double qu;
    double fps; // (Rx Frames + Tx Frames)/simTime    2022-05-05
    double snr; //2022-09-12
    int rx_un_suc; //2022-09-29
    double rx_suc_rate; //2022-09-29
    
    double path_cost; //2022-10-17
    simtime_t last_update;  //2022-10-31
    int numDIOrx; //2022-04-11
    
}

// Destination Advertisement Object [RFC 6550, 6.4] 
class Dao extends RplPacket {
    uint8_t seqNum;				// ID for each unique DAO sent by a node
    bool daoAckRequired;		// indicates whether DAO-ACK is expected by the sender 
    Ipv6Address reachableDest;	// advertised reachable destination		
    uint8_t chOffset;			// advertised channel offset (unique per branch)
    						    // as part of cross-layer scheduling 
}

cplusplus (Dao) {{
	std::vector<Ipv6Address> knownTargets; 
	
	std::vector<Ipv6Address> getKnownTargets() { return this->knownTargets; }
	void setKnownTargets(std::vector<Ipv6Address> knownTargets) { handleChange(); this->knownTargets = knownTargets; }
}}

// DODAG Information Solicitation
class Dis extends RplPacket {

}

// RPL Information Packet header [RFC 6550 11.2]
class RplPacketInfo extends RplPacket {
    bool down;
    bool rankError;
    bool fwdError;
    uint16_t senderRank;
}


class RplTargetInfo extends RplPacket {
    Ipv6Address target;
    // tbd fields according to 
}

class RplTransitInfo extends RplPacket {
    Ipv6Address transit;
    // tbd fields
}



class SourceRoutingHeader extends FieldsChunk {	
}

cplusplus (SourceRoutingHeader) {{
    std::deque<Ipv6Address> addresses;
    
    std::deque<Ipv6Address> getAddresses() { return this->addresses; }
    void setAddresses(std::deque<Ipv6Address> hopList) { handleChange(); this->addresses = hopList; }	
}}



