/*
 * NetworkManagment.h
 *
 *  Created on: Jul. 28, 2021
 *      Author: carlo
 */

#ifndef NETWORKMANAGMENT_H_
#define NETWORKMANAGMENT_H_

#include <omnetpp.h>

using namespace omnetpp;

/**
 * TODO - Generated class
 */
class NetworkManagment : public cSimpleModule
{
  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void finish();

  public:
    std::map<int,int> metersXY, routerXY, collectorsXY, wmetersXY, cameraXY, switchXY;  //to store the XY coordinates of modules
    std::vector<std::string> gisCSVvector;
};




#endif /* NETWORKMANAGMENT_H_ */
