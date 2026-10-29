#ifndef TRAFFIC_MANAGER_H
#define TRAFFIC_MANAGER_H

#include "DataStructures.h"
#include "Vehicle.h"
#include <string>
#include <vector>

// State of traffic light at the intersection
struct SignalState {
    Direction activeDirection;
    int remainingGreenTicks;
    std::string triggerReason;

    SignalState() : activeDirection(NORTH), remainingGreenTicks(0), triggerReason("") {}
    SignalState(Direction dir, int ticks, const std::string& reason)
        : activeDirection(dir), remainingGreenTicks(ticks), triggerReason(reason) {}
};

// Types of actions for Undo history
enum ActionType {
    ACTION_ADD_VEHICLE,
    ACTION_CHANGE_SIGNAL,
    ACTION_EMERGENCY_OVERRIDE,
    ACTION_RESOLVE_EMERGENCY,
    ACTION_BOTTLENECK_DIVERT,
    ACTION_BOTTLENECK_RESTORE
};

struct ControllerAction {
    ActionType type;
    std::string description;
    Direction affectedDirection;
    Vehicle affectedVehicle;
    SignalState previousSignalState;

    ControllerAction() : type(ACTION_ADD_VEHICLE), description(""), affectedDirection(NORTH) {}
};

// Vehicle Audit Record for Linked List
struct VehicleAuditRecord {
    std::string id;
    std::string plate;
    std::string type;
    std::string priority;
    int entryTick;
    int exitTick;
    int totalWaitTime;
    bool exited;

    VehicleAuditRecord() : id(""), plate(""), type(""), priority(""), entryTick(0), exitTick(-1), totalWaitTime(0), exited(false) {}
};

class TrafficManager {
private:
    // 1. Intersection Queues (FIFO) - 4 Directions
    CustomQueue<Vehicle> laneQueues[4];

    // 2. Emergency Preemption Stack (LIFO) - Saves interrupted signal states
    CustomStack<SignalState> preemptionStack;

    // 3. Bottleneck Corridor Evacuation Spur Pocket (LIFO) - Physical dead-end turnout pocket
    CustomStack<Vehicle> bottleneckSpurStack;

    // Bottleneck lane buffer (active corridor)
    CustomQueue<Vehicle> bottleneckLane;

    // 4. Operator Action Undo Stack (LIFO)
    CustomStack<ControllerAction> undoStack;

    // 5. Global Vehicle Registry & Audit (Singly Linked List)
    CustomLinkedList<VehicleAuditRecord> auditRegistry;

    // Simulation State
    Direction currentGreenDirection;
    int currentGreenTicksRemaining;
    int defaultGreenDuration;
    int currentTick;
    int totalVehiclesProcessed;
    int totalWaitTimeSum;
    bool emergencyModeActive;

public:
    TrafficManager(int defaultGreenTime = 3);
    ~TrafficManager();

    // Core Inflow / Enqueue
    void addVehicleToLane(Direction dir, const Vehicle& vehicle);
    void addVehicleToBottleneck(const Vehicle& vehicle);

    // Regular Signal Cycle & Vehicle Processing
    void advanceSimulationTick();
    void switchSignal(Direction newDirection, const std::string& reason, bool recordUndo = true);

    // Emergency Preemption (LIFO Stack)
    void triggerEmergencyPreemption(Direction emergencyDir, const Vehicle& emergencyVeh);
    void resolveEmergencyPreemption();

    // Bottleneck Management (LIFO Stack)
    void divertBottleneckToSpur();
    void restoreBottleneckFromSpur();
    void passBottleneckEmergencyVehicle(const Vehicle& amb);

    // Undo System (LIFO Stack)
    bool undoLastAction();

    // Inspection & Queries (Linked List Search & Visual Dashboard)
    void displayDashboard() const;
    void searchVehicle(const std::string& query) const;
    void printAuditRegistry() const;
    void printMetrics() const;

    // Helpers
    int getCurrentTick() const { return currentTick; }
    bool isEmergencyActive() const { return emergencyModeActive; }
    int getPreemptionDepth() const { return preemptionStack.size(); }
    int getSpurCount() const { return bottleneckSpurStack.size(); }
    int getUndoCount() const { return undoStack.size(); }

private:
    void logVehicleEntry(const Vehicle& v);
    void logVehicleExit(const std::string& vehicleId, int waitTime);
    void printColored(const std::string& text, const std::string& colorCode) const;
};

#endif // TRAFFIC_MANAGER_H
