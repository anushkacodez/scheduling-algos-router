#include <iostream>
#include <queue>
#include <vector>
#include <cstdlib>
#include <ctime>

using namespace std;

const int NUM_PORTS = 8;
const int BUFFER_SIZE = 64;
const int SIMULATION_TIME = 1000;  // Number of time units to run the simulation
const int PACKET_ARRIVAL_RATE = 4; // Packets arriving per unit time (uniform traffic)


// Define a structure for a Packet
struct Packet {
    int priority; // Priority of the packet (lower number = higher priority)
    int arrivalTime;
    int processingTime;
    int outputPort;
};
struct ComparePriority {
    bool operator()(const Packet& p1, const Packet& p2) {
        return p1.priority > p2.priority; // Higher priority packets have lower numbers
    }
};

class RouterSwitch {
public:

    priority_queue<Packet, vector<Packet>, ComparePriority> inputQueues[NUM_PORTS][NUM_PORTS];
    queue<Packet> outputQueues[NUM_PORTS];

    queue<int>pendingInputPorts[NUM_PORTS];

    int generatedPacketCountForEachInputPort[NUM_PORTS]={0};
    int bufferOccupancy[NUM_PORTS][NUM_PORTS] = {0};
    int totalBufferOccupancy[NUM_PORTS] = {0}; // Accumulating buffer occupancy per port
    int timeUnits[NUM_PORTS] = {0};            // Time units in which each port was active
    int packetsProcessed = 0;
    int totalTurnaroundTime = 0;
    int totalWaitingTime = 0;
    int totalPacketsDropped = 0;
    int totalArrivals = 0;                     // Total packets that attempted to enter the system
    int queueThroughput[NUM_PORTS] = {0};  
    
    // Packets processed per port
    RouterSwitch() {}
    void simulate();
    void generatePackets_uniform(int time);
    void generatePackets_non_uniform(int time);
    void generatePackets_bursty(int time);
    void processPackets(int time);
    void printStatistics(int time);
};

// Generate packets at input ports
void RouterSwitch::generatePackets_uniform(int time) {
    int addHua[NUM_PORTS][NUM_PORTS]={0};

    for (int i = 0; i < NUM_PORTS; i++) {
        for(int j=0; j< PACKET_ARRIVAL_RATE; j++){

            Packet pkt;
            pkt.priority = rand() % 3 + 1;  // Random priority between 1 and 5
            pkt.arrivalTime = time;
            pkt.processingTime = rand() % 10 + 1; // Random processing time between 1 and 10 units
            pkt.outputPort = rand() % NUM_PORTS;  // Random output port
            
            generatedPacketCountForEachInputPort[i]+=PACKET_ARRIVAL_RATE;
            totalArrivals++;

            // Check if buffer is full
            if (bufferOccupancy[i][pkt.outputPort] < BUFFER_SIZE) {
                inputQueues[i][pkt.outputPort].push(pkt);
                bufferOccupancy[i][pkt.outputPort]++;
                totalBufferOccupancy[i] += bufferOccupancy[i][pkt.outputPort];  // Accumulate buffer occupancy for average calculation
                timeUnits[i]++; // Track time unit for the port
                addHua[i][pkt.outputPort]=1;

            } else {
                totalPacketsDropped++;
            }
        }
    }
    for(int i=0; i<NUM_PORTS; i++){
        for(int j=0; j<NUM_PORTS; j++){
            if(addHua[i][j]==1){
                pendingInputPorts[j].push(i);
            }
        }
    }
}

// Generate packets at input ports
void RouterSwitch::generatePackets_non_uniform(int time) {
    int addHua[NUM_PORTS][NUM_PORTS]={0};

    for (int i = 0; i < NUM_PORTS; i++) {
        int arrivalRate=rand()%10;
        for(int j=0; j< arrivalRate; j++){

            Packet pkt;
            pkt.priority = rand() % 3 + 1;  // Random priority between 1 and 5
            pkt.arrivalTime = time;
            pkt.processingTime = rand() % 10 + 1; // Random processing time between 1 and 10 units
            pkt.outputPort = rand() % NUM_PORTS;  // Random output port
            
            generatedPacketCountForEachInputPort[i]+=arrivalRate;
            totalArrivals++;

            // Check if buffer is full
            if (bufferOccupancy[i][pkt.outputPort] < BUFFER_SIZE) {
                inputQueues[i][pkt.outputPort].push(pkt);
                bufferOccupancy[i][pkt.outputPort]++;
                totalBufferOccupancy[i] += bufferOccupancy[i][pkt.outputPort];  // Accumulate buffer occupancy for average calculation
                timeUnits[i]++; // Track time unit for the port
                addHua[i][pkt.outputPort]=1;

            } else {
                totalPacketsDropped++;
            }
        }
    }
    for(int i=0; i<NUM_PORTS; i++){
        for(int j=0; j<NUM_PORTS; j++){
            if(addHua[i][j]==1){
                pendingInputPorts[j].push(i);
            }
        }
    }
}

// Generate bursty traffic at input ports
void RouterSwitch::generatePackets_bursty(int time) {
    int addHua[NUM_PORTS][NUM_PORTS]={0};

    for (int i = 0; i < NUM_PORTS; i++) {
        // Randomly decide if this port is in a bursty period
        bool isBursty = (rand() % 100) < 30; // 30% chance for bursty traffic at a given time

        // If in bursty period, generate more packets
        int arrivalRate = isBursty ? PACKET_ARRIVAL_RATE * 2 : PACKET_ARRIVAL_RATE / 2;
        // Generate packets with higher or lower rate depending on burstiness
        for(int j=0; j<arrivalRate; j++) {
            Packet pkt;
            pkt.priority = rand() % 3 + 1;  // Random priority between 1 and 5
            pkt.arrivalTime = time;
            pkt.processingTime = rand() % 10 + 1; // Random processing time between 1 and 10 units
            pkt.outputPort = rand() % NUM_PORTS;  // Random output port
            
            generatedPacketCountForEachInputPort[i]+=arrivalRate;
            totalArrivals++;

            // Check if buffer is full
            if (bufferOccupancy[i][pkt.outputPort] < BUFFER_SIZE) {
                inputQueues[i][pkt.outputPort].push(pkt);
                bufferOccupancy[i][pkt.outputPort]++;
                totalBufferOccupancy[i] += bufferOccupancy[i][pkt.outputPort];  // Accumulate buffer occupancy for average calculation
                timeUnits[i]++; // Track time unit for the port
                addHua[i][pkt.outputPort]=1;
            } else {
                totalPacketsDropped++;
            }
        }
    }
    for(int i=0; i<NUM_PORTS; i++){
        for(int j=0; j<NUM_PORTS; j++){
            if(addHua[i][j]==1){
                pendingInputPorts[j].push(i);
            }
        }
    }
}

// Process packets at output ports
void RouterSwitch::processPackets(int time) {

    int inputPortCorrespondingToOutputPort[NUM_PORTS]={-1, -1, -1, -1, -1, -1, -1, -1};
    int outputPortCorrespondingToInputPort[NUM_PORTS]={-1, -1, -1, -1, -1, -1, -1, -1};

    //input port priority order = 1,2,3,4,5,6,7,8
    //output port priority order= 1,2,3,4,5,6,7,8
    //apply stabe matching for them
    for(int outputPort=0; outputPort<NUM_PORTS; outputPort++)
    {
        if(!pendingInputPorts[outputPort].empty()){
            int candidate=pendingInputPorts[outputPort].front();
            if(outputPortCorrespondingToInputPort[candidate]==-1){
                inputPortCorrespondingToOutputPort[outputPort]=candidate;
                outputPortCorrespondingToInputPort[candidate]=outputPort;
                pendingInputPorts[outputPort].pop();
            }
        }
    }
    // for(int i=0; i<NUM_PORTS; i++)
    // {
    //     cout<<inputPortCorrespondingToOutputPort[i]<<" ";
    // }
    // cout<<endl;
    // for(int i=0; i<NUM_PORTS; i++)
    // {
    //     cout<<outputPortCorrespondingToInputPort[i]<<" ";
    // }
    // cout<<endl;
    for(int outputPort=0; outputPort<NUM_PORTS; outputPort++)
    {
        if(inputPortCorrespondingToOutputPort[outputPort]!=-1){   
            int inputPort=inputPortCorrespondingToOutputPort[outputPort];
            if(!inputQueues[inputPort][outputPort].empty()){
                Packet pkt = inputQueues[inputPort][outputPort].top();
                inputQueues[inputPort][outputPort].pop();
                bufferOccupancy[inputPort][outputPort]--;

                // totalBufferOccupancy[i] += bufferOccupancy[i];

                int waitingTime = time - pkt.arrivalTime;
                totalWaitingTime += waitingTime;
                totalTurnaroundTime += waitingTime + pkt.processingTime;

                outputQueues[outputPort].push(pkt);
                packetsProcessed++;
                queueThroughput[outputPort]++;
                if (!inputQueues[inputPort][outputPort].empty()) {
                    pendingInputPorts[outputPort].push(inputPort);  // Re-add input port to the pending queue
                }
            }
        }
    }
}

void RouterSwitch::printStatistics(int time) {
    cout << "Simulation Time: " << time << " units" << endl;
    cout << "Total Packets Processed: " << packetsProcessed << endl;

    // Queue throughput per port
    cout << "Queue Throughput per port: " << endl;
    for (int i = 0; i < NUM_PORTS; i++) {
        cout << "Port " << i << ": " << queueThroughput[i] << " packets" << endl;
    }

    // Turnaround time and waiting time
    cout << "Average Turnaround Time: " << (packetsProcessed ? totalTurnaroundTime / packetsProcessed : 0) << " units" << endl;
    cout << "Average Waiting Time: " << (packetsProcessed ? totalWaitingTime / packetsProcessed : 0) << " units" << endl;

    // Packet drop rate
    cout << "Total Packets Dropped: " << totalPacketsDropped << endl;
    cout << "Packet Drop Rate: " << (totalArrivals ? (double)totalPacketsDropped / totalArrivals * 100 : 0) << "%" << endl;

    // Average Buffer Occupancy stats
    cout << "Average Buffer Occupancy per port: " << endl;
    for (int i = 0; i < NUM_PORTS; i++) {
        cout << "Port " << i << ": " << (timeUnits[i] ? (double)((double)totalBufferOccupancy[i] / (double)(timeUnits[i])) : 0) << " packets" << endl;
    }

    // cout << "Time units per port: " << endl;
    // for (int i = 0; i < NUM_PORTS; i++) {
    //     cout << "Port " << i << ": " << (timeUnits[i] ? ((double)timeUnits[i]) : 0) << "" << endl;
    // }

    // cout<<"Packets generated for every input port"<<endl;
    // for (int i = 0; i < NUM_PORTS; i++) {
    //     cout << "Port " << i << ": " << (generatedPacketCountForEachInputPort[i]) << " packets" << endl;
    // }

    cout << "-----------------------------" << endl;
}

void RouterSwitch::simulate() {
    srand(time(0)); // Seed for random packet generation
    cout<<"Enter 1 for generating uniform traffic"<<endl;
    cout<<"Enter 2 for generating non-uniform traffic"<<endl;
    cout<<"Enter 3 for generating bursty traffic"<<endl;
    int choice;
    cin>>choice;
    for (int time = 0; time < SIMULATION_TIME; time++) {
        if(choice==1){
            generatePackets_uniform(time);
        }
        else if(choice==2){
            generatePackets_non_uniform(time);
        }
        else if(choice==3){
            generatePackets_bursty(time);
        }
        processPackets(time);
    }
    printStatistics(SIMULATION_TIME);
}

int main() {
    RouterSwitch router;
    router.simulate();
    return 0;
}
