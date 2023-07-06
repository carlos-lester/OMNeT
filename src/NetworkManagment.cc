/*
 * NetworkManagment.cc
 *
 *  Created on: Jul. 28, 2021
 *      Author: carlo
 */

#include "NetworkManagment.h"
#include "RplRouter.h"
#include "Meter.h"
#include "Sink.h"
//#include "Collector.h"
//##include "WMeter.h"
//#include "Camera.h"
//#include "Switch.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

Define_Module(NetworkManagment);

void NetworkManagment::initialize()
{
    // TODO - Generated method body

    cModule *oMeter= nullptr;
    cModule *oCollector= nullptr;
    cModule *oRepeater= nullptr;

 //   cModule *oWMeter = nullptr; //add for water meters
 //   cModule *oCamera = nullptr;
 //   cModule *oSwitch = nullptr;
    char str [60];
    cModule *parent = nullptr;  //add by CL
//  cModule parent = this->getParentModule();
    parent = this->getParentModule();


    //Set position of devices
   // int numNodes= parent->par("numNodes").intValue();  //this was for the original version the RPL
    int numMeters= parent->par("numMeters").intValue();
    int numRepeaters = parent->par("numRepeaters").intValue();
    int numCollectors = parent->par("numSinks").intValue();
//    int numWMeters= parent->par("numWMeters").intValue();
//    int numCameras= parent->par("numCameras").intValue();
//    int numSwitches= parent->par("numSwitches").intValue();

    const double  rEarthB = 6356752.3 ;
    const double pi = 3.14159265358979323846;
    double latRef = 45.70501621;  // BLPC 13.335154 ;   //45.70501621;  These values seem to be for Montreal
    double lonRef = -73.97729327; // BLPC -59.650129 ;                    //-73.97729327;

    //double a_eqRad = 6378137;
    //double b_poRad = 6356752.3;
    //double rEarthL, a1, a2,  b1, b2, lat, lon ;

    int posX,posY;

   int strBegin, strFstComma, strSecComma;
    std::string rowString, xString, yString;
    const char *xChar = nullptr;  //CL
    const char *yChar = nullptr;  //CL
    xChar = xString.c_str();
    yChar = yString.c_str();
    //const char *xChar = xString.c_str();
    //const char *yChar = yString.c_str();
    char first = 'a';
    char *end = &first; // CL added null

// coordinates reading ......METERS
    //std::ifstream gisFile("metersCSVomnetpp.csv",std::ios_base::in);
    std::ifstream gisFile("meters_location.csv",std::ios_base::in);
    if(!gisFile.is_open() || !parent->par("GIS")){
        //file not ready to open, nothing to update
        EV_INFO <<"Sorry, GIS not enabled OR metersCSVomnetpp.csv file does not exist!! \n";
    }
    else{
        EV_INFO <<"File metersCSVomnetpp.csv with coordinates data exists!! \n";
        for(int i=0; i<(numMeters+1) ;i++){
            getline(gisFile,rowString);
//            // check end of file
            gisCSVvector.push_back(rowString);
        }
        gisFile.close();
        gisFile.clear();

        for(int m=0;m<numMeters;m++){
            rowString=gisCSVvector[m+1];
            strBegin = rowString.find_first_not_of("",0);
            strFstComma = rowString.find_first_of(",", 0);
            strSecComma = rowString.find_first_of(",", strFstComma+1);
            yString = rowString.substr(strFstComma+1,strSecComma-strFstComma-1);
            xString = rowString.substr(strSecComma+1,rowString.length()-strSecComma-1);
//            //Coordinates computation
            posX = 2*pi*rEarthB*(std::strtod(xChar,&end)-lonRef)/360 + 1000;
            posY = 2*pi*rEarthB*(latRef-std::strtod(yChar,&end))/360 + 1000;
            sprintf(str,"meter[%d]",m);
            oMeter=getModuleByPath(str);
            oMeter->par("meterID").setIntValue(m);
            oMeter->getDisplayString().setTagArg("p",0, posX);
            oMeter->getDisplayString().setTagArg("p",1, posY);
            oMeter->par("x").setDoubleValue(posX);
           oMeter->par("y").setDoubleValue(posY);
           ((RplRouter *)oMeter)->setXpos(posX);
            ((RplRouter *)oMeter)->setYpos(posY);
//            ((Meter *)oMeter)->findOptCollector();


        }
        gisCSVvector.clear();
    }
//end of coordinates reading for Meters

// coordinates reading ......COLLECTORS

  //gisFile.open("collectorsCSVomnetpp.csv",std::ios_base::in);
  gisFile.open("collectors_location.csv",std::ios_base::in);
    //if(!gisFile.is_open() || !getAncestorPar("GIS")){
      if(!gisFile.is_open() || !parent->par("GIS")){
        //file not ready to open, nothing to update
       EV_INFO <<"Sorry, GIS not enabled OR collectorsCSVomnetpp.csv file does not exist!! \n";
    }
    else{// file to be read exists
        EV_INFO <<"File collectorsCSVomnetpp.csv with coordinates data exists!!";
        for(int j=0; j<(numCollectors+1) ;j++){
           getline(gisFile,rowString);
            // check end of file
            gisCSVvector.push_back(rowString);
        }
        gisFile.close();

        for(int c=0; c<numCollectors ;c++){
           rowString=gisCSVvector[c+1];
           strBegin = rowString.find_first_not_of("",0);
           strFstComma = rowString.find_first_of(",", 0);
            strSecComma = rowString.find_first_of(",", strFstComma+1);
            yString = rowString.substr(strFstComma+1,strSecComma-strFstComma-1);
            xString = rowString.substr(strSecComma+1,rowString.length()-strSecComma-1);
            posX = 2*pi*rEarthB*(std::strtod(xChar,&end)-lonRef)/360 + 1000;
            posY = 2*pi*rEarthB*(latRef-std::strtod(yChar,&end))/360 + 1000;
            sprintf(str,"sink[%d]",c);
            //sprintf(str,"sink");
            oCollector = getModuleByPath(str);
            oCollector->getDisplayString().setTagArg("p",0, posX);
            oCollector->getDisplayString().setTagArg("p",1, posY);
            ((RplRouter *)oCollector)->setXpos(posX);
            ((RplRouter *)oCollector)->setYpos(posY);
            oCollector->par("x").setDoubleValue(posX);
            oCollector->par("y").setDoubleValue(posY);
        }
        gisCSVvector.clear();
    }
// end of coordinates reading ......COLLECTORS

// coordinates reading ......Routers

   gisFile.open("routersCSVomnetpp.csv",std::ios_base::in);
       if(!gisFile.is_open() || !parent->par("GIS")){
           //file not ready to open, nothing to update
           EV_INFO <<"Sorry, GIS not enabled OR routersCSVomnetpp.csv file does not exist!! \n";
       }
       else{
           EV_INFO <<"File routersCSVomnetpp.csv with coordinates data exists!! \n";
           for(int i=0; i<(numRepeaters+1) ;i++){
               getline(gisFile,rowString);
               // check end of file
               gisCSVvector.push_back(rowString);
           }
           gisFile.close();

          for(int r=0; r<numRepeaters ;r++){
              rowString=gisCSVvector[r+1];
              strBegin = rowString.find_first_not_of("",0);
              strFstComma = rowString.find_first_of(",", 0);
              strSecComma = rowString.find_first_of(",", strFstComma+1);
              yString = rowString.substr(strFstComma+1,strSecComma-strFstComma-1);
              xString = rowString.substr(strSecComma+1,rowString.length()-strSecComma-1);
             //coordinates computation
             posX = 2*pi*rEarthB*(std::strtod(xChar,&end)-lonRef)/360;
             posY = 2*pi*rEarthB*(latRef-std::strtod(yChar,&end))/360;
             sprintf(str,"router[%d]",r);
             oRepeater = getModuleByPath(str);
             oRepeater->getDisplayString().setTagArg("p",0, posX);
             oRepeater->getDisplayString().setTagArg("p",1, posY);
             oRepeater->par("x").setDoubleValue(posX);
             oRepeater->par("y").setDoubleValue(posY);
             ((RplRouter *)oRepeater)->setXpos(posX);
             ((RplRouter *)oRepeater)->setYpos(posY);

           }
           gisCSVvector.clear();
       }

// end of coordinates reading ......Routers

    //This part of the code works when the files in .csv are not available or not used
          for(int i=0; i<numMeters ;i++){
          sprintf(str,"meter[%d]",i);
          oMeter=getModuleByPath(str);
          EV_INFO << "oMeter1: " << oMeter << endl;
          posX = ((RplRouter *)oMeter)->getXPos();
          posY = ((RplRouter *)oMeter)->getYPos();
          oMeter->getDisplayString().setTagArg("p",0, posX);
          oMeter->getDisplayString().setTagArg("p",1, posY);
          oMeter->par("x").setDoubleValue(posX);
          oMeter->par("y").setDoubleValue(posY);
          ((RplRouter *)oMeter)->setXpos(posX);
          ((RplRouter *)oMeter)->setYpos(posY);
//          ((Meter *)oMeter)->findOptCollector();
          }


          for(int j=0; j<numRepeaters ;j++){
          sprintf(str,"repeater[%d]",j);
          oRepeater=getModuleByPath(str);
          EV_INFO << "oRepeater1: " << oRepeater << endl;
          posX = ((RplRouter *)oRepeater)->getXPos();
          posY = ((RplRouter *)oRepeater)->getYPos();
          oRepeater->getDisplayString().setTagArg("p",0, posX);
          oRepeater->getDisplayString().setTagArg("p",1, posY);
          oRepeater->par("x").setDoubleValue(posX);
          oRepeater->par("y").setDoubleValue(posY);
          ((RplRouter *)oRepeater)->setXpos(posX);
          ((RplRouter *)oRepeater)->setYpos(posY);
          }

          for(int j=0; j<numCollectors ;j++){
          sprintf(str,"sink[%d]",j);
          oCollector=getModuleByPath(str);
          posX = ((RplRouter *)oCollector)->getXPos();
          posY = ((RplRouter *)oCollector)->getYPos();
          oCollector->getDisplayString().setTagArg("p",0, posX);
          oCollector->getDisplayString().setTagArg("p",1, posY);
          oCollector->par("x").setDoubleValue(posX);
          oCollector->par("y").setDoubleValue(posY);
         ((RplRouter *)oCollector)->setXpos(posX);
         ((RplRouter *)oCollector)->setYpos(posY);
         }


}

void NetworkManagment::handleMessage(cMessage *msg)
{
    // TODO - Generated method body
}

void NetworkManagment::finish()
{
  //  EV_INFO <<"Sorry \n";
}




