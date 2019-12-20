#include <omnetpp.h>
#include <msg1_m.h>


using namespace omnetpp;


class Source : public cSimpleModule
{
  private:
    Msg1 *sendMessageEvent;
    int nbGenMessages;
    short prio;                         //since prio is generated at the end of execution of handlemessage to schedule next msg, we have to remember which kind is the packet to send in the next execution of handlemessage

  public:
    Source();
    virtual ~Source();

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
};

Define_Module(Source);

Source::Source()
{
    sendMessageEvent = nullptr;
}

Source::~Source()
{
    cancelAndDelete(sendMessageEvent);
}

void Source::initialize()
{
    sendMessageEvent = new Msg1("sendMessageEvent");
    scheduleAt(simTime()+par("interArrivalTime").doubleValue(), sendMessageEvent);
    nbGenMessages = 0;
    prio = short(par("SourceOfClass"));               //To save in the class the value of priority of the class that this source generates
}

void Source::handleMessage(cMessage *msg)
{

    ASSERT(msg == sendMessageEvent);
    Msg1* SelfMessage=check_and_cast<Msg1*>(msg);

    char msgname[20];
    sprintf(msgname, "message-%d.%d",(int)par("SourceOfClass"), ++nbGenMessages);
    Msg1 *message = new Msg1(msgname,prio);
    message->setResidual_time(0);
    message->setFirstServerEntrance(0);
    message->setServiceTime(par("serviceTime").doubleValue());  //Define a service time for the msg
    send(message, "out");
    EV<<"Sending message "<<message->getName()<<" at time "<<simTime()<<" with priority "<<prio<<endl;
    scheduleAt(simTime()+par("interArrivalTime").doubleValue(), sendMessageEvent);
}
