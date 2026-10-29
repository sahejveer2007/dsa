#ifndef VEHICLE_H
#define VEHICLE_H

#include <string>
#include <iostream>

enum VehicleType {
    CAR,
    BUS,
    TRUCK,
    MOTORBIKE,
    AMBULANCE,
    FIRE_TRUCK,
    POLICE
};

enum PriorityLevel {
    NORMAL_PRIORITY = 1,
    MEDIUM_PRIORITY = 2,
    HIGH_PRIORITY = 3,
    EMERGENCY_PRIORITY = 4
};

enum Direction {
    NORTH = 0,
    EAST = 1,
    SOUTH = 2,
    WEST = 3
};

struct Vehicle {
    std::string id;
    std::string plateNumber;
    VehicleType type;
    PriorityLevel priority;
    Direction destination;
    int entryTime; // Simulation tick when vehicle entered
    int waitTime;  // Accumulated wait time

    Vehicle() 
        : id(""), plateNumber(""), type(CAR), priority(NORMAL_PRIORITY), destination(NORTH), entryTime(0), waitTime(0) {}

    Vehicle(std::string vId, std::string plate, VehicleType vType, Direction dest, int tick = 0)
        : id(vId), plateNumber(plate), type(vType), destination(dest), entryTime(tick), waitTime(0) {
        
        // Assign priority based on vehicle type
        switch (type) {
            case AMBULANCE:
            case FIRE_TRUCK:
                priority = EMERGENCY_PRIORITY;
                break;
            case POLICE:
                priority = HIGH_PRIORITY;
                break;
            case BUS:
                priority = MEDIUM_PRIORITY;
                break;
            default:
                priority = NORMAL_PRIORITY;
                break;
        }
    }

    bool isEmergency() const {
        return priority == EMERGENCY_PRIORITY || priority == HIGH_PRIORITY;
    }

    std::string getTypeString() const {
        switch (type) {
            case CAR: return "Car";
            case BUS: return "Bus";
            case TRUCK: return "Truck";
            case MOTORBIKE: return "Motorbike";
            case AMBULANCE: return "Ambulance";
            case FIRE_TRUCK: return "Fire Truck";
            case POLICE: return "Police Cruizer";
            default: return "Unknown";
        }
    }

    std::string getPriorityString() const {
        switch (priority) {
            case EMERGENCY_PRIORITY: return "EMERGENCY (CRITICAL)";
            case HIGH_PRIORITY: return "HIGH (VIP/POLICE)";
            case MEDIUM_PRIORITY: return "MEDIUM (TRANSIT)";
            case NORMAL_PRIORITY: return "NORMAL";
            default: return "UNKNOWN";
        }
    }

    std::string getBadge() const {
        switch (type) {
            case AMBULANCE: return "[AMB]";
            case FIRE_TRUCK: return "[FIRE]";
            case POLICE: return "[COP]";
            case BUS: return "[BUS]";
            case TRUCK: return "[TRK]";
            case MOTORBIKE: return "[MOTO]";
            case CAR: return "[CAR]";
            default: return "[VEH]";
        }
    }

    static std::string getDirectionName(Direction d) {
        switch (d) {
            case NORTH: return "NORTH";
            case EAST: return "EAST";
            case SOUTH: return "SOUTH";
            case WEST: return "WEST";
            default: return "UNKNOWN";
        }
    }
};

#endif // VEHICLE_H
