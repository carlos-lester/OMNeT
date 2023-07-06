/*
 * RplRouter.cc
 *
 *  Created on: Jul. 28, 2021
 *      Author: carlo
 */

#include "RplRouter.h"

Define_Module(RplRouter);

void RplRouter::initialize()
{
    // TODO - Generated method body

    x = par("x").doubleValue();
    y = par("y").doubleValue();


    cModule *parentC = getParentModule();
    parentC->getDisplayString().setTagArg("p", 0, x);
    parentC->getDisplayString().setTagArg("p", 1, y);

    if(strcmp(this->getParentModule()->getName(),"RplNetwork") == 0){
        cModule *parentC = this;
        parentC->getDisplayString().setTagArg("p", 0, x);
        parentC->getDisplayString().setTagArg("p", 1, y);
    }
    else
    {
        cModule *parentC = getParentModule();
        parentC->getDisplayString().setTagArg("p", 0, x);
        parentC->getDisplayString().setTagArg("p", 1, y);
    }
}

void RplRouter::handleMessage(cMessage *msg)
{
    // TODO - Generated method body
}


