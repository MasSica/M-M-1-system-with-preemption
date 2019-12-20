#include <omnetpp.h>
#include <msg1_m.h>

using namespace omnetpp;


class Sink : public cSimpleModule
{
  private:
    simsignal_t lifetimeSignal;
    simsignal_t arrivedMsgSignal;
    int nb_arrivedMsg;

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
};

Define_Module(Sink);


void Sink::initialize()
{
    lifetimeSignal = registerSignal("lifetime");
    arrivedMsgSignal = registerSignal("arrivedMsg");
    nb_arrivedMsg = 0;
}

void Sink::handleMessage(cMessage *msg)
{
    Msg1* ServedMsg=check_and_cast<Msg1*>(msg);
    simtime_t lifetime = simTime() - ServedMsg->getCreationTime();
    EV << "Received " << ServedMsg->getName() << ", lifetime: " << lifetime << "s" << endl;
    emit(lifetimeSignal, lifetime);

    nb_arrivedMsg ++;
    emit(arrivedMsgSignal, nb_arrivedMsg);
    delete ServedMsg;
}
