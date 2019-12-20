simple Queue
{
    parameters:
        
        @display("i=block/queue;q=queue");
        @signal[qlen](type="long");
        @signal[busy](type="bool");
        @signal[queueingTime](type="simtime_t");
        @signal[responseTime](type="simtime_t");
        @signal[Class*-ResponseTime](type="simtime_t");
        @signal[Class*-QueueingTime](type="simtime_t");
	    @signal[Class*-ExtServiceTime](type="simtime_t");
		
        //Queue lenght
        @statistic[qlen](title="queue length";record=vector,timeavg,mean,max;interpolationmode=sample-hold);
        //Utilization of the server
        @statistic[busy](title="server busy state";record=vector?,timeavg,mean;interpolationmode=sample-hold);
        //Generic queueing time
        @statistic[queueingTime](title="queueing time at dequeue";unit=s;record=vector,mean,max;interpolationmode=none);
        //Generic response time
        @statistic[responseTime](title="response time at end of service";unit=s;record=vector,mean,max;interpolationmode=none);
        //Per-Class response time
        @statisticTemplate[ClassResponseTime](title="response time per class";unit=s;record=vector,timeavg,mean;interpolationmode=none);
        //Per-Class queueing time
		@statisticTemplate[ClassQueueingTime](title="queueing time per class";unit=s;record=vector,timeavg,mean;interpolationmode=none);
		//Per class extended service time
		@statisticTemplate[ClassExtServiceTime](title="extended service time per class";unit=s;record=vector,timeavg,mean;interpolationmode=none);		
    
    gates:
        input in[];		//Queue has a vector of input, the number is determined by the connection in the net.ned
        output out;
}