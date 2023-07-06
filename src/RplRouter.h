/*
 * RplRouter.h
 *
 *  Created on: Jul. 28, 2021
 *      Author: carlos
 */

#ifndef RPLROUTER_H_
#define RPLROUTER_H_


#include <omnetpp.h>

using namespace omnetpp;

/**
 * TODO - Generated class
 */
class RplRouter : public cSimpleModule
{
  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);

  public:
    // position on the canvas, unit is m
    double x, y;

    //getters
    double getXPos() {return x = this->par("x");};
    double getYPos() {return y = this->par("y");};


    //setters
    void setXpos(double xpos){x=xpos;};
    void setYpos(double ypos){y=ypos;};

};


#endif /* RPLROUTER_H_ */
