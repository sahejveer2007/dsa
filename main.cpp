#include <iostream>
#include <string>

using namespace std;

// ============================================================================
// 1. VEHICLE STRUCTURE
// ============================================================================
struct Vehicle {
    string id;
    string type; // "Car", "Ambulance", "FireTruck"
};

// ============================================================================
// 2. NODE FOR QUEUE (Used for Traffic Lanes)
// ============================================================================
struct Node {
    Vehicle data;
    Node* next;

    Node(Vehicle v) {
        data = v;
        next = NULL;
    }
};

// ============================================================================
// 3. QUEUE IMPLEMENTATION (FIFO - First In, First Out)
// Used for: North, East, South, West Road Lanes
// ============================================================================
class Queue {
private:
    Node* front;
    Node* rear;

public:
    Queue() {
        front = NULL;
        rear = NULL;
    }

    bool isEmpty() {
        return front == NULL;
    }

    // O(1) Enqueue: Add vehicle at the back (rear)
    void enqueue(Vehicle v) {
        Node* newNode = new Node(v);
        if (rear == NULL) {
            front = rear = newNode;
        } else {
            rear->next = newNode;
            rear = newNode;
        }
    }

    // O(1) Dequeue: Remove vehicle from the front
    Vehicle dequeue() {
        if (isEmpty()) {
            return {"EMPTY", "EMPTY"};
        }
        Node* temp = front;
        Vehicle v = temp->data;
        front = front->next;

        if (front == NULL) {
            rear = NULL;
        }
        delete temp; // Free memory
        return v;
    }

    // Display all cars waiting in this lane
    void display() {
        if (isEmpty()) {
            cout << "(Empty)";
            return;
        }
        Node* temp = front;
        while (temp != NULL) {
            cout << "[" << temp->data.id << ":" << temp->data.type << "] -> ";
            temp = temp->next;
        }
    }
};

// ============================================================================
// 4. SIGNAL STATE (Stores paused traffic light info)
// ============================================================================
struct SignalState {
    int laneIndex;         // 0: North, 1: East, 2: South, 3: West
    int remainingSeconds;  // Remaining green time when interrupted
    string reason;         // "Ambulance" or "Fire Truck"
};

// ============================================================================
// 5. STACK NODE (For Emergency Preemption)
// ============================================================================
struct StackNode {
    SignalState state;
    StackNode* next;

    StackNode(SignalState s) {
        state = s;
        next = NULL;
    }
};

// ============================================================================
// 6. STACK IMPLEMENTATION (LIFO - Last In, First Out)
// Used for: Emergency Preemption Memory
// ============================================================================
class Stack {
private:
    StackNode* top;

public:
    Stack() {
        top = NULL;
    }

    bool isEmpty() {
        return top == NULL;
    }

    // O(1) Push: Save paused light state on top
    void push(SignalState s) {
        StackNode* newNode = new StackNode(s);
        newNode->next = top;
        top = newNode;
    }

    // O(1) Pop: Restore the last saved light state
    SignalState pop() {
        if (isEmpty()) {
            return {-1, 0, "NONE"};
        }
        StackNode* temp = top;
        SignalState s = temp->state;
        top = top->next;
        delete temp; // Free memory
        return s;
    }

    // Display the stack from top to bottom
    void display(string laneNames[]) {
        if (isEmpty()) {
            cout << "  [Stack is Empty - Normal Flow]" << endl;
            return;
        }
        cout << "  --- STACK TOP (Pops First) ---" << endl;
        StackNode* temp = top;
        while (temp != NULL) {
            cout << "  | Saved: " << laneNames[temp->state.laneIndex] 
                 << " Lane (" << temp->state.remainingSeconds << "s remaining) | Reason: " 
                 << temp->state.reason << endl;
            temp = temp->next;
        }
        cout << "  --- STACK BOTTOM -------------" << endl;
    }
};

// ============================================================================
// 7. MAIN TRAFFIC CONTROLLER SIMULATION
// ============================================================================
int main() {
    string laneNames[4] = {"NORTH", "EAST", "SOUTH", "WEST"};
    Queue lanes[4];        // 4 FIFO Queues for the 4 approaches
    Stack emergencyStack;  // 1 LIFO Stack for saving interrupted lights

    int currentGreen = 0;  // Starts with North Green
    int timer = 6;         // 6 seconds per green phase
    bool isEmergency = false;
    int carCount = 100;

    // Add 2 sample cars initially
    lanes[0].enqueue({"CAR-101", "Car"});
    lanes[1].enqueue({"CAR-102", "Car"});

    int choice = -1;
    while (choice != 0) {
        cout << "\n========================================================" << endl;
        cout << "   TRAFFIC MANAGEMENT SYSTEM (DSA: STACK & QUEUE)       " << endl;
        cout << "========================================================" << endl;
        cout << " Active Light : " << laneNames[currentGreen] << " GREEN (" << timer << "s remaining)" << endl;
        cout << " Status       : " << (isEmergency ? "EMERGENCY OVERRIDE ACTIVE" : "NORMAL CYCLE") << endl;
        cout << "--------------------------------------------------------" << endl;
        
        cout << "1. Display Traffic Dashboard (Lanes & Stack)" << endl;
        cout << "2. Add Regular Car (Queue Enqueue)" << endl;
        cout << "3. Dispatch Ambulance (Stack Push & Preempt West)" << endl;
        cout << "4. Dispatch Fire Truck (Stack Push & Preempt South)" << endl;
        cout << "5. Advance Time (+1 Second / Dequeue Car)" << endl;
        cout << "6. Run Automated Viva Demo" << endl;
        cout << "0. Exit" << endl;
        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice) {
            case 1: {
                // Display 4 Queues
                cout << "\n--- INTERSECTION LANES (FIFO QUEUES) ---" << endl;
                for (int i = 0; i < 4; i++) {
                    cout << " " << laneNames[i] << " Lane " 
                         << (i == currentGreen ? "[GREEN] : " : "[RED]   : ");
                    lanes[i].display();
                    cout << endl;
                }

                // Display Emergency Stack
                cout << "\n--- EMERGENCY PREEMPTION STACK (LIFO) ---" << endl;
                emergencyStack.display(laneNames);
                break;
            }

            case 2: {
                int d;
                cout << "Select Lane (0:North, 1:East, 2:South, 3:West): ";
                cin >> d;
                if (d >= 0 && d < 4) {
                    carCount++;
                    string id = "CAR-" + to_string(carCount);
                    lanes[d].enqueue({id, "Car"});
                    cout << " [+] " << id << " added to " << laneNames[d] << " queue." << endl;
                } else {
                    cout << "Invalid lane!" << endl;
                }
                break;
            }

            case 3: {
                // AMBULANCE DISPATCH:
                // 1. PUSH current state to Stack
                emergencyStack.push({currentGreen, timer, "Ambulance"});
                isEmergency = true;

                // 2. Preempt West Lane (3) with 8 seconds
                currentGreen = 3;
                timer = 8;
                lanes[3].enqueue({"AMB-911", "Ambulance"});

                cout << "\n [!] EMERGENCY: Ambulance arrived at WEST Lane!" << endl;
                cout << " [!] Paused signal pushed to Stack. WEST is now GREEN for 8s." << endl;
                break;
            }

            case 4: {
                // FIRE TRUCK DISPATCH:
                // 1. PUSH current state to Stack
                emergencyStack.push({currentGreen, timer, "Fire Truck"});
                isEmergency = true;

                // 2. Preempt South Lane (2) with 8 seconds
                currentGreen = 2;
                timer = 8;
                lanes[2].enqueue({"FIRE-101", "FireTruck"});

                cout << "\n [!] EMERGENCY: Fire Truck arrived at SOUTH Lane!" << endl;
                cout << " [!] Paused signal pushed to Stack. SOUTH is now GREEN for 8s." << endl;
                break;
            }

            case 5: {
                // ADVANCE TIME BY 1 SECOND
                cout << "\n--- Advancing 1 Second ---" << endl;
                
                // If light is green and has vehicles, dequeue 1 vehicle every 2 seconds
                if (!lanes[currentGreen].isEmpty() && timer % 2 == 0) {
                    Vehicle v = lanes[currentGreen].dequeue();
                    cout << " [--> Departure] " << v.id << " (" << v.type 
                         << ") crossed the intersection from " << laneNames[currentGreen] << " Lane." << endl;
                }

                timer--;

                // When timer finishes (reaches 0)
                if (timer <= 0) {
                    // Check if there is an emergency state to restore from Stack (LIFO Pop)
                    if (!emergencyStack.isEmpty()) {
                        SignalState saved = emergencyStack.pop();
                        currentGreen = saved.laneIndex;
                        timer = saved.remainingSeconds;
                        cout << " [*] Emergency cleared! Restored " << laneNames[currentGreen] 
                             << " Lane from Stack with " << timer << "s remaining." << endl;
                        if (emergencyStack.isEmpty()) isEmergency = false;
                    } else {
                        // Normal round-robin cycle: North -> East -> South -> West
                        currentGreen = (currentGreen + 1) % 4;
                        timer = 6;
                        cout << " [Light Change] " << laneNames[currentGreen] << " is now GREEN (6s)." << endl;
                    }
                }
                break;
            }

            case 6: {
                // AUTOMATED VIVA DEMONSTRATION SCRIPT
                cout << "\n========================================================" << endl;
                cout << "         RUNNING AUTOMATED VIVA DEMONSTRATION           " << endl;
                cout << "========================================================" << endl;
                
                cout << "\n[Step 1] Adding regular cars to North and East lanes..." << endl;
                lanes[0].enqueue({"CAR-103", "Car"});
                lanes[1].enqueue({"CAR-104", "Car"});
                
                cout << "\n[Step 2] Displaying Queues (North is Green):" << endl;
                for (int i = 0; i < 4; i++) {
                    cout << " " << laneNames[i] << " Lane: ";
                    lanes[i].display();
                    cout << endl;
                }

                cout << "\n[Step 3] Ambulance arrives! Pushing North light (4s) to Stack..." << endl;
                emergencyStack.push({currentGreen, 4, "Ambulance"});
                isEmergency = true;
                currentGreen = 3; // West
                timer = 8;
                lanes[3].enqueue({"AMB-911", "Ambulance"});

                cout << "\n[Step 4] Viewing Stack (Notice North is stored at top):" << endl;
                emergencyStack.display(laneNames);

                cout << "\n[Step 5] Emergency clears! Popping Stack to restore North light..." << endl;
                SignalState saved = emergencyStack.pop();
                currentGreen = saved.laneIndex;
                timer = saved.remainingSeconds;
                isEmergency = false;
                cout << " Successfully popped! Restored " << laneNames[currentGreen] 
                     << " Lane with " << timer << "s remaining." << endl;
                cout << "\n[Done] Automated Demo finished smoothly!" << endl;
                break;
            }

            case 0: {
                cout << "\nExiting Traffic System. Thank you!" << endl;
                break;
            }

            default:
                cout << "Invalid choice! Enter 0 to 6." << endl;
        }
    }

    return 0;
}
