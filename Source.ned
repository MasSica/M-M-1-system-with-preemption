simple Source
{
    parameters:
        volatile double interArrivalTime @unit(s);
        volatile double serviceTime @unit(s);
        @display("i=block/source");
        int SourceOfClass;
    gates:
        output out;
}