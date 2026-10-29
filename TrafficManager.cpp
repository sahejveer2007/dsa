#include "../include/TrafficManager.h"
#include <iostream>
#include <iomanip>
#include <sstream>

// ANSI color codes for clean terminal presentation
#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31;1m"
#define COLOR_GREEN   "\033[32;1m"
#define COLOR_YELLOW  "\033[33;1m"
#define COLOR_BLUE    "\033[34;1m"
#define COLOR_MAGENTA "\033[35;1m"
#define COLOR_CYAN    "\033[36;1m"
#define COLOR_WHITE   "\033[37;1m"
#define COLOR_BG_RED  "\033[41;37;1m"

TrafficManager::TrafficManager(int defaultGreenTime)
    : currentGreenDirection(NORTH),
      currentGreenTicksRemaining(defaultGreenTime),
      defaultGreenDuration(defaultGreenTime),
      currentTick(1),
      totalVehiclesProcessed(0),
      totalWaitTimeSum(0),
      emergencyModeActive(false) {}

TrafficManager::~TrafficManager() {}

void TrafficManager::logVehicleEntry(const Vehicle& v) {
    VehicleAuditRecord rec;
    rec.id = v.id;
    rec.plate = v.plateNumber;
    rec.type = v.getTypeString();
    rec.priority = v.getPriorityString();
    rec.entryTick = currentTick;
    rec.exitTick = -1;
    rec.totalWaitTime = 0;
    rec.exited = false;
    auditRegistry.append(rec);
}

void TrafficManager::logVehicleExit(const std::string& vehicleId, int waitTime) {
    Node<VehicleAuditRecord>* node = auditRegistry.find([&](const VehicleAuditRecord& r) {
        return r.id == vehicleId && !r.exited;
    });

    if (node != nullptr) {
        node->data.exitTick = currentTick;
        node->data.totalWaitTime = waitTime;
        node->data.exited = true;
    }
}

void TrafficManager::addVehicleToLane(Direction dir, const Vehicle& vehicle) {
    Vehicle v = vehicle;
    v.entryTime = currentTick;
    laneQueues[dir].enqueue(v);
    logVehicleEntry(v);

    ControllerAction act;
    act.type = ACTION_ADD_VEHICLE;
    act.affectedDirection = dir;
    act.affectedVehicle = v;
    act.description = "Vehicle " + v.id + " (" + v.getTypeString() + ") enqueued to " + Vehicle::getDirectionName(dir) + " Lane.";
    undoStack.push(act);

    std::cout << COLOR_GREEN << " [+] " << act.description << COLOR_RESET << "\n";
}

void TrafficManager::addVehicleToBottleneck(const Vehicle& vehicle) {
    Vehicle v = vehicle;
    v.entryTime = currentTick;
    bottleneckLane.enqueue(v);
    logVehicleEntry(v);

    std::cout << COLOR_YELLOW << " [~] Vehicle " << v.id << " (" << v.getTypeString() 
              << ") entered the Narrow Corridor/Tunnel." << COLOR_RESET << "\n";
}

void TrafficManager::switchSignal(Direction newDirection, const std::string& reason, bool recordUndo) {
    if (recordUndo) {
        ControllerAction act;
        act.type = ACTION_CHANGE_SIGNAL;
        act.previousSignalState = SignalState(currentGreenDirection, currentGreenTicksRemaining, "Manual or Cycle Switch");
        act.description = "Signal switched from " + Vehicle::getDirectionName(currentGreenDirection) +
                          " to " + Vehicle::getDirectionName(newDirection) + " (" + reason + ")";
        undoStack.push(act);
    }

    currentGreenDirection = newDirection;
    currentGreenTicksRemaining = defaultGreenDuration;
    std::cout << COLOR_CYAN << " [Traffic Light] Switched to " << Vehicle::getDirectionName(newDirection) 
              << " (" << reason << ")" << COLOR_RESET << "\n";
}

void TrafficManager::triggerEmergencyPreemption(Direction emergencyDir, const Vehicle& emergencyVeh) {
    // 1. Push interrupted signal state onto preemption stack (LIFO)
    SignalState currentState(currentGreenDirection, currentGreenTicksRemaining, "Interrupted by " + emergencyVeh.getTypeString() + " " + emergencyVeh.id);
    preemptionStack.push(currentState);
    emergencyModeActive = true;

    // 2. Add emergency vehicle to the lane
    Vehicle v = emergencyVeh;
    v.entryTime = currentTick;
    laneQueues[emergencyDir].enqueue(v);
    logVehicleEntry(v);

    // 3. Immediately switch green light to this direction with emergency priority
    currentGreenDirection = emergencyDir;
    currentGreenTicksRemaining = 8; // Extended 8-second clearance time for Ambulance / Fire Truck

    // 4. Record action to undo stack
    ControllerAction act;
    act.type = ACTION_EMERGENCY_OVERRIDE;
    act.previousSignalState = currentState;
    act.affectedDirection = emergencyDir;
    act.affectedVehicle = v;
    act.description = "EMERGENCY PREEMPTION: " + v.getTypeString() + " " + v.id + 
                      " preempted intersection! Saved interrupted state (" + 
                      Vehicle::getDirectionName(currentState.activeDirection) + ") to Stack.";
    undoStack.push(act);

    std::cout << COLOR_BG_RED << " !!! " << act.description << COLOR_RESET << "\n";
    std::cout << COLOR_MAGENTA << " [Stack Update] Preemption Stack Depth: " << preemptionStack.size() 
              << " | Interrupted state stored at top." << COLOR_RESET << "\n";
}

void TrafficManager::resolveEmergencyPreemption() {
    if (preemptionStack.isEmpty()) {
        std::cout << COLOR_YELLOW << " [!] Preemption Stack is already empty. No emergency override to resolve." << COLOR_RESET << "\n";
        emergencyModeActive = false;
        return;
    }

    // Pop the interrupted state from Stack (LIFO restoration)
    SignalState previousState = preemptionStack.pop();
    currentGreenDirection = previousState.activeDirection;
    currentGreenTicksRemaining = previousState.remainingGreenTicks > 0 ? previousState.remainingGreenTicks : defaultGreenDuration;

    if (preemptionStack.isEmpty()) {
        emergencyModeActive = false;
    }

    std::cout << COLOR_GREEN << " [Preemption Cleared] Restored previous signal state from Stack: "
              << Vehicle::getDirectionName(currentGreenDirection) 
              << " with " << currentGreenTicksRemaining << "s remaining." << COLOR_RESET << "\n";
    std::cout << COLOR_MAGENTA << " [Stack Update] Remaining Preemption Stack Depth: " 
              << preemptionStack.size() << COLOR_RESET << "\n";
}

void TrafficManager::advanceSimulationTick() {
    std::cout << "\n" << COLOR_WHITE << "=== ADVANCING SIMULATION (Tick " << currentTick << " -> " << currentTick + 1 << ") ===" << COLOR_RESET << "\n";
    currentTick++;

    // Increment wait times for waiting vehicles
    for (int d = 0; d < 4; ++d) {
        CustomQueue<Vehicle> tempQueue;
        while (!laneQueues[d].isEmpty()) {
            Vehicle v = laneQueues[d].dequeue();
            if (d != currentGreenDirection) {
                v.waitTime++;
            }
            tempQueue.enqueue(v);
        }
        laneQueues[d] = tempQueue;
    }

    // Process vehicle from green lane if available
    if (!laneQueues[currentGreenDirection].isEmpty()) {
        if (emergencyModeActive) {
            // Give emergency vehicle extended transit time (clears when countdown finishes)
            if (currentGreenTicksRemaining <= 1) {
                Vehicle departed = laneQueues[currentGreenDirection].dequeue();
                totalVehiclesProcessed++;
                totalWaitTimeSum += departed.waitTime;
                logVehicleExit(departed.id, departed.waitTime);

                std::cout << COLOR_GREEN << " [--> Clearance] " << departed.getTypeString() << " (" << departed.id 
                          << ", Plate: " << departed.plateNumber << ") successfully cleared the junction!\n" << COLOR_RESET;
                std::cout << COLOR_CYAN << " [*] Emergency vehicle has completed transit. Resolving preemption stack...\n" << COLOR_RESET;
                resolveEmergencyPreemption();
                return;
            } else {
                std::cout << COLOR_RED << " [🚨 Priority Transit Active] " 
                          << laneQueues[currentGreenDirection].peek().getTypeString() 
                          << " (" << laneQueues[currentGreenDirection].peek().id 
                          << ") crossing junction corridor... (" << currentGreenTicksRemaining << "s clearance window remaining)\n" << COLOR_RESET;
            }
        } else {
            Vehicle departed = laneQueues[currentGreenDirection].dequeue();
            totalVehiclesProcessed++;
            totalWaitTimeSum += departed.waitTime;
            logVehicleExit(departed.id, departed.waitTime);

            std::cout << COLOR_GREEN << " [--> Departure] " << departed.getTypeString() << " (" << departed.id 
                      << ", Plate: " << departed.plateNumber << ") passed through " 
                      << Vehicle::getDirectionName(currentGreenDirection) 
                      << " Intersection. Total wait: " << departed.waitTime << " ticks." << COLOR_RESET << "\n";
        }
    } else {
        std::cout << COLOR_YELLOW << " [Signal Green] " << Vehicle::getDirectionName(currentGreenDirection) 
                  << " is GREEN, but lane is currently empty." << COLOR_RESET << "\n";
    }

    // Decrement green light timer
    currentGreenTicksRemaining--;
    if (currentGreenTicksRemaining <= 0) {
        if (!preemptionStack.isEmpty()) {
            std::cout << COLOR_YELLOW << " [*] Emergency green expired. Restoring previous state from preemption stack..." << COLOR_RESET << "\n";
            resolveEmergencyPreemption();
        } else {
            // Round-robin advance to next direction
            Direction nextDir = static_cast<Direction>((currentGreenDirection + 1) % 4);
            switchSignal(nextDir, "Standard Round-Robin Cycle", false);
        }
    }
}

void TrafficManager::divertBottleneckToSpur() {
    if (bottleneckLane.isEmpty()) {
        std::cout << COLOR_YELLOW << " [!] Bottleneck corridor is already clear. Nothing to divert." << COLOR_RESET << "\n";
        return;
    }

    std::cout << COLOR_RED << " [Bottleneck Gridlock Alert] Vehicles pulling into dead-end Spur Pocket (LIFO Stack)..." << COLOR_RESET << "\n";
    
    // Vehicles drive into the dead-end spur pocket one after another:
    // First car drives into the back of the pocket (Stack bottom).
    // Last car remains at the opening of the pocket (Stack top).
    int count = 0;
    while (!bottleneckLane.isEmpty()) {
        Vehicle v = bottleneckLane.dequeue();
        bottleneckSpurStack.push(v);
        count++;
        std::cout << COLOR_YELLOW << "    -> Pulled into Spur Pocket (Pushed to Stack): " 
                  << v.id << " (" << v.getTypeString() << ", Plate: " << v.plateNumber << ")" << COLOR_RESET << "\n";
    }

    std::cout << COLOR_GREEN << " [Bottleneck Cleared] " << count 
              << " vehicle(s) safely buffered in Spur Pocket Stack. Corridor is open!" << COLOR_RESET << "\n";
}

void TrafficManager::restoreBottleneckFromSpur() {
    if (bottleneckSpurStack.isEmpty()) {
        std::cout << COLOR_YELLOW << " [!] Evacuation Spur Stack is empty. No vehicles waiting." << COLOR_RESET << "\n";
        return;
    }

    std::cout << COLOR_GREEN << " [Corridor Free] Popping vehicles from Spur Pocket back to Corridor (LIFO order)..." << COLOR_RESET << "\n";
    while (!bottleneckSpurStack.isEmpty()) {
        Vehicle v = bottleneckSpurStack.pop();
        bottleneckLane.enqueue(v);
        std::cout << COLOR_CYAN << "    <- Popped from Spur Stack: " << v.id 
                  << " (" << v.getTypeString() << ") re-entered corridor." << COLOR_RESET << "\n";
    }
}

void TrafficManager::passBottleneckEmergencyVehicle(const Vehicle& amb) {
    std::cout << COLOR_BG_RED << " [HIGH PRIORITY TRANSIT] " << amb.getTypeString() 
              << " " << amb.id << " (Plate: " << amb.plateNumber << ") speeding through cleared bottleneck corridor!" 
              << COLOR_RESET << "\n";
    totalVehiclesProcessed++;
    logVehicleEntry(amb);
    logVehicleExit(amb.id, 0);
}

bool TrafficManager::undoLastAction() {
    if (undoStack.isEmpty()) {
        std::cout << COLOR_RED << " [!] Undo Stack is empty. Nothing to undo." << COLOR_RESET << "\n";
        return false;
    }

    ControllerAction act = undoStack.pop();
    std::cout << COLOR_YELLOW << " [UNDOING ACTION] " << act.description << COLOR_RESET << "\n";

    switch (act.type) {
        case ACTION_ADD_VEHICLE: {
            // Remove vehicle from rear of that lane queue
            Direction dir = act.affectedDirection;
            CustomQueue<Vehicle> temp;
            Vehicle last;
            int qSize = laneQueues[dir].size();
            for (int i = 0; i < qSize - 1; ++i) {
                temp.enqueue(laneQueues[dir].dequeue());
            }
            if (!laneQueues[dir].isEmpty()) {
                last = laneQueues[dir].dequeue();
            }
            laneQueues[dir] = temp;
            auditRegistry.removeIf([&](const VehicleAuditRecord& r) {
                return r.id == last.id && !r.exited;
            });
            std::cout << COLOR_GREEN << " [Undo Success] Removed " << last.id << " from " 
                      << Vehicle::getDirectionName(dir) << " lane." << COLOR_RESET << "\n";
            break;
        }
        case ACTION_CHANGE_SIGNAL: {
            currentGreenDirection = act.previousSignalState.activeDirection;
            currentGreenTicksRemaining = act.previousSignalState.remainingGreenTicks;
            std::cout << COLOR_GREEN << " [Undo Success] Reverted signal to " 
                      << Vehicle::getDirectionName(currentGreenDirection) << COLOR_RESET << "\n";
            break;
        }
        case ACTION_EMERGENCY_OVERRIDE: {
            if (!preemptionStack.isEmpty()) {
                resolveEmergencyPreemption();
            }
            std::cout << COLOR_GREEN << " [Undo Success] Emergency override cancelled and state reverted." << COLOR_RESET << "\n";
            break;
        }
        default:
            std::cout << COLOR_YELLOW << " [Undo Success] Reverted action." << COLOR_RESET << "\n";
            break;
    }
    return true;
}

void TrafficManager::displayDashboard() const {
    std::cout << "\n" << COLOR_CYAN << "================================================================================" << COLOR_RESET << "\n";
    std::cout << COLOR_WHITE << "             SMARTFLOW: URBAN BOTTLENECK & TRAFFIC CONTROL SYSTEM               " << COLOR_RESET << "\n";
    std::cout << COLOR_CYAN << "================================================================================" << COLOR_RESET << "\n";
    
    std::cout << " [SIMULATION TICK]: " << COLOR_YELLOW << currentTick << COLOR_RESET
              << " | [SYSTEM STATUS]: " << (emergencyModeActive ? (std::string(COLOR_RED) + "EMERGENCY OVERRIDE ACTIVE") : (std::string(COLOR_GREEN) + "NORMAL OPERATIONAL")) << COLOR_RESET
              << " | [THROUGHPUT]: " << COLOR_GREEN << totalVehiclesProcessed << " cars passed" << COLOR_RESET << "\n";
    std::cout << "--------------------------------------------------------------------------------\n";

    // 1. Visual 4-Way Intersection
    std::cout << COLOR_WHITE << " [1] 4-WAY INTERSECTION QUEUES (FIFO QUEUES):\n" << COLOR_RESET;
    for (int d = 0; d < 4; ++d) {
        Direction dir = static_cast<Direction>(d);
        bool isGreen = (dir == currentGreenDirection);
        std::cout << "   " << std::setw(6) << Vehicle::getDirectionName(dir) << " LANE "
                  << (isGreen ? (std::string(COLOR_GREEN) + "[🟢 GREEN (" + std::to_string(currentGreenTicksRemaining) + "s)]" + COLOR_RESET)
                              : (std::string(COLOR_RED) + "[🔴 RED  ]" + COLOR_RESET))
                  << " : ";

        if (laneQueues[d].isEmpty()) {
            std::cout << COLOR_WHITE << "(Empty)" << COLOR_RESET;
        } else {
            laneQueues[d].forEach([](const Vehicle& v, int idx) {
                if (idx > 0) std::cout << " -> ";
                if (v.isEmergency()) {
                    std::cout << COLOR_RED << v.getBadge() << " " << v.id << COLOR_RESET;
                } else {
                    std::cout << COLOR_WHITE << v.getBadge() << " " << v.id << COLOR_RESET;
                }
            });
        }
        std::cout << "\n";
    }

    std::cout << "--------------------------------------------------------------------------------\n";

    // 2. Emergency Preemption Stack (LIFO)
    std::cout << COLOR_WHITE << " [2] EMERGENCY SIGNAL PREEMPTION STACK (LIFO):\n" << COLOR_RESET;
    if (preemptionStack.isEmpty()) {
        std::cout << "   " << COLOR_YELLOW << "[Empty Stack] (No interrupted signal states held)" << COLOR_RESET << "\n";
    } else {
        std::cout << "   [TOP OF STACK]\n";
        preemptionStack.forEach([](const SignalState& st, int idx) {
            std::cout << "    |-- Depth " << idx << ": Stored Lane " << Vehicle::getDirectionName(st.activeDirection)
                      << " (" << st.remainingGreenTicks << "s remaining) | Reason: " << st.triggerReason << "\n";
        });
        std::cout << "   [BOTTOM OF STACK]\n";
    }

    std::cout << "--------------------------------------------------------------------------------\n";

    // 3. Bottleneck Corridor & Spur Stack (LIFO)
    std::cout << COLOR_WHITE << " [3] NARROW TUNNEL / CORRIDOR & EVACUATION SPUR POCKET (LIFO STACK):\n" << COLOR_RESET;
    std::cout << "   Corridor Lane: ";
    if (bottleneckLane.isEmpty()) {
        std::cout << COLOR_GREEN << "[CLEAR / EMPTY]" << COLOR_RESET << "\n";
    } else {
        bottleneckLane.forEach([](const Vehicle& v, int idx) {
            if (idx > 0) std::cout << " -> ";
            std::cout << COLOR_YELLOW << v.getBadge() << " " << v.id << COLOR_RESET;
        });
        std::cout << "\n";
    }

    std::cout << "   Turnout Spur Pocket Stack (LIFO): ";
    if (bottleneckSpurStack.isEmpty()) {
        std::cout << COLOR_WHITE << "(Empty Pocket)" << COLOR_RESET << "\n";
    } else {
        std::cout << "\n   [SPUR TOP (Exits First)]\n";
        bottleneckSpurStack.forEach([](const Vehicle& v, int idx) {
            std::cout << "     [" << idx << "] " << v.getBadge() << " " << v.id 
                      << " (" << v.plateNumber << ", " << v.getTypeString() << ")\n";
        });
        std::cout << "   [SPUR BOTTOM (Exits Last)]\n";
    }

    std::cout << "--------------------------------------------------------------------------------\n";

    // 4. Undo Stack
    std::cout << COLOR_WHITE << " [4] CONTROLLER ACTION UNDO STACK: " << COLOR_RESET
              << COLOR_MAGENTA << undoStack.size() << " action(s) logged." << COLOR_RESET << "\n";
    std::cout << COLOR_CYAN << "================================================================================" << COLOR_RESET << "\n";
}

void TrafficManager::searchVehicle(const std::string& query) const {
    std::cout << "\n" << COLOR_CYAN << "--- SEARCHING VEHICLE REGISTRY FOR: '" << query << "' ---" << COLOR_RESET << "\n";
    bool found = false;
    auditRegistry.forEach([&](const VehicleAuditRecord& r, int idx) {
        if (r.id == query || r.plate == query) {
            found = true;
            std::cout << COLOR_GREEN << " [FOUND MATCH #" << (idx + 1) << "]" << COLOR_RESET << "\n";
            std::cout << "  Vehicle ID    : " << r.id << "\n";
            std::cout << "  License Plate : " << r.plate << "\n";
            std::cout << "  Type          : " << r.type << "\n";
            std::cout << "  Priority      : " << r.priority << "\n";
            std::cout << "  Entry Tick    : " << r.entryTick << "\n";
            std::cout << "  Status        : " << (r.exited ? (std::string(COLOR_GREEN) + "DEPARTED at Tick " + std::to_string(r.exitTick) + " (Wait: " + std::to_string(r.totalWaitTime) + " ticks)" + COLOR_RESET)
                                                            : (std::string(COLOR_YELLOW) + "STILL IN QUEUE / SYSTEM" + COLOR_RESET)) << "\n";
        }
    });

    if (!found) {
        std::cout << COLOR_RED << " [X] No vehicle found matching ID or License Plate: " << query << COLOR_RESET << "\n";
    }
}

void TrafficManager::printAuditRegistry() const {
    std::cout << "\n" << COLOR_CYAN << "============================== VEHICLE AUDIT REGISTRY (LINKED LIST) ==============================" << COLOR_RESET << "\n";
    std::cout << std::left << std::setw(12) << "ID"
              << std::setw(20) << "Plate"
              << std::setw(16) << "Type"
              << std::setw(22) << "Priority"
              << std::setw(12) << "Entry Tick"
              << std::setw(12) << "Exit Tick"
              << std::setw(10) << "Wait Time" << "\n";
    std::cout << "--------------------------------------------------------------------------------------------------\n";

    auditRegistry.forEach([](const VehicleAuditRecord& r, int idx) {
        std::cout << std::left << std::setw(12) << r.id
                  << std::setw(20) << r.plate
                  << std::setw(16) << r.type
                  << std::setw(22) << r.priority
                  << std::setw(12) << r.entryTick
                  << std::setw(12) << (r.exited ? std::to_string(r.exitTick) : "IN TRANSIT")
                  << std::setw(10) << r.totalWaitTime << "\n";
    });
    std::cout << COLOR_CYAN << "==================================================================================================" << COLOR_RESET << "\n";
}

void TrafficManager::printMetrics() const {
    std::cout << "\n" << COLOR_CYAN << "========================= TRAFFIC PERFORMANCE METRICS =========================" << COLOR_RESET << "\n";
    std::cout << "  Total Simulation Ticks   : " << currentTick << "\n";
    std::cout << "  Total Vehicles Processed : " << totalVehiclesProcessed << "\n";
    double avgWait = totalVehiclesProcessed > 0 ? (double)totalWaitTimeSum / totalVehiclesProcessed : 0.0;
    std::cout << "  Average Wait Time        : " << std::fixed << std::setprecision(2) << avgWait << " ticks\n";
    std::cout << "  Active Preemption Depth  : " << preemptionStack.size() << "\n";
    std::cout << "  Spur Stack Occupancy     : " << bottleneckSpurStack.size() << "\n";
    std::cout << "  Undo Stack Depth         : " << undoStack.size() << "\n";
    std::cout << COLOR_CYAN << "================================================================================" << COLOR_RESET << "\n";
}
