/*
 * Meter.h
 *
 *  Created on: Jul 29, 2021
 *      Author: cduenas
 */

#ifndef METER_H_
#define METER_H_

#include <omnetpp.h>
#include "RplRouter.h"
#include <iostream>
#include <string>

using namespace omnetpp;

/**
 * TODO - Generated class
 */
class Meter : public RplRouter
{
  protected:
   virtual void initialize() override;
   virtual void handleMessage(cMessage *msg) override;

  public:
    // position on the canvas, unit is m
    double x, y;

    //getters
    double getXPos() {return x = this->par("x");};
    double getYPos() {return y = this->par("y");};

    //set destination address
//  void setDestAddress(const char *destColl);
//  void findOptCollector();
//  void setMeterIdApp(int meterId);
};




#endif /* METER_H_ */
