#include "RplRouter.h"

//CL
#include <sqlite3.h>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include "Sink.h"

Define_Module(Sink);

void Sink::initialize()
{
/*    //CL, All this code was added to write in the PingApp the value of the variable: destAddr
    // I am using this app to simulate demand reading traffic, from collector to meters.

    // Pointer to SQLite connection
       sqlite3* db;

   // Save the result of opening the file
       int rc;

       // Save the result of opening the file
       rc = sqlite3_open("HandleOmnetApplications.sqlite", &db);
       sqlite3_stmt* stmt = 0;

       //get the number of meter then  numMeters;
       //int numMeters = 2;
       cModule *parent = nullptr;
       parent = this->getParentModule();
       int numMeters= parent->par("numMeters").intValue();
       int dss;
       char destMeter[60];

       std::string str ;
       //str.append(1,'"');

       for (int i=0; i<numMeters; i++){
           std::string String = static_cast<std::ostringstream*>( &(std::ostringstream() << i) )->str();
           std::string sqlSelectSpe = "SELECT * FROM demand_read WHERE meter_id = " + String + ";";  //change the name of the table
           sqlite3_prepare_v2(db, sqlSelectSpe.c_str(), -1, &stmt, NULL);
           sqlite3_step(stmt);
           dss = sqlite3_column_int(stmt, 1);
           if (dss == 1){
               sprintf(destMeter,"meter[%d] ",i);
               str.append(destMeter);
           }
       }

       sqlite3_finalize(stmt);

       // Close the SQL connection
       sqlite3_close(db);

       //str.append(1,'"');
       EV_INFO << "destAddr = " << str << endl; //I did it just to see what was going on

       const char * c = str.c_str(); //I had to convert form string to char

      setdestAddr(c);  */

    //added by CL 2022-01-18
         std::ofstream file;
         file.open("St_packet-tracer.txt", std::ios_base::app);

         int numMeters = getParentModule()->par("numMeters");
         EV_INFO << "nummeters: " <<numMeters << endl;
              //double simtime = getParentModule()->getParentModule()->par("sim-time-limit");
              //EV_INFO << "simtime: " <<simtime << endl;
              //int seed = getParentModule()->par("seed-set");
              //EV_INFO << "seed-set: " <<seed << endl;

         std::string routing = getParentModule()->par("Routing");
         file << "NumMeters: "<< numMeters << " - Tech: " << routing << " - seed: " << rand() << endl;
}

void Sink::handleMessage(cMessage *msg)
{
    // TODO - Generated method body
}

//CL
/*void Collector::setdestAddr(const char *destMeter){
    cModule *appModule = nullptr;
    appModule = this->getSubmodule("app",0);
    appModule->par("destAddr").setStringValue(destMeter);
} */
