network Net
{
    parameters:
        int n;								//number of classes
        string policy;						//policy, PSWP or PSNP
        
    submodules:
        gen[n]: Source {					//Generator is declarated as an array of sources
            parameters:
                @display("p=89,100");
                SourceOfClass = 1+index;	//To write in the param SourceOfClass (contained in "Source.ned") its number. It will be used as priority 
        }
        sys: Queue {
            parameters:
                @display("p=329,100");
        }
        sink: Sink {
            parameters:
                @display("p=569,100");
        }
    connections:
    for i=0..n-1{    				//to connect the output of all the sources to the queue; 
        gen[i].out --> sys.in++;	//note sys.in++ needed to add an element in the vector on inputs of the queue 
    }
    
        sys.out --> sink.in;
}
