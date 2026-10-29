# SmartFlow: Urban Bottleneck & Emergency Corridor Traffic Management System
**Data Structures and Algorithms (DSA) Course Project**  
*Implemented in C++ from Scratch (No STL Containers)*

---

## 1. Project Overview & Problem Statement

### 1.1 Real-World Context
In urban traffic networks, standard intersections are routinely regulated using **First-In, First-Out (FIFO)** lanes. However, conventional traffic models break down in two critical scenarios:
1. **Single-Lane Bottlenecks (Bridges, Tunnels, Construction Overpasses):**  
   When two opposing vehicle flows or an unexpected blockage occurs in a narrow single-lane corridor, vehicles cannot simply move forward. To clear the corridor for high-priority emergency responders (Ambulances, Fire Engines), vehicles must pull into a single-entry dead-end **turnout spur pocket**. Because the pocket has only one entry/exit point, vehicles must enter and exit in **Last-In, First-Out (LIFO)** order.
2. **Multi-Level Emergency Signal Preemption (Interrupt Handling):**  
   When an emergency responder arrives at a busy intersection, the normal light rotation cannot simply restart from scratch after the emergency departs. Doing so causes starve-outs and cross-traffic collisions. The interrupted light cycle and its remaining timers must be saved onto an **Emergency Preemption Stack**. If multiple overlapping emergencies occur (e.g., an Ambulance followed by a Fire Brigade on different approaches), the signal states must be popped in reverse order (LIFO) to safely restore normal flow.

### 1.2 Core Problem Statement
> Design and implement a robust, memory-safe, and interactive **Urban Traffic Management System** utilizing **Linear Data Structures** with a primary emphasis on **Stack (LIFO)** and **Queue (FIFO)** dynamics built entirely from scratch without using standard library containers (e.g., `std::stack`, `std::queue`, `std::vector`). The system must handle regular round-robin intersection cycles, nested emergency signal preemptions, dead-end spur bottleneck evacuations, operator action undos, and an audit registry with real-time analytics.

---

## 2. Mapping of Linear Data Structures

| Data Structure | Internal Implementation | Concrete Traffic Engineering Role | Time Complexity | Space Complexity |
| :--- | :--- | :--- | :--- | :--- |
| **`CustomQueue<Vehicle>`** | Dynamic Linked List with `front` & `rear` pointers | **4-Way Intersection Lanes (FIFO)**: Vehicles arrive at the back of the queue and depart from the front when green. | Enqueue: $\mathcal{O}(1)$<br>Dequeue: $\mathcal{O}(1)$<br>Peek: $\mathcal{O}(1)$ | $\mathcal{O}(N)$ |
| **`CustomStack<SignalState>`** | Dynamic Linked List (`top` pointer) | **Emergency Preemption Stack (LIFO)**: Saves interrupted signal state (lane index, remaining green time, trigger reason) upon emergency arrival; pops to restore exact cycle when cleared. | Push: $\mathcal{O}(1)$<br>Pop: $\mathcal{O}(1)$<br>Peek: $\mathcal{O}(1)$ | $\mathcal{O}(S)$ |
| **`CustomStack<Vehicle>`** | Dynamic Linked List (`top` pointer) | **Bottleneck Evacuation Spur Pocket (LIFO)**: In a single-lane tunnel or bridge, vehicles pull into a dead-end turnout pocket to clear the road. The last car pulled in is closest to the opening and must reverse/exit first. | Push: $\mathcal{O}(1)$<br>Pop: $\mathcal{O}(1)$<br>Peek: $\mathcal{O}(1)$ | $\mathcal{O}(K)$ |
| **`CustomStack<ControllerAction>`**| Dynamic Linked List (`top` pointer) | **Traffic Operator Action Undo Stack (LIFO)**: Enables 1-click rollback of dispatch decisions, signal overrides, or manual route changes. | Push: $\mathcal{O}(1)$<br>Pop: $\mathcal{O}(1)$<br>Peek: $\mathcal{O}(1)$ | $\mathcal{O}(U)$ |
| **`CustomLinkedList<VehicleAuditRecord>`** | Singly Linked List with `head` pointer | **Vehicle Audit Registry & Metrics**: Stores vehicle history, computes wait times, tracks departures, and supports plate/ID lookups. | Append: $\mathcal{O}(N)$<br>Search: $\mathcal{O}(N)$<br>Delete: $\mathcal{O}(N)$ | $\mathcal{O}(M)$ |

---

## 3. System Architecture & Project Structure

```
dsacp/
├── include/
│   ├── DataStructures.h   # Custom Stack, Queue, and Linked List templates
│   ├── Vehicle.h          # Vehicle struct, types, priority levels, badges
│   └── TrafficManager.h   # Signal controller, bottleneck manager, undo manager
├── src/
│   ├── TrafficManager.cpp # Core logic for preemption, spur stack, tick simulation
│   └── main.cpp           # Interactive CLI menu, ASCII dashboard, automated demo
├── smartflow.exe          # Compiled binary executable
└── README.md              # Project report and documentation
```

### 3.1 Memory Management & Safety
- All custom structures adhere to the C++ **Rule of Three**:
  - Custom Destructors traverse and deallocate all nodes (`delete temp`).
  - Custom Copy Constructors perform deep copies.
  - Custom Copy Assignment Operators prevent memory leaks and dangling pointer aliasing.

---

## 4. Key Algorithms

### 4.1 Emergency Preemption & Stack Restoration (LIFO)
```text
ALGORITHM TriggerEmergencyPreemption(laneDirection, emergencyVehicle):
    1. currentState = SignalState(currentGreenDirection, remainingTicks, "Interrupted")
    2. preemptionStack.push(currentState)
    3. laneQueues[laneDirection].enqueue(emergencyVehicle)
    4. currentGreenDirection = laneDirection
    5. currentGreenTicksRemaining = 6
    6. emergencyModeActive = TRUE

ALGORITHM ResolveEmergencyPreemption():
    1. IF preemptionStack.isEmpty():
           emergencyModeActive = FALSE
           RETURN
    2. savedState = preemptionStack.pop()
    3. currentGreenDirection = savedState.activeDirection
    4. currentGreenTicksRemaining = savedState.remainingGreenTicks
    5. IF preemptionStack.isEmpty():
           emergencyModeActive = FALSE
```

### 4.2 Dead-End Spur Pocket Evacuation (LIFO)
```text
ALGORITHM ClearBottleneckCorridorToSpur():
    WHILE NOT bottleneckLane.isEmpty():
        vehicle = bottleneckLane.dequeue()
        bottleneckSpurStack.push(vehicle) // Last car in remains at the top
    Corridor is now clear for emergency vehicle transit!

ALGORITHM RestoreFromSpur():
    WHILE NOT bottleneckSpurStack.isEmpty():
        vehicle = bottleneckSpurStack.pop() // Top vehicle exits first (LIFO)
        bottleneckLane.enqueue(vehicle)
```

---

## 5. How to Compile and Run

### 5.1 Prerequisites
- A C++11 (or higher) compliant compiler, e.g. **MinGW GCC (`g++`)** or **MSVC**.

### 5.2 Build Command
From the project root (`e:\SPIDER MAN\dsacp`):
```powershell
g++ -std=c++11 -Wall -I./include src/TrafficManager.cpp src/main.cpp -o smartflow.exe
```

### 5.3 Execution Command
```powershell
.\smartflow.exe
```

---

## 6. Interactive Features & Menu

When running `smartflow.exe`, the interactive controller provides the following options:

1. **Display Real-time Visual Traffic Dashboard**: Displays the 4 intersection lanes, signal lights, spur pocket, stack depths, and throughput.
2. **Add Vehicle to Intersection Lane (FIFO Queue Enqueue)**: Manually enqueue a car, bus, truck, or motorbike into North, East, South, or West lanes.
3. **Advance Simulation by 1 Tick (FIFO Queue Dequeue)**: Advances the simulation clock, dispatches vehicles on green light, updates wait timers, and rotates signals.
4. **Trigger Emergency Vehicle Arrival (LIFO Preemption Stack Push)**: Dispatches an Ambulance, Fire Truck, or Police Cruizer, interrupting current traffic.
5. **Resolve Emergency Preemption (LIFO Preemption Stack Pop)**: Restores interrupted light states from the stack.
6. **Bottleneck Evacuation Spur Demo (LIFO Tunnel Clearance)**: Sub-menu allowing cars to enter a single-lane tunnel, divert into the dead-end spur pocket, let an emergency responder cross, and restore cars in LIFO order.
7. **Undo Last Controller Action (LIFO Undo Stack Pop)**: Roll back erroneous vehicle entries or manual signal overrides.
8. **Search Vehicle by ID or Plate (Linked List Search)**: O(N) lookup across all active or past vehicles.
9. **View Vehicle Audit Registry & Performance Metrics**: Summary of throughput, average delay, and active data structure occupancies.
10. **Run Automated Demonstration Scenario**: 1-click comprehensive walkthrough tailored for academic viva/evaluations.

---

## 7. Sample Evaluation & Viva Q&A

**Q1: Why is a Stack chosen for Emergency Preemption instead of simply restarting the signal cycle?**  
> **Answer:** If an intersection has 30 seconds of green for East-bound traffic and an emergency vehicle interrupts at second 5, wiping out the cycle causes the remaining East-bound vehicles to wait through an entire additional rotation cycle (causing unfair starvation and gridlock). By pushing the `SignalState` onto a LIFO stack, the interrupted state (including the remaining 25 seconds) is cleanly popped and resumed. Furthermore, if *nested* emergencies occur (e.g. an Ambulance followed by a Fire Engine), a Stack naturally unwinds nested interrupts in exact reverse order.

**Q2: What is the physical justification for a Stack in the Bottleneck Corridor?**  
> **Answer:** In single-lane tunnels, mountain passes, or bridges, vehicles drive in one behind another. A roadside emergency turnout pocket is a single-entry dead-end spur (like a cul-de-sac or train siding). When vehicles back or drive into this dead-end pocket, the first vehicle to enter reaches the back (bottom of stack), and the last vehicle stays at the mouth (top of stack). When leaving the pocket to resume travel, the last vehicle must pull out first (LIFO order).

**Q3: How are memory leaks prevented when using custom templates?**  
> **Answer:** Every data structure (`CustomStack`, `CustomQueue`, `CustomLinkedList`) implements a destructor that iterates through the dynamic nodes and calls `delete` on each node. Deep copy constructors and assignment operators ensure that pass-by-value operations allocate independent node chains.
