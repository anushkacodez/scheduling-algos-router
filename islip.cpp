#include <iostream>
#include <queue>
#include <vector>
#include <cstdlib>
#include <ctime>

using namespace std;

const int NUM_PORTS = 8;
const int BUFFER_SIZE = 64;
const int SIMULATION_TIME = 1000;  // Number of time units to run the simulation
const int PACKET_ARRIVAL_RATE = 2; // Packets arriving per unit time (uniform traffic)

// Define a structure for a Packet
struct Packet {
    int priority; // Priority of the packet (1 = low, 2 = medium, 3 = high)
    int arrivalTime;
    int processingTime;
    int outputPort;
    int size;
};

struct ComparePriority {
    bool operator()(const Packet& p1, const Packet& p2) {
        return p1.priority > p2.priority; // Higher priority packets have lower numbers
    }
};

class RouterSwitch {
public:
    // Each port now has 3 priority queues
    priority_queue<Packet, vector<Packet>, ComparePriority> inputQueues[NUM_PORTS][NUM_PORTS];
    queue<Packet>outputQueues[NUM_PORTS];
    int grantPointer[NUM_PORTS]={0};
    int acceptPointer[NUM_PORTS]={0};

    int bufferOccupancy[NUM_PORTS][NUM_PORTS] = {0};  // Buffer occupancy per port
    int packetsProcessed = 0;
    int totalTurnaroundTime = 0;
    int totalWaitingTime = 0;
    int totalPacketsDropped = 0;
    int totalArrivals = 0;  // Total packets that entered the system
    int queueThroughput[NUM_PORTS] = {0}; // Packets processed per port
    int totalBufferOccupancy[NUM_PORTS] = {0}; // Total buffer occupancy per port (for average calculation)
    int timeUnits[NUM_PORTS] = {0}; // Time units tracked per port

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
    for (int i = 0; i < NUM_PORTS; i++) {
        for (int j = 0; j < PACKET_ARRIVAL_RATE; j++) {
            Packet pkt;
            pkt.priority = rand() % 3 + 1;  // Random priority between 1 and 3
            pkt.arrivalTime = time;
            pkt.processingTime = rand() % 10 + 1; // Random processing time between 1 and 10 units
            pkt.outputPort = rand() % NUM_PORTS;  // Random output port
            pkt.size = rand() % 10 + 1; // Packet size between 1 and 10 units
            totalArrivals++;

            // Check if buffer is full
            if (bufferOccupancy[i][pkt.outputPort] < BUFFER_SIZE) {
                inputQueues[i][pkt.outputPort].push(pkt); // Place packet in the appropriate priority queue
                bufferOccupancy[i][pkt.outputPort]++;
                totalBufferOccupancy[i] += bufferOccupancy[i][pkt.outputPort];  // Track buffer occupancy
                timeUnits[i]++;
            } else {
                totalPacketsDropped++;
            }
        }
    }
}

// Generate packets at input ports (non-uniform traffic)
void RouterSwitch::generatePackets_non_uniform(int time) {
    for (int i = 0; i < NUM_PORTS; i++) {
        for (int j = 0; j < rand() % 10; j++) {
            Packet pkt;
            pkt.priority = rand() % 3 + 1;  // Random priority between 1 and 3
            pkt.arrivalTime = time;
            pkt.processingTime = rand() % 10 + 1; // Random processing time between 1 and 10 units
            pkt.outputPort = rand() % NUM_PORTS;  // Random output port
            pkt.size = rand() % 10 + 1; // Packet size between 1 and 10 units
            totalArrivals++;

            // Check if buffer is full
            if (bufferOccupancy[i][pkt.outputPort] < BUFFER_SIZE) {
                inputQueues[i][pkt.outputPort].push(pkt); // Place packet in the appropriate priority queue
                bufferOccupancy[i][pkt.outputPort]++;
                totalBufferOccupancy[i] += bufferOccupancy[i][pkt.outputPort];  // Track buffer occupancy
                timeUnits[i]++;
            } else {
                totalPacketsDropped++;
            }
        }
    }
}

// Generate bursty traffic at input ports
void RouterSwitch::generatePackets_bursty(int time) {
    for (int i = 0; i < NUM_PORTS; i++) {
        bool isBursty = (rand() % 100) < 30; // 30% chance for bursty traffic at a given time
        int arrivalRate = isBursty ? PACKET_ARRIVAL_RATE * 2 : PACKET_ARRIVAL_RATE / 2;

        for (int j = 0; j < arrivalRate; j++) {
            Packet pkt;
            pkt.priority = rand() % 3 + 1;  // Random priority between 1 and 3
            pkt.arrivalTime = time;
            pkt.processingTime = rand() % 10 + 1; // Random processing time between 1 and 10 units
            pkt.outputPort = rand() % NUM_PORTS;  // Random output port
            pkt.size = rand() % 10 + 1; // Packet size between 1 and 10 units
            totalArrivals++;

            if (bufferOccupancy[i][pkt.outputPort] < BUFFER_SIZE) {
                inputQueues[i][pkt.outputPort].push(pkt); // Place packet in the appropriate priority queue
                bufferOccupancy[i][pkt.outputPort]++;
                totalBufferOccupancy[i] += bufferOccupancy[i][pkt.outputPort];  // Track buffer occupancy
                timeUnits[i]++;
            } else {
                totalPacketsDropped++;
            }
        }
    }
}

// Process packets using Round Robin
void RouterSwitch::processPackets(int time) {
    
    //request phase
    int requests[NUM_PORTS][NUM_PORTS]={0};
    for(int inputPort=0; inputPort<NUM_PORTS; inputPort++){
        for(int outputPort=0; outputPort<NUM_PORTS; outputPort++){
            if(!inputQueues[inputPort][outputPort].empty()){
                requests[inputPort][outputPort]=1;
            }
        }
    }
    int granted[NUM_PORTS]={-1,-1,-1,-1,-1,-1,-1,-1};
    //grant phase
    for(int outputPort=0; outputPort<NUM_PORTS; outputPort++){
        int grant=grantPointer[outputPort];
        for(int i=grant; i<NUM_PORTS; i++){
            if(requests[i][outputPort]==1){
                granted[outputPort]=i;
                grantPointer[outputPort]=(i+1)%NUM_PORTS;
                break;
            }
        }
        if(granted[outputPort]==-1){
            for(int i=0; i<grant; i++){
                if(requests[i][outputPort]==1){
                    granted[outputPort]=i;
                    grantPointer[outputPort]=(i+1)%NUM_PORTS;
                    break;
                }
            }
        }
    }
    int accepted[NUM_PORTS]={-1,-1,-1,-1,-1,-1,-1,-1};
    //accept phase
    for(int inputPort=0; inputPort<NUM_PORTS; inputPort++){
        int accept=acceptPointer[inputPort];
        for(int i=accept; i<NUM_PORTS; i++){
            if(granted[i]==inputPort){
                accepted[inputPort]=i;
                acceptPointer[inputPort]=(i+1)%NUM_PORTS;
                break;
            }
        }
        if(accepted[inputPort]==-1){
            for(int i=0; i<accept; i++){
                if(granted[i]==inputPort){
                    accepted[inputPort]=i;
                    acceptPointer[inputPort]=(i+1)%NUM_PORTS;
                    break;
                }
            }
        }
    }
    for (int inputPort = 0; inputPort < NUM_PORTS; inputPort++) {
        if(accepted[inputPort]!=-1 && !inputQueues[inputPort][accepted[inputPort]].empty()){
            // Process the first packet in the queue
            int outputPort=accepted[inputPort];
            Packet pkt = inputQueues[inputPort][outputPort].top();
            inputQueues[inputPort][outputPort].pop();
            bufferOccupancy[inputPort][outputPort]--;

            int waitingTime = time - pkt.arrivalTime;
            totalWaitingTime += waitingTime;
            totalTurnaroundTime += waitingTime + pkt.processingTime;

            // Send the packet to the output queue
            outputQueues[outputPort].push(pkt);
            packetsProcessed++;
            queueThroughput[outputPort]++;
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
        cout << "Port " << i << ": " << (timeUnits[i] ?(double)((double) totalBufferOccupancy[i] / (double)timeUnits[i]) : 0) << " packets" << endl;
    }

    cout << "-----------------------------" << endl;
}

void RouterSwitch::simulate() {
    srand(time(0)); // Seed for random packet generation
    cout << "Enter 1 for generating uniform traffic" << endl;
    cout << "Enter 2 for generating non-uniform traffic" << endl;
    cout << "Enter 3 for generating bursty traffic" << endl;
    int choice;
    cin >> choice;

    for (int time = 0; time < SIMULATION_TIME; time++) {
        if (choice == 1) {
            generatePackets_uniform(time);
        } else if (choice == 2) {
            generatePackets_non_uniform(time);
        } else if (choice == 3) {
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
