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

#include "inet/routing/rpl/ObjectiveFunction.h"
#include <fstream>
#include <iostream>
#include <string>

#include <winsock.h>

//#include "inet/routing/rpl/Rpl.h" // 2022-04-28 to avoid circular dependency
#include "inet/routing/rpl/Catboost.h"

namespace inet {

Define_Module(ObjectiveFunction);  //added by CL 2022-01-27

ObjectiveFunction::ObjectiveFunction() :
    type(HOP_COUNT),
    minHopRankIncrease(0)
{}

ObjectiveFunction::ObjectiveFunction(std::string objFunctionType) {
    if (objFunctionType.compare(std::string("ETX")) == 0)
        type = ETX;
    else if (objFunctionType.compare(std::string("HC_MOD")) == 0)
        type = HC_MOD;
    else if (objFunctionType.compare(std::string("HOP_COUNT")) == 0)
        type = HOP_COUNT;
    else if (objFunctionType.compare(std::string("ML")) == 0)
        type = ML;
    else if (objFunctionType.compare(std::string("RPL_ENH")) == 0)
        type = RPL_ENH;
    else if (objFunctionType.compare(std::string("RPL_ENH2")) == 0)
        type = RPL_ENH2;
    else
        type = ENERGY;
    EV_DETAIL << "Objective function initialized with type - " << objFunctionType << endl;
}

ObjectiveFunction::~ObjectiveFunction() {
    //cancelAndDelete();
}

Dio* ObjectiveFunction::getPreferredParent(std::map<Ipv6Address, Dio *> candidateParents, Dio* currentPreferredParent) {
//2022-03-08: I do not need to pass currentPreferredParent, Yes, I think I need it (2022-04-29)
//Dio* ObjectiveFunction::getPreferredParent(std::map<Ipv6Address, Dio *> candidateParents) {

//    std::ofstream file;  // CL 2022-02-02
//    file.open("St_PrefParentChanges.txt", std::ios_base::app);
    //file << simTime() <<" " << getParentModule()->getParentModule()->getFullName() << " 1"<< endl;

    // determine parent with lowest rank
    if (candidateParents.empty()) {
        EV_WARN << "Couldn't determine preferred parent, provided set is empty" << endl;
        return nullptr;
    }

    EV_DETAIL << "List of candidate parents: "<<endl;  //CL
    for (auto cp : candidateParents){
        EV_DETAIL << cp.first << " - " << "Ranking: " <<cp.second->getRank() <<" - " << "path_cost: " <<cp.second->getPath_cost() << " - " << "last_update: " << cp.second->getLast_update() << endl;
    }

    //CL : this part is going to be different depending on the OF    //2022-02-02
    Dio *newPrefParent = candidateParents.begin()->second;
    switch (type){
         case HOP_COUNT:{
             //Dio *newPrefParent = candidateParents.begin()->second;
             //uint16_t currentMinRank = newPrefParent->getRank();
             EV_INFO << "I am inside getPreferredParent - case: HOP_COUNT" << endl;
             double currentMinRank = newPrefParent->getRank() ;         //Changed by CL 2022-02-01
             for (std::pair<Ipv6Address, Dio *> candidate : candidateParents) {
                 //uint16_t candidateParentRank = candidate.second->getRank();
                 double candidateParentRank = candidate.second->getRank();   ////Changed by CL 2022-02-01
                     if (candidateParentRank < currentMinRank) {
                         currentMinRank = candidateParentRank;
                         newPrefParent = candidate.second;
                    }
                 }
             if (currentMinRank == currentPreferredParent->getRank()){
             //if (newPrefParent->getRank() == currentPreferredParent->getRank()){
                 EV_INFO << "Preferred parent did not change b/c the new one has the same ranking"  << endl ;
                 return currentPreferredParent;
             }else{
                 return newPrefParent;
             }
//2022-03-08:
/*             if (!currentPreferredParent){
                    //Always enter here when the node receives the first DIO
                    EV_INFO << "I am inside if (!currentPreferredParent)_HOPcount"  << endl;
                    return newPrefParent;
                }

                if (currentPreferredParent->getRank() - newPrefParent->getRank() >= minHopRankIncrease){
                    return newPrefParent;
                }else{
                    return currentPreferredParent;
                }

*/
         }case ETX:{
            EV_INFO << "I am inside getPreferredParent - case: ETX" << endl;
            //Dio *newPrefParent = candidateParents.begin()->second;
            //uint16_t currentMinRank = newPrefParent->getRank();
            double currentMinRank = newPrefParent->getRank() + ETX_Calculator_onLink_calcRank(newPrefParent); //Changed by CL 2022-02-02
            for (std::pair<Ipv6Address, Dio *> candidate : candidateParents) {
                //uint16_t candidateParentRank = candidate.second->getRank();
                double candidateParentRank = candidate.second->getRank() + ETX_Calculator_onLink_calcRank(candidate.second) ;   ////Changed by CL 2022-02-02
                if ((candidateParentRank + thre) < currentMinRank) {
                    currentMinRank = candidateParentRank;
                    newPrefParent = candidate.second;
                   }
            }
            if (currentMinRank == currentPreferredParent->getRank()){
            //if (newPrefParent->getRank() + thre >= currentPreferredParent->getRank()){
                EV_INFO << "Preferred parent did not change b/c the new one has the same ranking"  << endl ;
                return currentPreferredParent;
            }else{
                return newPrefParent;
            }

         }case HC_MOD:{ //2022-04-29

             EV_INFO << "I am inside getPreferredParent - case: HC_MOD" << endl;

             //To get variables from Rpl.........2022-05-03
             //rplModule = getParentModule();
             //rpl = check_and_cast<Rpl *>(rplModule);

             //double currentMinRank = newPrefParent->getRank() + ETX_Calculator_onLink_calcRank(newPrefParent); //Changed by CL 2022-02-02
             double currentMinRank = newPrefParent->getRank() ;

             double current_tie_breaker_score = Tie_breaker_Calculator(newPrefParent);
             EV_INFO <<"current_tie_breaker_score = " << current_tie_breaker_score << endl;

             for (std::pair<Ipv6Address, Dio *> candidate : candidateParents) {
                 EV_INFO << " ...... here the for begins ...." << endl;
                 //uint16_t candidateParentRank = candidate.second->getRank();
                 //double candidateParentRank = candidate.second->getRank() + ETX_Calculator_onLink_calcRank(candidate.second) ;   ////Changed by CL 2022-02-02
                 double candidateParentRank = candidate.second->getRank();

                 double candidate_tie_breaker_score = Tie_breaker_Calculator(candidate.second);
                 EV_INFO <<"candidate_tie_breaker_score = " << candidate_tie_breaker_score << endl;

                 if (candidateParentRank + thre < currentMinRank){
                     currentMinRank = candidateParentRank;
                     current_tie_breaker_score = candidate_tie_breaker_score;
                     newPrefParent = candidate.second;
                 }
                 if (candidateParentRank == currentMinRank){
                     //(currentMinRank - candidateParentRank <= thre & currentMinRank - candidateParentRank >= 0)
                     //count how many times this happens
                     std::ofstream file;
                     file.open("St_tie-breaker-counter.txt", std::ios_base::app);
                     file << simTime() << " " << getParentModule()->getParentModule()->getFullName()<< " "
                          << current_tie_breaker_score << " " << candidate_tie_breaker_score <<  endl;
                     if (candidate_tie_breaker_score + tie_thre < current_tie_breaker_score){
                         current_tie_breaker_score = candidate_tie_breaker_score;
                         currentMinRank = candidateParentRank;
                         newPrefParent = candidate.second;
                     }
                 }
            }
             EV_INFO << " ... End of the for ..." << endl ;

             double currentPrefParent_tie_breaker_score = Tie_breaker_Calculator(currentPreferredParent);
             if (currentMinRank == currentPreferredParent->getRank() & current_tie_breaker_score == currentPrefParent_tie_breaker_score ){
                 EV_INFO << "Preferred parent did not change b/c the new one has the same ranking and tie-breaker score"  << endl ;
                 return currentPreferredParent;
             }else{
                 return newPrefParent;
             }
         }case ML:{
             EV_INFO << "I am inside getPreferredParent - case: ML" << endl;
             //Dio *newPrefParent = candidateParents.begin()->second;
             //uint16_t currentMinRank = newPrefParent->getRank();
             double currentMinRank = newPrefParent->getRank() + ML_rank_calculator_FORcalcRank(newPrefParent); //Changed by CL 2022-02-02
             for (std::pair<Ipv6Address, Dio *> candidate : candidateParents) {
                //uint16_t candidateParentRank = candidate.second->getRank();
                double candidateParentRank = candidate.second->getRank() + ML_rank_calculator_FORcalcRank(candidate.second) ;   ////Changed by CL 2022-02-02
                if ((candidateParentRank + thre) < currentMinRank) {
                   currentMinRank = candidateParentRank;
                   newPrefParent = candidate.second;
                }
             }
             if (currentMinRank == currentPreferredParent->getRank()){
                //if (newPrefParent->getRank() + thre >= currentPreferredParent->getRank()){
                EV_INFO << "Preferred parent did not change b/c the new one has the same ranking"  << endl ;
                return currentPreferredParent;
             }else{
                return newPrefParent;
             }

         }
         case RPL_ENH:{

             EV_INFO << "I am inside getPreferredParent - case: RPL_ENH" << endl;

             double currentMinRank = newPrefParent->getRank() ;

             double current_path_cost_score;

             double candidate_path_cost_score;

             if (simTime() - newPrefParent->getLast_update() > 600)
                 candidate_path_cost_score = newPrefParent->getHC() + 1;
             else
                 candidate_path_cost_score = newPrefParent->getPath_cost();

             current_path_cost_score = candidate_path_cost_score + Path_Cost_Calculator(newPrefParent);

             EV_INFO <<"Path cost = " << current_path_cost_score << endl;

                          for (std::pair<Ipv6Address, Dio *> candidate : candidateParents) {
                              EV_INFO << " ...... here the for begins ...." << endl;
                              //uint16_t candidateParentRank = candidate.second->getRank();
                              //double candidateParentRank = candidate.second->getRank() + ETX_Calculator_onLink_calcRank(candidate.second) ;   ////Changed by CL 2022-02-02
                              double candidateParentRank = candidate.second->getRank();

                              double candidate_path_cost_score;
                              //I can introduce here an 'if' to give a path_cost = 1 (max possible value) to the candidates with last_update > x
                              //Although maybe it is better to pass that feature to the path cost calculator
                              if (simTime() - candidate.second->getLast_update() > 600)
                                  candidate_path_cost_score = candidate.second->getHC() + 1 + Path_Cost_Calculator(candidate.second);
                              else
                                  candidate_path_cost_score = candidate.second->getPath_cost() + Path_Cost_Calculator(candidate.second);

                              EV_INFO <<"candidate_path_cost_score = " << candidate_path_cost_score << endl;

                              if (candidateParentRank + thre < currentMinRank){
                                  currentMinRank = candidateParentRank;
                                  current_path_cost_score = candidate_path_cost_score;
                                  newPrefParent = candidate.second;
                              }
                              if (candidateParentRank == currentMinRank){
                                  //(currentMinRank - candidateParentRank <= thre & currentMinRank - candidateParentRank >= 0)
                                  //count how many times this happens
                                  std::ofstream file;
                                  file.open("St_tie-breaker-counter.txt", std::ios_base::app);
                                  file << simTime() << " " << getParentModule()->getParentModule()->getFullName()<< " "
                                       << current_path_cost_score << " " << candidate_path_cost_score <<  endl;
                                  if (candidate_path_cost_score + tie_thre < current_path_cost_score){
                                      current_path_cost_score = candidate_path_cost_score;
                                      currentMinRank = candidateParentRank;
                                      newPrefParent = candidate.second;
                                  }
                              }
                         }
                          EV_INFO << " ... End of the for ..." << endl ;

                          double currentPrefParent_path_cost_score = currentPreferredParent->getPath_cost() + Path_Cost_Calculator(currentPreferredParent);
                          if (currentMinRank == currentPreferredParent->getRank() & current_path_cost_score == currentPrefParent_path_cost_score ){
                              EV_INFO << "Preferred parent did not change b/c the new one has the same ranking and tie-breaker score"  << endl ;
                              return currentPreferredParent;
                          }else{
                              return newPrefParent;
                          }

         }
         case RPL_ENH2:{
                      //Dio *newPrefParent = candidateParents.begin()->second;
                      //uint16_t currentMinRank = newPrefParent->getRank();
                      EV_INFO << "I am inside getPreferredParent - case: " << endl;
                      double currentMinRank = newPrefParent->getRank() ;         //Changed by CL 2022-02-01
                      for (std::pair<Ipv6Address, Dio *> candidate : candidateParents) {
                          //uint16_t candidateParentRank = candidate.second->getRank();
                          double candidateParentRank = candidate.second->getRank();   ////Changed by CL 2022-02-01
                              if (candidateParentRank < currentMinRank) {
                                  currentMinRank = candidateParentRank;
                                  newPrefParent = candidate.second;
                             }
                          }
                      if (currentMinRank == currentPreferredParent->getRank()){
                      //if (newPrefParent->getRank() == currentPreferredParent->getRank()){
                          EV_INFO << "Preferred parent did not change b/c the new one has the same ranking"  << endl ;
                          return currentPreferredParent;
                      }else{
                          return newPrefParent;
                      }

    }

/*    Dio *newPrefParent = candidateParents.begin()->second;
    //uint16_t currentMinRank = newPrefParent->getRank();
    double currentMinRank = newPrefParent->getRank();         //Changed by CL 2022-02-01
    for (std::pair<Ipv6Address, Dio *> candidate : candidateParents) {
        //uint16_t candidateParentRank = candidate.second->getRank();
        double candidateParentRank = candidate.second->getRank();   ////Changed by CL 2022-02-01
        if (candidateParentRank < currentMinRank) {
            currentMinRank = candidateParentRank;
            newPrefParent = candidate.second;
//            break;
        }
    }  */

//2022-03-08
/*
    if (!currentPreferredParent){
        //file.open("St_PrefParentChanges.txt", std::ios_base::app);
        //file << simTime() <<" " << getParentModule()->getParentModule()->getFullName() << " 2"<< endl;
        //Always enter here when the node receives the first DIO
        EV_INFO << "I am inside if (!currentPreferredParent)"  << endl;
        return newPrefParent;
    }
    //CL 2022-02-01
    EV_INFO << "rank respect to preferred parent = " << currentPreferredParent->getRank() + ETX_Calculator_onLink_calcRank(currentPreferredParent) << endl;
    EV_INFO << "my pref parent is: " << currentPreferredParent->getSrcAddress() << endl;
    EV_INFO << "rank respect to the possible new pref parent = " << newPrefParent->getRank() + ETX_Calculator_onLink_calcRank(newPrefParent) << endl;
    EV_INFO << "my possible new pref parent is: " << newPrefParent->getSrcAddress() << endl;
    //EV_INFO << "newPrefParentRank - currentPreferredParentRank = " << currentPreferredParent->getRank() - newPrefParent->getRank() << endl;
    //EV_INFO << "minHopRankIncrease = " << minHopRankIncrease << endl;
    //end CL

    //if (currentPreferredParent->getRank() - newPrefParent->getRank() >= minHopRankIncrease){
    if (currentPreferredParent->getRank() + ETX_Calculator_onLink_calcRank(currentPreferredParent) - newPrefParent->getRank() - ETX_Calculator_onLink_calcRank(newPrefParent)>0){
        //to record every time the node changes prefer parent
        file << simTime() <<" " << getParentModule()->getParentModule()->getFullName() << "My rank has improved and I'll switch to another parent" << endl;
        return newPrefParent;
    }else{
        //file.open("St_PrefParentChanges.txt", std::ios_base::app);
        //Two options: My rank respect to my pref parent is worse, but it continues being my pref parent because there is no better
        //parent or the threshold between the newPrefParent and the current one was not reached
        file << simTime() <<" " << getParentModule()->getParentModule()->getFullName() << "No changes in the pref parent "<< endl;
        return currentPreferredParent;
    }
    file.close();
*/
}
}

double ObjectiveFunction::Tie_breaker_Calculator(Dio* candidate) {

      double w1 = 0.51; //0.48; //0.76;
      double w2 = 0.14; //0.12; //0.12;
      double w3 = 0.14; //0.25; //0.07;
      double w4 = 0.08; //0.09; //0.05;

      double tie_breaker_score = w1*(Norm_etx(ETX_Calculator_onLink_calcRank(candidate))) + w2*(Norm_fps(candidate->getFps())) + w3*(Norm_maclosses(candidate->getDropB() + candidate->getDropR())) + w4*(candidate->getBw());

      //double current_tie_breaker_score = 0.30*(Norm_den(newPrefParent->getDen())) + 0.25*(Norm_maclosses(newPrefParent->getDropB() + newPrefParent->getDropR())) + 0.16*(newPrefParent->getBw());

      tie_breaker_score = round(tie_breaker_score*100)/100.0;  //2022-05-04

      return tie_breaker_score;
}

double ObjectiveFunction::Path_Cost_Calculator(Dio* candidate) {

    EV_INFO << "I am inside Path_Cost_Calculator" << endl;

    double path_cost; // = 0;

    //If the DIO comes from the root the path cost is going to be only the etx of the link
    //to know if the DIO comes from the root, I'll check if the rank == 1
/*    if ( candidate->getRank() == 1){
        EV_INFO << "My pref parent is the root" << endl;
        path_cost = ETX_Calculator_onLink_calcRank(candidate);
    }else {
        //If the DIO comes from another node, I'll apply ML to calculate the path cost:
        //First, I test the entire implementation with the ETX
        path_cost = candidate->getPath_cost() + ETX_Calculator_onLink_calcRank(candidate);
    }
*/
/*
    if(simTime() < 1200)
        path_cost = 0;  //candidate->getPath_cost() + 1;
    else{
        if ( candidate->getRank() == 1)
            path_cost = 0;
        else
            path_cost = ML_rank_calculator_FORcalcRank (candidate);
    }
*/
    if(simTime() < 10000)
        path_cost = 0;   //max value, I was considering zero
    else{
        //path_cost = candidate->getPath_cost() + ML_rank_calculator_FORcalcRank (candidate);  //11/03/2022
        path_cost = ML_rank_calculator_FORcalcRank (candidate);  //11/08/2022
    }
    //path_cost = ML_rank_calculator_FORcalcRank (candidate);  //11/08/2022

    return path_cost;
}

double ObjectiveFunction::Path_Cost_Calculator_ptr(const Ptr<const Dio>& dio) {

    EV_INFO << "I am inside Path_Cost_Calculator" << endl;

    double path_cost; // = 0;

    //If the DIO comes from the root the path cost is going to be only the etx of the link
    //to know if the DIO comes from the root, I'll check if the rank == 1
/*    if ( dio->getRank() == 1)
        path_cost = ETX_Calculator_onLink(dio);
    else {
        //If the DIO comes from another node, I'll apply ML to calculate the path cost:
        //First, I test the entire implementation with the ETX
        path_cost = dio->getPath_cost() + ETX_Calculator_onLink(dio);
    }
*/
/*
    if(simTime() < 1200)
        path_cost = 0;  //candidate->getPath_cost() + 1;
    else{
        if ( dio->getRank() == 1)
            path_cost = 0;
        else
            path_cost = ML_rank_calculator(dio);
    }
*/

    if(simTime() < 10000)
        path_cost = 0;
    else{
        //path_cost = dio->getPath_cost() + ML_rank_calculator(dio);  //11/03/2022
        path_cost = ML_rank_calculator(dio);  //11/08/2022
    }
    //path_cost = ML_rank_calculator(dio);  //11/08/2022

    return path_cost;
}

//uint16_t ObjectiveFunction::calcRank(Dio* preferredParent) {
double ObjectiveFunction::calcRank(Dio* preferredParent) {
    if (!preferredParent)
        throw cRuntimeError("Cannot calculate rank, preferredParent argument is null");

    //uint16_t prefParentRank = preferredParent->getRank();
    double prefParentRank = preferredParent->getRank();
    /** Calculate node's rank based on the objective function policy */
    switch (type) {
        case HOP_COUNT:
            //rpl->setHC(prefParentRank + 1);
            //maybe include the calculation of the ETX metric   (how?)
            return prefParentRank + 1;
        case ETX:  //Modified by CL 2021-11-02
            EV_INFO <<"I am inside calcRank using ETX OF" << endl;
            //EV_INFO <<"ETX_Calculator_onLink_calcRank(preferredParent) = "<< ETX_Calculator_onLink_calcRank(preferredParent)<<endl;
            EV_INFO <<"prefParentRank = "<< prefParentRank <<endl;
            //rpl->setETX(prefParentRank + ETX_Calculator_onLink_calcRank(preferredParent));
            //maybe include the calculation of the hc metric  (how?)
            return (prefParentRank + ETX_Calculator_onLink_calcRank(preferredParent) + 1); //What I have to do is two different ETX calculator on link
        case HC_MOD:  //2022-04-30 ... this part is same as ETX
            EV_INFO <<"I am inside calcRank using ETX OF_MOD" << endl;
                    //EV_INFO <<"ETX_Calculator_onLink_calcRank(preferredParent) = "<< ETX_Calculator_onLink_calcRank(preferredParent)<<endl;
                    EV_INFO <<"prefParentRank = "<< prefParentRank <<endl;
                    //rpl->setETX(prefParentRank + ETX_Calculator_onLink_calcRank(preferredParent));
                    //maybe include the calculation of the hc metric  (how?)
                    return prefParentRank + 1;
                    //return (prefParentRank + ETX_Calculator_onLink_calcRank(preferredParent)); //What I have to do is two different ETX calculator on link
        case ML: //2022-07-06
            EV_INFO << "I am inside calcRank using ML prediction" << endl;
            EV_INFO <<"prefParentRank = "<< prefParentRank <<endl;
            return (prefParentRank + ML_rank_calculator_FORcalcRank(preferredParent));
        case RPL_ENH:
            return prefParentRank + 1;
        case RPL_ENH2:
            return prefParentRank + 1;
        default:
            return prefParentRank + DEFAULT_MIN_HOP_RANK_INCREASE;
    }
}

//CL
//uint16_t ObjectiveFunction::calcTemp_Rank(const Ptr<const Dio>& dio) {
double ObjectiveFunction::calcTemp_Rank(const Ptr<const Dio>& dio) {
    //if (!preferredParent)
        //throw cRuntimeError("Cannot calculate rank, preferredParent argument is null");

    //This part is only to test if I can read the amount of ack received from the dio sender
    //macModule = getParentModule()->getParentModule()->getSubmodule("wlan",0)->getSubmodule("mac");
    //auto mac = check_and_cast<Ieee802154Mac *>(macModule);
    //EV_INFO << "trying to get into mac from OF" << mac->countercache_L2 << endl;
    //

    //uint16_t Dio_sender_rank = dio->getRank();
    double Dio_sender_rank = dio->getRank();
    type = dio->getOcp();
    /** Calculate node's rank based on the objective function policy */
    switch (type) {
        case HOP_COUNT:
            return Dio_sender_rank + 1;
        case ETX:
            return (Dio_sender_rank + ETX_Calculator_onLink(dio) + 1);
        case HC_MOD:
            //return (Dio_sender_rank + ETX_Calculator_onLink(dio));
            return Dio_sender_rank + 1;
        case ML:
            return (Dio_sender_rank + ML_rank_calculator(dio));
        case RPL_ENH:
            return Dio_sender_rank + 1;
        case RPL_ENH2:
            return Dio_sender_rank + 1;
        default:
            return Dio_sender_rank + DEFAULT_MIN_HOP_RANK_INCREASE;
    }
}
//Part of the ETX implementation, different from previous ETX implementation, I'll use ACKs to calculate the metric value
double ObjectiveFunction::ETX_Calculator_onLink(const Ptr<const Dio>& dio){

    uint64_t nodeId = dio->getNodeId();

    //I need a method in Ieee802154Mac that returns ACK received and missed
    macModule = getParentModule()->getParentModule()->getSubmodule("wlan",0)->getSubmodule("mac");
    auto mac = check_and_cast<Ieee802154Mac *>(macModule);
    EV_INFO << "I am inside ETX_Calculator_onLink" << endl;
    EV_INFO << "Reading from OF ACKrcv, ACKrcv =  " << mac->getACKrcv(nodeId) << endl;
    EV_INFO << "Reading from OF ACKmissed, ACKmissed = " << mac->getACKmissed(nodeId) << endl;

    EV_INFO << "OF parent module : " << getParentModule() << endl;

    if (mac->getACKrcv(nodeId)==0 & mac->getACKmissed(nodeId) ==0){
        etx_prev = 1;
        return 1;
    }if (mac->getACKrcv(nodeId) ==0){
        etx_prev = 100;
        return 100; //a very high ETX value
    }
    double etx = alpha*((mac->getACKmissed(nodeId) + mac->getACKrcv(nodeId))/mac->getACKrcv(nodeId)) + (1 - alpha)*etx_prev;
    etx = round(etx*100)/100.0;  //2022-05-04
    etx_prev = etx;

    return etx;

    //return ((mac->getACKmissed(nodeId) + mac->getACKrcv(nodeId))/mac->getACKrcv(nodeId)) ;

}

double ObjectiveFunction::ETX_Calculator_onLink_calcRank(Dio* preferredParent){

    uint64_t nodeId = preferredParent->getNodeId();

        //I need a method in Ieee802154Mac that returns ACK received and missed
        macModule = getParentModule()->getParentModule()->getSubmodule("wlan",0)->getSubmodule("mac");
        auto mac = check_and_cast<Ieee802154Mac *>(macModule);
        EV_INFO << "I am inside ETX_Calculator_onLink_calcRank" << endl;
        EV_INFO << "Reading from OF ACKrcv: " << mac->getACKrcv(nodeId) << endl;
        EV_INFO << "Reading from OF ACKmissed: " << mac->getACKmissed(nodeId) << endl;

        if (mac->getACKrcv(nodeId)==0 & mac->getACKmissed(nodeId) ==0){
            etx_prev = 1;
            return 1;
        }if (mac->getACKrcv(nodeId) ==0){
            etx_prev = 100;
            return 100; //a very high ETX value
        }
        double etx = alpha*((mac->getACKmissed(nodeId) + mac->getACKrcv(nodeId))/mac->getACKrcv(nodeId)) + (1 - alpha)*etx_prev;
        etx = round(etx*100)/100.0;  //2022-05-04
        etx_prev = etx;

        return etx;
        //In case that I want to calculate the etx as before I just have to make alpha = 1
        //return ((mac->getACKmissed(nodeId) + mac->getACKrcv(nodeId))/mac->getACKrcv(nodeId)) ;
        //double x = 4;
        //double y = 5;
        //return ((x+y)/y) ;
}

//Methods to normalize:
double ObjectiveFunction::Norm_den(int den){

    int Xmin = 5;
    int Xmax = 19;

    double Xnorm = (den - Xmin)/( Xmax - Xmin);

    Xnorm = round(Xnorm*100)/100.0;  //2022-05-04

    return Xnorm;

}

double ObjectiveFunction::Norm_maclosses(int mac_losses){

    int Xmin = 0;
    int Xmax = 2972;

    double Xnorm = (mac_losses - Xmin)/( Xmax - Xmin);

    Xnorm = round(Xnorm*100)/100.0;  //2022-05-04

    EV_INFO << "Normalized mac-losses: " << Xnorm << endl;

    return Xnorm;
}

double ObjectiveFunction::Norm_fps(double fps){

    double Xmin = 0;
    double Xmax = 1168;

    double Xnorm = (fps - Xmin)/( Xmax - Xmin);

    Xnorm = round(Xnorm*100)/100.0;  //2022-05-04

    EV_INFO << "Normalized fps: " << Xnorm << endl;

    return Xnorm;
}

double ObjectiveFunction::Norm_etx (double etx) {

    int Xmin = 1;
    int Xmax = 100;

    double Xnorm = (etx - Xmin)/( Xmax - Xmin);

    Xnorm = round(Xnorm*100)/100.0;  //2022-05-04

    EV_INFO << "Normalized etx: " << Xnorm << endl;

    return Xnorm;
}

double ObjectiveFunction::ML_rank_calculator_SP(const Ptr<const Dio>& dio) {

    if(simTime() < 1500)
        return 1;

    EV_INFO << "Before connecting to the server, check if the same set of metrics was already predicted" << endl;

    double etx_onlink = ETX_Calculator_onLink(dio);

    //sleep (10);

    //To get variables from Rpl.........2022-05-03
//    rplModule = getParentModule();
//  rpl = check_and_cast<Rpl *>(rplModule);

    macModule = getParentModule()->getParentModule()->getSubmodule("wlan",0)->getSubmodule("mac");
    auto mac = check_and_cast<Ieee802154Mac *>(macModule);

    //To get SNR AVE
    double snr_ave = mac->getSNRave(dio->getNodeId());

    //to get my current frame fail rate at the mac layer
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
    //

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

    if (prev_metric1 == dio->getBw() and prev_metric2 == dio->getDen() and prev_metric3 == etx_onlink and prev_metric4 == dio->getFps() and prev_metric5 == dio->getDropR() + dio->getDropB() ){
        EV_INFO << "Returning previous prediction" << endl;
        printf("\nReturning previous prediction");
        return (1 - prev_response);
    }else{

    EV_INFO << "connecting from ML rank calculator" << endl;

    /*Create a TCP socket*/

    #pragma comment(lib,"ws2_32.lib") //Winsock Library

    //int main(int argc , char *argv[])
    //{
    WSADATA wsa;
    SOCKET s;
    struct sockaddr_in server;
    const char *message ;
    char *space = " " ;
    char server_reply [4096];
    int recv_size;

    printf("\nInitialising Winsock...");

    if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
    {
           //EV_INFO << "Failed. Error Code : %d", WSAGetLastError() << endl;
           //return 1;
    }

    EV_INFO << "Initialised" << endl ;

    //Create a socket
    if((s = socket(AF_INET , SOCK_STREAM , 0 )) == INVALID_SOCKET)  //SOCK_DGRAM, SOCK_STREAM
    {
      //EV_INFO << "Could not create socket : %d" , WSAGetLastError() << endl;

        EV_INFO << "Could not create socket" << endl;

      std::ofstream file;
      file.open("St_Couldnt_create_socket.txt", std::ios_base::app);
      file << getParentModule()->getParentModule()->getFullName() << " " << simTime() << endl;
      file.close();
    }

            EV_INFO << "Socket created" << endl ;


            server.sin_addr.s_addr = inet_addr("127.0.0.1");
            server.sin_family = AF_INET;
            server.sin_port = htons( 5005 );

            //Connect to remote server
     label1: if (connect(s , (struct sockaddr *)&server , sizeof(server)) < 0)
            {
                puts("connect error");

                std::ofstream file;
                file.open("St_Connect_Error.txt", std::ios_base::app);
                file << getParentModule()->getParentModule()->getFullName() << " " << simTime() << endl;
                file.close();

                sleep (1);
                goto label1;
               // return 1;
            }

            puts("Connected");
            EV_INFO <<"conected"<< endl;
            //return 0;

            //Send some data
            //message = sprintf(2.53) ;//"GET / HTTP/1.1\r\n\r\n";
            //Get from the DIO the metric values that I am going to use to predict
            //double etx_onlink = ETX_Calculator_onLink(dio);

            //Custom precision
            std::ostringstream streamObj_m1;
            std::ostringstream streamObj_m2;
            std::ostringstream streamObj_m3;
            std::ostringstream streamObj_m4;
            std::ostringstream streamObj_m5;
            //std::ostringstream streamObj_m6;
            //std::ostringstream streamObj_m7;
            //std::ostringstream streamObj_m8;

            streamObj_m1 << std::fixed;
            streamObj_m1 << std::setprecision(2);
            streamObj_m1 << dio->getBw();

            streamObj_m2 << std::fixed;
            streamObj_m2 << std::setprecision(2);
            streamObj_m2 << dio->getDen();  //dio->getTxF(); //dio->getETX() + etx_onlink ;         //dio->getDen();  //get the value from DIO

            //streamObj_m3 << std::fixed;
            //streamObj_m3 << std::setprecision(2);
            //streamObj_m3 << dio->getFps();//dio->getRxF();      //dio->getDen();                       //get the value from DIO

            streamObj_m3 << std::fixed;
            streamObj_m3 << std::setprecision(2);
            streamObj_m3 << etx_onlink; //snr_ave ; //fail_retry; //dio->getSnr();  //rpl->getneighbors();  // how many neighbors the node that received the dio has.

            streamObj_m4 << std::fixed;
            streamObj_m4 << std::setprecision(2);
            streamObj_m4 << dio->getFps(); //rx_frame_rate ; //fail_cong ; //dio->getRx_un_suc();  // etx_onlink;  //calculate the etx of the link  //dio->getFps();  //get the value from DIO

            streamObj_m5 << std::fixed;
            streamObj_m5 << std::setprecision(2);
            streamObj_m5 << dio->getDropR() + dio->getDropB(); //fail_retry + fail_cong; //dio->getRx_suc_rate(); // dio->getSnr();  //get the value from DIO

/*            streamObj_m7 << std::fixed;
            streamObj_m7 << std::setprecision(2);
            streamObj_m7 << dio->getSnr();  //mac->nbTxFrames_copy; // + mac-> dio->getDropR() + dio->getDropB();  //get the value from DIO

            streamObj_m8 << std::fixed;
            streamObj_m8 << std::setprecision(2);
            streamObj_m8 << dio->getRx_suc_rate();  //dio->getDropR() + dio->getDropB();  //mac->framedropbycongestion + mac->framedropbyretry_limit_reached;  //get the value from the MAC layer
*/
            std::string strObj = streamObj_m1.str() + " " + streamObj_m2.str() +  " " + streamObj_m3.str() + " " + streamObj_m4.str() + " " + streamObj_m5.str() ;
            message = strObj.c_str();

            sendto(s , message , strlen(message) , 0 , 0, 0 );


         //Receive a reply from the server
                    if((recv_size = recv(s , server_reply , 4096 , 0)) == SOCKET_ERROR)
                    {
                        //puts("recv failed");
                        EV_INFO << "recv failed" << endl ;
                        std::ofstream file;
                        file.open("St_Recv_Failed.txt", std::ios_base::app);
                        file << getParentModule()->getParentModule()->getFullName() << " " << simTime() << endl;
                        file.close();
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
                    file << getParentModule()->getParentModule()->getFullName() << " received: " << num_double << endl;
                    file.close();


                //recv_size = recv(s , server_reply , 2000 , 0)

                closesocket(s);
//                WSACleanup();

                //The idea is to save the inputs and output to not open the socket connection to
                //calculate the prediction with the same set of metrics

                prev_metric1 = dio->getBw(); //my_ch_util ;
                prev_metric2 = dio->getDen(); //etx_onlink;        //dio->getTxF() ; //dio->getETX() + etx_onlink ;
                //prev_metric3 = dio->getFps();   //dio->getRxF() ; //etx_onlink; //dio->getDen();
                prev_metric3 = etx_onlink;        //snr_ave; //etx_onlink; //fail_retry; //dio->getSnr(); //rpl->getneighbors();
                prev_metric4 = dio->getFps();     //rx_frame_rate ; //fail_cong; //dio->getRx_un_suc(); // etx_onlink;
                prev_metric5 = dio->getDropR() + dio->getDropB();   //fail_retry + fail_cong ; //etx_onlink;
                //prev_metric7 = dio->getSnr(); //mac->nbTxFrames_copy;
                //prev_metric8 = dio->getRx_suc_rate(); //dio->getDropR() + dio->getDropB(); // mac->framedropbycongestion + mac->framedropbyretry_limit_reached;

                prev_response = num_double;

                return (1 - num_double); //Because I am working with probability of getting '1'
                //return (num_double);
    }
}

double ObjectiveFunction::ML_rank_calculator_FORcalcRank_SP (Dio* preferredParent) {

    if(simTime() < 1500)
        return 1;

    EV_INFO << "Before connecting to the server, check if the same set of metrics was already predicted" << endl;

    double etx_onlink = ETX_Calculator_onLink_calcRank(preferredParent);

    //To get variables from Rpl.........2022-05-03
//        rplModule = getParentModule();
//        rpl = check_and_cast<Rpl *>(rplModule);

        macModule = getParentModule()->getParentModule()->getSubmodule("wlan",0)->getSubmodule("mac");
        auto mac = check_and_cast<Ieee802154Mac *>(macModule);

        //To get SNR AVE
        double snr_ave = mac->getSNRave(preferredParent->getNodeId());

        //to get my current frame fail rate at the mac layer
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
            //
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

        if (prev_metric1 ==  preferredParent->getBw() and prev_metric2 == preferredParent->getDen() and prev_metric3 == etx_onlink and prev_metric4 == preferredParent->getFps()  and prev_metric5 == preferredParent->getDropR() + preferredParent->getDropB()){
            EV_INFO << "Returning previous prediction" << endl;
            printf("\nReturning previous prediction");
            return (1 - prev_response);
        }else{
         EV_INFO << "connecting from ML rank calculator" << endl;

        /*Create a TCP socket*/

        #pragma comment(lib,"ws2_32.lib") //Winsock Library

        //int main(int argc , char *argv[])
        //{
        WSADATA wsa;
        SOCKET s;
        struct sockaddr_in server;
        const char *message ;
        char *space = " " ;
        char server_reply [4096];
        int recv_size;

        printf("\nInitialising Winsock...");

        if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
        {
               //EV_INFO << "Failed. Error Code : %d", WSAGetLastError() << endl;
               //return 1;
        }

        EV_INFO << "Initialised" << endl ;

        //Create a socket
        if((s = socket(AF_INET , SOCK_STREAM , 0 )) == INVALID_SOCKET)  //for udp: SOCK_DGRAM, for TCP:SOCK_STREAM
        {
          //EV_INFO << "Could not create socket : %d" , WSAGetLastError() << endl;
          EV_INFO << "Could not create socket" << endl;

          std::ofstream file;
          file.open("St_Couldnt_create_socket.txt", std::ios_base::app);
          file << getParentModule()->getParentModule()->getFullName() << " " << simTime() << endl;
          file.close();
        }

                EV_INFO << "Socket created" << endl ;


                server.sin_addr.s_addr = inet_addr("127.0.0.1");
                server.sin_family = AF_INET;
                server.sin_port = htons( 5005 );

                //Connect to remote server
          label2:   if (connect(s , (struct sockaddr *)&server , sizeof(server)) < 0)
                {
                    puts("connect error");

                    std::ofstream file;
                    file.open("St_Connect_Error.txt", std::ios_base::app);
                    file << getParentModule()->getParentModule()->getFullName() << " " << simTime() << endl;
                    file.close();

                    sleep(1);
                    goto label2;
                   // return 1;
                }

                puts("Connected");
                EV_INFO <<"conected"<< endl;
                //return 0;

                //Send some data
                //message = sprintf(2.53) ;//"GET / HTTP/1.1\r\n\r\n";
                //Get from the DIO the metric values that I am going to use to predict
                //double etx_onlink = ETX_Calculator_onLink_calcRank(preferredParent);


                //Custom precision
                            std::ostringstream streamObj_m1;
                            std::ostringstream streamObj_m2;
                            std::ostringstream streamObj_m3;
                            std::ostringstream streamObj_m4;
                            std::ostringstream streamObj_m5;
                            //std::ostringstream streamObj_m6;
                            //std::ostringstream streamObj_m7;
                            //std::ostringstream streamObj_m8;

                            streamObj_m1 << std::fixed;
                            streamObj_m1 << std::setprecision(2);
                            streamObj_m1 << preferredParent->getBw() ;                    //dio->getBw();

                            streamObj_m2 << std::fixed;
                            streamObj_m2 << std::setprecision(2);
                            streamObj_m2 << preferredParent->getDen() ;         //dio->getDen();  //get the value from DIO

                            //streamObj_m3 << std::fixed;
                            //streamObj_m3 << std::setprecision(2);
                            //streamObj_m3 << preferredParent->getFps(); //preferredParent->getRxF();  //preferredParent->getDen();                       //get the value from DIO

                            streamObj_m3 << std::fixed;
                            streamObj_m3 << std::setprecision(2);
                            streamObj_m3 << etx_onlink; //fail_retry;  // how many neighbors the node that received the dio has.

                            streamObj_m4 << std::fixed;
                            streamObj_m4 << std::setprecision(2);
                            streamObj_m4 << preferredParent->getFps(); //preferredParent->getRx_un_suc();  //calculate the etx of the link  //dio->getFps();  //get the value from DIO

                            streamObj_m5 << std::fixed;
                            streamObj_m5 << std::setprecision(2);
                            streamObj_m5 << preferredParent->getDropR() + preferredParent->getDropB();//preferredParent->getTxF() + preferredParent->getRxF(); //etx_onlink; //get the value from DIO
/*
                            streamObj_m7 << std::fixed;
                            streamObj_m7 << std::setprecision(2);
                            streamObj_m7 << preferredParent->getSnr(); //mac->nbTxFrames_copy;  //get the value from DIO

                            streamObj_m8 << std::fixed;
                            streamObj_m8 << std::setprecision(2);
                            streamObj_m8 << preferredParent->getRx_suc_rate(); //preferredParent->getDropR() + preferredParent->getDropB(); //get the value from the MAC layer
*/
                            std::string strObj = streamObj_m1.str() + " " + streamObj_m2.str() +  " " + streamObj_m3.str() + " " +
                                    streamObj_m4.str() + " " + streamObj_m5.str() ;
                            message = strObj.c_str();

                sendto(s , message , strlen(message) , 0 , 0, 0 );


             //Receive a reply from the server
                        if((recv_size = recv(s , server_reply , 4096 , 0)) == SOCKET_ERROR)
                        {
                            //puts("recv failed");
                            EV_INFO << "recv failed" << endl ;

                            std::ofstream file;
                            file.open("St_Recv_Failed.txt", std::ios_base::app);
                            file << getParentModule()->getParentModule()->getFullName() << " " << simTime() << endl;
                            file.close();

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
                        file << getParentModule()->getParentModule()->getFullName() << " received: " << num_double << endl;
                        file.close();


                    //recv_size = recv(s , server_reply , 2000 , 0)

                    closesocket(s);
//                    WSACleanup();

                    //The idea is to save the inputs and output to not open the socket connection to
                    //calculate the prediction with the same set of metrics

                    prev_metric1 = preferredParent->getBw(); //my_ch_util ;
                    prev_metric2 = preferredParent->getDen();  //etx_onlink; //getETX() + etx_onlink ;
                    //prev_metric3 = preferredParent->getFps(); //etx_onlink; //preferredParent->getDen();
                    prev_metric3 = etx_onlink; //snr_ave; //fail_retry; //rpl->getneighbors();
                    prev_metric4 = preferredParent->getFps();  //rx_frame_rate; //fail_cong ; //preferredParent->getRx_un_suc();
                    prev_metric5 = preferredParent->getDropR() + preferredParent->getDropB(); //fail_cong + fail_retry; //preferredParent->getTxF() + preferredParent->getRxF() ; //etx_onlink ; //preferredParent->getRx_suc_rate();
                    //prev_metric7 = preferredParent->getSnr(); //preferredParent->getRx_un_suc(); //mac->nbTxFrames_copy;
                    //prev_metric8 =  preferredParent->getRx_suc_rate(); //preferredParent->getDropR() + preferredParent->getDropB();;

                   prev_response = num_double;

                   return ( 1 - num_double);
                    //return (num_double);
        }
}

Dio* ObjectiveFunction::GetBestCandidate(std::map<Ipv6Address, Dio *> candidateParents) {

    EV_INFO << "I am inside GetBestCandidate - case: RPL_ENH2" << endl;

    /* This is just to choose the best candidate among all the ones that I have in the
     * neighbor table.
     */

    /*
    if (candidateParents.empty()) {
        EV_WARN << "Couldn't determine preferred parent, provided set is empty" << endl;
        return nullptr;
    }
    */

    EV_DETAIL << "List of candidate parents: "<<endl;  //CL

    std::ofstream file;
    file.open("St_BestCandidateDecision.txt", std::ios_base::app);
    file << simTime() << ": " << getParentModule()->getParentModule()->getFullName() << endl;
    file << "List of candidate parents: " << endl;
    //file.close();

    for (auto cp : candidateParents){
        EV_DETAIL << cp.first << " - " << "Ranking: " <<cp.second->getRank() <<" - " << "path_cost: " <<cp.second->getPath_cost() << " - " << "last_update: " << cp.second->getLast_update() << endl;
        file << cp.first << " - " << "Ranking: " <<cp.second->getRank() <<" - " << "path_cost: " <<cp.second->getPath_cost() << " - " << "last_update: " << cp.second->getLast_update() << endl;
    }
    file << "--------------- " << endl ;
    file.close();

             Dio *newPrefParent = candidateParents.begin()->second;

             //EV_INFO << "I am inside GetBestCandidate - case: RPL_ENH2" << endl;

             double currentMinRank = newPrefParent->getRank() ;

             double current_path_cost_score;

             double candidate_path_cost_score;

             if (simTime() - newPrefParent->getLast_update() > 10000)
                 candidate_path_cost_score = newPrefParent->getHC();
             else
                 candidate_path_cost_score = newPrefParent->getPath_cost();

             current_path_cost_score = candidate_path_cost_score + Path_Cost_Calculator(newPrefParent);
             EV_INFO <<"Path cost through this candidate = " << current_path_cost_score << endl;

                          for (std::pair<Ipv6Address, Dio *> candidate : candidateParents) {
                              EV_INFO << " ...... here the for begins ...." << endl;
                              //uint16_t candidateParentRank = candidate.second->getRank();
                              //double candidateParentRank = candidate.second->getRank() + ETX_Calculator_onLink_calcRank(candidate.second) ;   ////Changed by CL 2022-02-02
                              double candidateParentRank = candidate.second->getRank();

                              //double candidate_path_cost_score;
                              //I can introduce here an 'if' to give a path_cost = 1 (max possible value) to the candidates with last_update > x
                              //Although maybe it is better to pass that feature to the path cost calculator
                              if (simTime() - candidate.second->getLast_update() > 10000)
                                  candidate_path_cost_score = candidate.second->getHC() + Path_Cost_Calculator(candidate.second);
                              else
                                  candidate_path_cost_score = candidate.second->getPath_cost() + Path_Cost_Calculator(candidate.second);

                              EV_INFO <<"candidate_path_cost_score = " << candidate_path_cost_score << endl;

                              if (candidateParentRank + thre < currentMinRank){
                                  currentMinRank = candidateParentRank;
                                  current_path_cost_score = candidate_path_cost_score;
                                  newPrefParent = candidate.second;
                              }
                              if (candidateParentRank == currentMinRank){
                                  //(currentMinRank - candidateParentRank <= thre & currentMinRank - candidateParentRank >= 0)
                                  //count how many times this happens
                                  std::ofstream file;
                                  file.open("St_tie-breaker-counter.txt", std::ios_base::app);
                                  file << simTime() << " " << getParentModule()->getParentModule()->getFullName()<< " "
                                       << "current_path_cost_score: " << current_path_cost_score << " | " << "candidate_path_cost_score: " <<candidate_path_cost_score <<  endl;
                                  if (candidate_path_cost_score + tie_thre < current_path_cost_score){
                                      current_path_cost_score = candidate_path_cost_score;
                                      currentMinRank = candidateParentRank;
                                      newPrefParent = candidate.second;
                                  }
                              }
                         }
                          EV_INFO << " ... End of the for ..." << endl ;

                          //double currentPrefParent_path_cost_score = currentPreferredParent->getPath_cost() + Path_Cost_Calculator(currentPreferredParent);
                          //if (currentMinRank == currentPreferredParent->getRank() & current_path_cost_score == currentPrefParent_path_cost_score ){
                          //    EV_INFO << "Preferred parent did not change b/c the new one has the same ranking and tie-breaker score"  << endl ;
                          //    return currentPreferredParent;
                          //}else{
                              return newPrefParent;
                          //}

}

double ObjectiveFunction::GetBestCandidatePATHCOST (std::map<Ipv6Address, Dio *> candidateParents) {

    EV_INFO << "I am inside GetBestCandidatePATHCOST - case: RPL_ENH2" << endl;

    /* This is just to choose the best candidate among all the ones that I have in the
     * neighbor table.
     */

    /*
    if (candidateParents.empty()) {
        EV_WARN << "Couldn't determine preferred parent, provided set is empty" << endl;
        return nullptr;
    }
    */

    EV_DETAIL << "List of candidate parents: "<<endl;  //CL
    for (auto cp : candidateParents){
        EV_DETAIL << cp.first << " - " << "Ranking: " <<cp.second->getRank() <<" - " << "path_cost: " <<cp.second->getPath_cost() << " - " << "last_update: " << cp.second->getLast_update() << endl;
    }

             Dio *newPrefParent = candidateParents.begin()->second;

             //EV_INFO << "I am inside GetBestCandidate - case: RPL_ENH2" << endl;

             double currentMinRank = newPrefParent->getRank() ;

             double current_path_cost_score_eTe;

             double candidate_path_cost_score;

             if (simTime() - newPrefParent->getLast_update() > 10000)
                 candidate_path_cost_score = newPrefParent->getHC()  ;
             else
                 candidate_path_cost_score = newPrefParent->getPath_cost();

             current_path_cost_score_eTe = candidate_path_cost_score + Path_Cost_Calculator(newPrefParent);
             EV_INFO <<"Path cost through this candidate = " << current_path_cost_score_eTe << endl;

                          for (std::pair<Ipv6Address, Dio *> candidate : candidateParents) {
                              EV_INFO << " ...... here the for begins ...." << endl;
                              //uint16_t candidateParentRank = candidate.second->getRank();
                              //double candidateParentRank = candidate.second->getRank() + ETX_Calculator_onLink_calcRank(candidate.second) ;   ////Changed by CL 2022-02-02
                              double candidateParentRank = candidate.second->getRank();

                              double candidate_path_cost_score_eTe;
                              //I can introduce here an 'if' to give a path_cost = 1 (max possible value) to the candidates with last_update > x
                              //Although maybe it is better to pass that feature to the path cost calculator
                              if (simTime() - candidate.second->getLast_update() > 10000)
                                  candidate_path_cost_score_eTe = candidate.second->getHC() + Path_Cost_Calculator(candidate.second);
                              else
                                  candidate_path_cost_score_eTe = candidate.second->getPath_cost() + Path_Cost_Calculator(candidate.second);

                              EV_INFO <<"candidate_path_cost_score = " << candidate_path_cost_score << endl;

                              if (candidateParentRank + thre < currentMinRank){
                                  currentMinRank = candidateParentRank;
                                  current_path_cost_score_eTe = candidate_path_cost_score_eTe;
                                  newPrefParent = candidate.second;
                              }
                              if (candidateParentRank == currentMinRank){
                                  //(currentMinRank - candidateParentRank <= thre & currentMinRank - candidateParentRank >= 0)
                                  //count how many times this happens
                                  //std::ofstream file;
                                  //file.open("St_tie-breaker-counter.txt", std::ios_base::app);
                                  //file << simTime() << " " << getParentModule()->getParentModule()->getFullName()<< " "
                                    //   << current_path_cost_score_eTe << " " << candidate_path_cost_score_eTe <<  endl;
                                  if (candidate_path_cost_score_eTe + tie_thre < current_path_cost_score_eTe){
                                      current_path_cost_score_eTe = candidate_path_cost_score_eTe;
                                      currentMinRank = candidateParentRank;
                                      newPrefParent = candidate.second;
                                  }
                              }
                         }
                          EV_INFO << " ... End of the for ..." << endl ;

                          return current_path_cost_score_eTe;

                          //double currentPrefParent_path_cost_score = currentPreferredParent->getPath_cost() + Path_Cost_Calculator(currentPreferredParent);
                          //if (currentMinRank == currentPreferredParent->getRank() & current_path_cost_score == currentPrefParent_path_cost_score ){
                          //    EV_INFO << "Preferred parent did not change b/c the new one has the same ranking and tie-breaker score"  << endl ;
                          //    return currentPreferredParent;
                          //}else{
                          //    return newPrefParent;
                          //}

}

double ObjectiveFunction::ML_rank_calculator(const Ptr<const Dio>& dio) {

    //Get from the DIO the metric values that I am going to use to predict
   float etx_onlink = ETX_Calculator_onLink(dio);

   float bw = dio->getBw();
   float den = dio->getDen();
   float fps = dio->getFps();
   float drop = dio->getDropR() + dio->getDropB();


   const std::vector<float> features = { bw, den, etx_onlink, fps, drop};

   double prediction = ApplyCatboostModel(features);
   //double prediction = 0;

   std::ofstream file;
   file.open("St_RankingCalcReplayfromML.txt", std::ios_base::app);
   file << getParentModule()->getParentModule()->getFullName() << " received: " << prediction << endl;
   file.close();

   return prediction;

}

double ObjectiveFunction::ML_rank_calculator_FORcalcRank (Dio* preferredParent) {

    //Get from the DIO the metric values that I am going to use to predict
    float etx_onlink = ETX_Calculator_onLink_calcRank(preferredParent);

    float bw = preferredParent->getBw();
    float den = preferredParent->getDen();
    float fps = preferredParent->getFps();
    float drop = preferredParent->getDropR() + preferredParent->getDropB();


    const std::vector<float> features = {bw, den, etx_onlink, fps, drop };

    double prediction = ApplyCatboostModel(features);

    //double prediction = 0;
    std::ofstream file;
    file.open("St_RankingCalcReplayfromML.txt", std::ios_base::app);
    file << getParentModule()->getParentModule()->getFullName() << " received: " << prediction << endl;
    file.close();

    return prediction;

}

void ObjectiveFunction::initialize()
{
    //Open a connection with the server
//    EV_INFO << "connecting from ML rank calculator" << endl;

            /*Create a TCP socket*/

/*            #pragma comment(lib,"ws2_32.lib") //Winsock Library

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
              EV_INFO << "Could not create socket" << endl;

              std::ofstream file;
              file.open("St_Couldnt_create_socket.txt", std::ios_base::app);
              file << getParentModule()->getParentModule()->getFullName() << " " << simTime() << endl;
              file.close();
            }

                    EV_INFO << "Socket created" << endl ;


                    server.sin_addr.s_addr = inet_addr("127.0.0.1");
                    server.sin_family = AF_INET;
                    server.sin_port = htons( 8080 );

                    //Connect to remote server
              label2:   if (connect(s , (struct sockaddr *)&server , sizeof(server)) < 0)
                    {
                        puts("connect error");

                        std::ofstream file;
                        file.open("St_Connect_Error.txt", std::ios_base::app);
                        file << getParentModule()->getParentModule()->getFullName() << " " << simTime() << endl;
                        file.close();

                        sleep(1);
                        goto label2;
                       // return 1;
                    }

                    puts("Connected");
                    EV_INFO <<"conected"<< endl;
                    //return 0;
*/
}

void ObjectiveFunction::finish()
{
    //Close the connection with the server
//    closesocket(s);
    WSACleanup();
}

} // namespace inet

/* (Old implementation of the ETX using DIOs msg)
 * Part of the ETX implementation, I'll try first using DIO as in the old implementation
double ObjectiveFunction::ETX_Calculator_onLink(const Ptr<const Dio>& dio){

    std::vector<ParentStructure*>::iterator it;
    for(it = countercache.begin(); it != countercache.end(); it++)
    {
        if(dio->getSrcAddress() == (*it)->SenderIpAddr){
            (*it)->Rcvdcounter = (*it)->Rcvdcounter + 1;
            EV_INFO <<"Number of DIOs received from this sender: " << (*it)->Rcvdcounter <<endl;
            EV_INFO <<"Number of DIOs sent by this sender = "<< dio->getNumDIO()  <<endl;
            return (dio->getNumDIO()/(*it)->Rcvdcounter);
        }
    }
    if (it == countercache.end()) {
        ParentStructure* rcv = new ParentStructure();
        rcv->SenderIpAddr = dio->getSrcAddress();
        rcv->Rcvdcounter = 1;
        countercache.push_back(rcv);
        EV_INFO <<"first DIO received from this sender " <<endl;
        return (dio->getNumDIO()/rcv->Rcvdcounter);
    }
}

double ObjectiveFunction::ETX_Calculator_onLink_calcRank(Dio* preferredParent){
    std::vector<ParentStructure*>::iterator it;
        for(it = countercache.begin(); it != countercache.end(); it++)
        {
            if(preferredParent->getSrcAddress() == (*it)->SenderIpAddr){
                //(*it)->Rcvdcounter = (*it)->Rcvdcounter + 1;   //no necessary
                EV_INFO << "I am inside ETX_Calculator_onLink_calcRank" << endl;
                EV_INFO <<"Number of DIOs received from this sender: " << (*it)->Rcvdcounter <<endl;
                EV_INFO <<"Number of DIOs sent by this sender = "<< preferredParent->getNumDIO()  <<endl;
                return (preferredParent->getNumDIO()/(*it)->Rcvdcounter);
            }
        }
//        if (it == countercache.end()) {
//            ParentStructure* rcv = new ParentStructure();
//            rcv->SenderIpAddr = dio->getSrcAddress();
//            rcv->Rcvdcounter = 1;
//            countercache.push_back(rcv);
//            EV_INFO <<"first DIO received from this sender " <<endl;
//            return (dio->getNumDIO()/rcv->Rcvdcounter);
//        }
 *
 */


