/*
 * Meter.cc
 *
 *  Created on: Jul 29, 2021
 *      Author: cduenas
 */

#include "Meter.h"
#include "RplRouter.h"

#include <iostream>
#include <string>
#include <fstream>

Define_Module(Meter);

void Meter::initialize()
{
    // TODO - Generated method body

    //Open a connection with the server


}
//
void Meter::handleMessage(cMessage *msg)
{
    // TODO - Generated method body
}
/*
void Meter::findOptCollector(){

    char strC[60] , destColl[60];
    cModule *collectorMod = nullptr;
    //cModule *collectorMod ;
    double posX = Meter::getXPos();
    double posY = Meter::getYPos();
    double posXColl,posYColl,dist;
    double minDist = 1000000000000000;
    int optimColl= 0;
    int numCollectors = this->getAncestorPar("numCollectors").intValue();

    for(int j=0; j<numCollectors; j++){
       sprintf(strC,"collector[%d]",j);
       collectorMod=getModuleByPath(strC);
       posXColl = ((Node *)collectorMod)->getXPos();
       posYColl = ((Node *)collectorMod)->getYPos();
       dist = sqrt((posX - posXColl)*(posX - posXColl) + (posY - posYColl)*(posY - posYColl));
       EV_INFO << "The distance of meter to collector " << j << " is : " << dist << std::endl;
       if (minDist > dist) {
           minDist = dist;
           optimColl = j;
        }
    }
    //to print distance to the closest collector
    //std:: ofstream file ;
    //file.open("DistancetoClosestCollector_dist.txt", std::ios_base::app);
    //file <<  dist << endl;   //I do not why with "dist" does not work properly
    //file.close();

    sprintf(destColl,"collector[%d]",optimColl);

    //to print closest collector
    //std:: ofstream file ;
    //file.open("ClosestCollector.txt", std::ios_base::app);
    //file <<  destColl << endl;
    //file.close();

    //to print distance to the closest collector
    //std:: ofstream file ;
    //file.open("DistancetoClosestCollector_minDist.txt", std::ios_base::app);
    //file <<  minDist << endl;
   // file.close();

    Meter::setDestAddress(destColl);



}

void Meter::setDestAddress(const char *destColl){
    cModule *appModule = nullptr;
    appModule = this->getSubmodule("app",0);
    appModule->par("destAddresses").setStringValue(destColl);

}

void Meter::setMeterIdApp(int meterId){
    cModule *appModule1 = nullptr;
    appModule1 = this->getSubmodule("app",0);
    appModule1->par("meterId").setIntValue(meterId);
}
*/

