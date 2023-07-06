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

#ifndef _OBJECTIVEFUNCTION_H
#define _OBJECTIVEFUNCTION_H

#include <map>
#include <vector>

#include "inet/common/INETDefs.h"
#include "Rpl_m.h"
#include "RplDefs.h"
//#include "Rpl.h"
#include "inet/linklayer/ieee802154/Ieee802154Mac.h"  //CL  2022-01-26: to access L2 layer

#include <fstream>
#include <iostream>
#include <string>

//Related to what I did using socket programming
#include <sstream>
#include <iomanip>
//#include <Python.h>
#include <stdio.h>
#include <winsock.h>

namespace inet {

//class Rpl;  // 2022-04-28: to avoid circular dependency between Rpl.h and ObjectiveFunction.h
//class ObjectiveFunction ;  // 2022-05-03: to avoid circular dependency between Rpl.h and ObjectiveFunction.h

//class ObjectiveFunction : public cObject   //Changed by CL 2022-01-27
class ObjectiveFunction : public cSimpleModule
{
  private:
    Ocp type; /** Objective Function (OF) type as defined in RFC 6551. */
    int minHopRankIncrease; /** base step of rank increment [RFC 6550, 6.7.6] */

    /*************CL*******************/
        struct ParentStructure {
            Ipv6Address SenderIpAddr;
            double Rcvdcounter = 0;       //it is necessary for of_etx to keep record of DIO received
            //std::string ParentName;
        };
    /*************end CL****************/

    //To get variables from L2   CL 2022-01-26
    cModule *host;
    cModule *macModule = nullptr;
    Ieee802154Mac *mac = nullptr;

    //To get variables from Rpl module   CL 2021-12-10
    cModule *rplModule = nullptr;
//    Rpl *rpl = nullptr; //CL to access to Rpl.cc   22-05-03

    //In order to implement EWMA (Exponentially Weighted Moving Average) to calculate ETX
    double etx_prev = 1;
    double alpha = 0.8; //0.8 ; //0.8;

    double thre = 0 ;

    double tie_thre = 0; //2022-05-27


  public:
    ObjectiveFunction();
    ObjectiveFunction(std::string type);

   ~ObjectiveFunction();

    /**
     * Determine node's preferred parent from the candidate neighbor set using
     * relevant metric (defined by OF type).
     *
     * @param candidateParents map of node's neighborhood in form of latest DIO packets
     * from each neighbor
     * @return best parent candidate based on the type of objective function in use
     */

    virtual Dio* getPreferredParent(std::map<Ipv6Address, Dio *> candidateParents, Dio* currentPreferredParent); //back to
                                                                    //this method on 2022-04-29
    //virtual Dio* getPreferredParent(std::map<Ipv6Address, Dio *> candidateParents);  //2022-03-08

    virtual Dio* GetBestCandidate(std::map<Ipv6Address, Dio *> candidateParents);
    double GetBestCandidatePATHCOST (std::map<Ipv6Address, Dio *> candidateParents);

    /**
     * Calculate node's rank based on the chosen preferred parent [RFC 6550, 3.5].
     *
     * @param preferredParent node's preferred parent properties (rank, address, ...)
     * represented by last DIO received from it
     * @return updated rank based on the minHopRankIncrease and OF
     */
    //virtual uint16_t calcRank(Dio* preferredParent);
    virtual double calcRank(Dio* preferredParent);

    void setMinHopRankIncrease(int incr) { minHopRankIncrease = incr; }

    //virtual uint16_t calcTemp_Rank(const Ptr<const Dio>& dio);  //CL
    virtual double calcTemp_Rank(const Ptr<const Dio>& dio);  //CL

    double ETX_Calculator_onLink(const Ptr<const Dio>& dio); //CL: This one is for using with calcTemp_Rank()

    double ETX_Calculator_onLink_calcRank(Dio* preferredParent); //CL: This one is for using with calcRank()

    double Tie_breaker_Calculator(Dio* candidate);  //2022-05-18

    double Path_Cost_Calculator(Dio* candidate); //2022-10-17
    double Path_Cost_Calculator_ptr(const Ptr<const Dio>& dio);

    //Added by CL to cache the count the rcvd msg from specific src Addr
    typedef std::vector<ParentStructure*> CounterCache;
    CounterCache countercache;

    //Methods to normalize:
    virtual double Norm_den (int den);
    virtual double Norm_maclosses (int mac_losses);
    virtual double Norm_fps (double fps );
    virtual double Norm_etx (double etx);

    double ML_rank_calculator(const Ptr<const Dio>& dio);
    double ML_rank_calculator_FORcalcRank(Dio* preferredParent); //CL: This one is for using with calcRank()

    double ML_rank_calculator_SP(const Ptr<const Dio>& dio);
    double ML_rank_calculator_FORcalcRank_SP(Dio* preferredParent); //CL: This one is for using with calcRank()

    //variable of socket connection
    /*    WSADATA wsa;
        SOCKET s;
        struct sockaddr_in server;
        const char *message ;
        char *space = " " ;
        char server_reply [2000];
        int recv_size;
    */
    //to avoid open the socket connection
    int prev_metric1 = 0   ;                   //m1-HC
    double prev_metric2 = 0   ;                   //m2-ETX
    double prev_metric3 =  0  ;                   //m8-Den
    double prev_metric4 = 0;              //m14-den
    int prev_metric5 = 0;                       //m15-etx
    double prev_metric6 = 0;                //SNR
    int prev_metric7 =  0;                      //mac losses from dio
    int prev_metric8 = 0;                 //mac losses from me
    double prev_response = 0;


  protected:
    virtual void initialize() override;  //CL
    void finish() override; //CL
};

} // namespace inet

#endif

