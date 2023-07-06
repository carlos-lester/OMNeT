#ifndef SINK_H_
#define SINK_H_

#include <omnetpp.h>
#include "RplRouter.h"

using namespace omnetpp;

/**
 * TODO - Generated class
 */
class Sink : public RplRouter               //cSimpleModule
{
  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);

  //CL
  public:
    //set destination address
 //   void setdestAddr(const char *destMeter);
};



#endif /* SINK_H_ */
