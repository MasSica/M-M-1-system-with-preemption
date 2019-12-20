#include <omnetpp.h>
#include <msg1_m.h>

using namespace omnetpp;


class Queue : public cSimpleModule
{
  protected:
    Msg1 *msgServiced;
    Msg1 *endServiceMsg;
    Msg1 *msgAux;

    cQueue queue;

    simsignal_t qlenSignal;
    simsignal_t busySignal;
    simsignal_t queueingTimeSignal;
    simsignal_t responseTimeSignal;
    simsignal_t *ClassResponseTimeSignal;
    simsignal_t *ClassQueueingTimeSignal;
    simsignal_t *ClassExtServiceTimeSignal;

    int *msgQueueClassCount;


  public:
    Queue();
    virtual ~Queue();

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    void insertOrderedQueue(Msg1 *msg);
};

Define_Module(Queue);

Queue::Queue()
{
    msgServiced = endServiceMsg = msgAux = nullptr;
}

Queue::~Queue()
{
    delete msgServiced;
    cancelAndDelete(endServiceMsg);
}

void Queue::insertOrderedQueue(Msg1 *msg){                  //Declaration of a member function used to insert the message in the right position of the queue (Priority scheduling)
    int k = msg->getKind();                                 //Get priority of the message
    if(queue.getLength()==0){                               //If queue is empty
        queue.insert(msg);
        msgQueueClassCount[k]++;
        return;
    }
    int i;
    int beforeMsg=0;
    for(i=1; i<=k; i++){                                    //To count how many messages should be in front of the one we have to insert
        beforeMsg+=msgQueueClassCount[i];
    }
    if(beforeMsg==0){                                       //If the message I have must be insert in the first position
        queue.insertBefore(queue.front(), msg);
        msgQueueClassCount[k]++;
        return;
    }
    else{                                                   //If the message I have must be insert in others position
        queue.insertAfter(queue.get(beforeMsg-1), msg);
        msgQueueClassCount[k]++;
        return;
    }
}

void Queue::initialize()
{
    endServiceMsg = new Msg1("end-service");
    queue.setName("queue");

    qlenSignal = registerSignal("qlen");
    busySignal = registerSignal("busy");
    queueingTimeSignal = registerSignal("queueingTime");
    responseTimeSignal = registerSignal("responseTime");

    emit(qlenSignal, queue.getLength());
    emit(busySignal, false);

    //Register per-class signals' statistics
    char signalName[32];
    char statisticName[32];
    int i;
    simsignal_t *ClassRTStat = new simsignal_t[1 + int(getParentModule()->par("n"))];
    for(i=1; i<=int(getParentModule()->par("n")); i++){
        sprintf(signalName, "Class%d-ResponseTime", i);
        ClassRTStat[i]= registerSignal(signalName);
        sprintf(statisticName, "Class %d ResponseTime", i);
        cProperty *statisticTemplate = getProperties()->get("statisticTemplate", "ClassResponseTime");
        getEnvir()->addResultRecorders(this, ClassRTStat[i], statisticName, statisticTemplate);
    }
    ClassResponseTimeSignal=ClassRTStat;

    simsignal_t *ClassQTStat = new simsignal_t[1 + int(getParentModule()->par("n"))];
        for(i=1; i<=int(getParentModule()->par("n")); i++){
            sprintf(signalName, "Class%d-QueueingTime", i);
            ClassQTStat[i]= registerSignal(signalName);
            sprintf(statisticName, "Class %d QueueingTime", i);
            cProperty *statisticTemplate = getProperties()->get("statisticTemplate", "ClassQueueingTime");
            getEnvir()->addResultRecorders(this, ClassQTStat[i], statisticName, statisticTemplate);
        }
    ClassQueueingTimeSignal=ClassQTStat;

    simsignal_t *ClassESTStat = new simsignal_t[1 + int(getParentModule()->par("n"))];
        for(i=1; i<=int(getParentModule()->par("n")); i++){
            sprintf(signalName, "Class%d-ExtServiceTime", i);
            ClassESTStat[i]= registerSignal(signalName);
            sprintf(statisticName, "Class %d ExtServiceTime", i);
            cProperty *statisticTemplate = getProperties()->get("statisticTemplate", "ClassExtServiceTime");
            getEnvir()->addResultRecorders(this, ClassESTStat[i], statisticName, statisticTemplate);
        }
    ClassExtServiceTimeSignal=ClassESTStat;

    int *msgQCC = new int[1 + int(getParentModule()->par("n"))];        //To store the number of messages in queue per class
    for(i=1; i<=int(getParentModule()->par("n")); i++){
        msgQCC[i]=0;
    }
    msgQueueClassCount = msgQCC;
}

void Queue::handleMessage(cMessage *msg){

    Msg1* messaggio=check_and_cast<Msg1*>(msg);

    if (messaggio == endServiceMsg) { // Self-message arrived

        EV << "Completed service of " << msgServiced->getName() << endl;
        send(msgServiced, "out");
        //Response time: time from msg arrival timestamp to time msg ends service (now)
        emit(responseTimeSignal, simTime() - msgServiced->getTimestamp());
        emit(ClassResponseTimeSignal[msgServiced->getKind()], simTime() - msgServiced->getTimestamp());
        emit(ClassExtServiceTimeSignal[msgServiced->getKind()], simTime() - msgServiced->getFirstServerEntrance());

        if (queue.isEmpty()) { // Empty queue, server goes in IDLE
            EV << "Empty queue, server goes IDLE" <<endl;
            msgServiced = nullptr;
            emit(busySignal, false);
        }

        else { // Queue contains users, PSNP and PSWP order the messages when arrives, so they are kept FCFS
                msgServiced = (Msg1 *)queue.pop();
                msgQueueClassCount[msgServiced->getKind()]--;               //Update number of msg in queue (per class)
                if(msgServiced->getFirstServerEntrance()==0){               //If the msg meet the server for the first time
                    msgServiced->setFirstServerEntrance(simTime());
                }
                emit(qlenSignal, queue.getLength()); //Queue length changed, emit new length!
                //Waiting time: time from msg arrival to time msg enters the server (now)
                emit(queueingTimeSignal, simTime() - msgServiced->getTimestamp());
                emit(ClassQueueingTimeSignal[msgServiced->getKind()], simTime() - msgServiced->getTimestamp());
                EV << "Starting service of " << msgServiced->getName() << endl;
                //To set Service Time
                //Always, except if we are in PSWP resume and I'm dealing with a message that was kicked out
                if(strcmp(getParentModule()->par("policy"),"PSNP")==0 || (strcmp(getParentModule()->par("policy"),"PSWPresume")==0 && msgServiced->getResidual_time()==0)|| (strcmp(getParentModule()->par("policy"),"PSWPrestart")==0)){
                    double serviceTime = msgServiced->getServiceTime();
                    EV << "Service time " <<serviceTime<<" of "<< msgServiced->getName()<<endl;
                    scheduleAt(simTime()+serviceTime, endServiceMsg);
                }
                //In PSWP resume, if a message was kicked out
                else {
                    scheduleAt(simTime() + msgServiced->getResidual_time(), endServiceMsg);
                    EV<<"Time left for completion of service (because of preemption): "<<msgServiced->getResidual_time()<<endl;
                }
        }
    }

    else { // Data msg has arrived

        //Setting arrival timestamp as msg field
        messaggio->setTimestamp();

        if (!msgServiced) { //No message in service (server IDLE) ==> No queue ==> Direct service

            ASSERT(queue.getLength() == 0);

            msgServiced = messaggio;
            msgServiced->setFirstServerEntrance(simTime());
            emit(queueingTimeSignal, SIMTIME_ZERO);
            emit(ClassQueueingTimeSignal[msgServiced->getKind()], SIMTIME_ZERO);

            EV << "Starting service of " << msgServiced->getName() << endl;
            double serviceTime = msgServiced->getServiceTime();
            EV << "Service time " <<serviceTime<<" of "<< msgServiced->getName()<<endl;
            scheduleAt(simTime()+serviceTime, endServiceMsg);
            emit(busySignal, true);
        }
        else {  //Message in service (server BUSY) ==> Queuing or preemption

            //PSWP (restart or resume) case where I need to kick out the msg in service, so I need to perform a preemption
            if((strcmp(getParentModule()->par("policy"),"PSWPresume")==0 || strcmp(getParentModule()->par("policy"),"PSWPrestart")==0 )&& messaggio->getKind() < msgServiced->getKind()){
                EV << "Message " << msgServiced->getName()<< " will be preempted in favor of " << messaggio->getName() <<endl;

                if(strcmp(getParentModule()->par("policy"),"PSWPresume")==0){
                msgServiced->setResidual_time(endServiceMsg->getArrivalTime() - simTime());       //I set his residual time for preemptive resume
                }
                cancelEvent(endServiceMsg);
                msgAux=msgServiced;

                msgServiced = messaggio;                                    //I put in service the new msg
                msgServiced->setFirstServerEntrance(simTime());
                emit(queueingTimeSignal, SIMTIME_ZERO);
                emit(ClassQueueingTimeSignal[msgServiced->getKind()], SIMTIME_ZERO);
                EV << "Starting service of " << msgServiced->getName() << endl;
                double serviceTime = msgServiced->getServiceTime();                 //A msg that preempts another msg is the first time that arrives in the system
                EV << "Service time " <<serviceTime<<" of "<< msgServiced->getName()<<endl;
                scheduleAt(simTime()+serviceTime, endServiceMsg);


                EV << msgAux->getName() << " RE-enters queue"<< endl;       //The kicked out msg re-enters the queue
                insertOrderedQueue(msgAux);                                 //We call the method to insert the message in the right position (defined above)
                emit(qlenSignal, queue.getLength());                                         //Queue length changed, emit new length!
            }

            //PSNP or PSWP (restart or resume) where we don't need to perform a preemption operation
            else{
                EV << messaggio->getName() << " enters queue"<< endl;
                insertOrderedQueue(messaggio);
                emit(qlenSignal, queue.getLength()); //Queue length changed, emit new length!
            }
       }
    }
}
