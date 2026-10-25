#include "Ship.h"
#include "PortTime.h"
#include <iostream>
#include <cstring>

using namespace std;

int Ship::nextId = 1;

// Placeholder ship used to fill arrays; it does not use up an ID
Ship::Ship() {
    shipId = 0;
    strcpy(shipName, "Unassigned");
    length = 0.0;
    requiredDepth = 0.0;
    arrivalTime = 0;
    departureDeadline = 0;
    actualArrivalTime = -1;
    berthedTime = -1;
    actualDepartureTime = -1;
    status = SHIP_SCHEDULED;
    berthId = -1;
}

Ship::Ship(const char* name, double length, double depth)
    : Ship(name, length, depth, 0, 0) {
}

Ship::Ship(const char* name, double length, double depth,
           int arrival, int deadline) {
    shipId = nextId++;
    if (name == nullptr) {
        name = "Unnamed";
    }
    strncpy(shipName, name, SHIP_NAME_SIZE - 1);
    shipName[SHIP_NAME_SIZE - 1] = '\0';
    this->length = (length > 0) ? length : 0.0;
    requiredDepth = (depth > 0) ? depth : 0.0;
    actualArrivalTime = -1;
    berthedTime = -1;
    actualDepartureTime = -1;
    status = SHIP_SCHEDULED;
    berthId = -1;
    setSchedule(arrival, deadline);
}

int Ship::getId() const {
    return shipId;
}

const char* Ship::getName() const {
    return shipName;
}

double Ship::getLength() const {
    return length;
}

double Ship::getRequiredDepth() const {
    return requiredDepth;
}

int Ship::getArrivalTime() const {
    return arrivalTime;
}

int Ship::getDeadline() const {
    return departureDeadline;
}

int Ship::getActualArrivalTime() const {
    return actualArrivalTime;
}

int Ship::getBerthedTime() const {
    return berthedTime;
}

int Ship::getActualDepartureTime() const {
    return actualDepartureTime;
}

int Ship::getStatus() const {
    return status;
}

int Ship::getBerthId() const {
    return berthId;
}

const Manifest& Ship::getManifest() const {
    return manifest;
}

const Manifest& Ship::getLoadingList() const {
    return loadingList;
}

// A deadline earlier than the arrival is moved to the arrival time
void Ship::setSchedule(int arrival, int deadline) {
    arrivalTime = (arrival >= 0) ? arrival : 0;
    departureDeadline = (deadline >= arrivalTime) ? deadline : arrivalTime;
}

bool Ship::addContainer(int containerId) {
    if (loadingList.contains(containerId)) {
        cout << "Error: container " << containerId << " is already on "
             << shipName << "'s loading list" << endl;
        return false;
    }
    return manifest.addContainer(containerId);
}

bool Ship::addLoadContainer(int containerId) {
    if (manifest.contains(containerId)) {
        cout << "Error: container " << containerId << " is already on "
             << shipName << "'s unloading manifest" << endl;
        return false;
    }
    return loadingList.addContainer(containerId);
}

// State changes follow:
// SCHEDULED -> WAITING -> BERTHED -> OPERATING -> READY_TO_DEPART -> DEPARTED
bool Ship::markArrived(int currentTime) {
    if (status != SHIP_SCHEDULED) {
        cout << "Error: " << shipName << " cannot arrive while "
             << statusToString(status) << endl;
        return false;
    }
    actualArrivalTime = currentTime;
    status = SHIP_WAITING;
    return true;
}

bool Ship::markBerthed(int berth, int currentTime) {
    if (status != SHIP_WAITING) {
        cout << "Error: " << shipName << " cannot berth while "
             << statusToString(status) << endl;
        return false;
    }
    berthId = berth;
    berthedTime = currentTime;
    status = SHIP_BERTHED;
    return true;
}

bool Ship::markOperating() {
    if (status != SHIP_BERTHED) {
        cout << "Error: " << shipName << " cannot start operations while "
             << statusToString(status) << endl;
        return false;
    }
    status = SHIP_OPERATING;
    return true;
}

bool Ship::markReadyToDepart() {
    if (status != SHIP_OPERATING) {
        cout << "Error: " << shipName << " cannot be ready to depart while "
             << statusToString(status) << endl;
        return false;
    }
    status = SHIP_READY_TO_DEPART;
    return true;
}

bool Ship::markDeparted(int currentTime) {
    if (status != SHIP_READY_TO_DEPART) {
        cout << "Error: " << shipName << " cannot depart while "
             << statusToString(status) << endl;
        return false;
    }
    actualDepartureTime = currentTime;
    status = SHIP_DEPARTED;
    berthId = -1;
    return true;
}

// Time spent at anchor before getting a berth (-1 if not berthed yet)
int Ship::getWaitingTime() const {
    if (actualArrivalTime < 0 || berthedTime < 0) {
        return -1;
    }
    return berthedTime - actualArrivalTime;
}

// Arrival to departure (-1 if the ship has not departed)
int Ship::getTurnaroundTime() const {
    if (status != SHIP_DEPARTED) {
        return -1;
    }
    return actualDepartureTime - actualArrivalTime;
}

int Ship::getDelay() const {
    if (status != SHIP_DEPARTED || actualDepartureTime <= departureDeadline) {
        return 0;
    }
    return actualDepartureTime - departureDeadline;
}

bool Ship::isLate() const {
    return getDelay() > 0;
}

bool Ship::operator==(const Ship& other) const {
    return shipId == other.shipId;
}

void Ship::display() const {
    cout << "Ship #" << shipId << " " << shipName << endl;
    cout << "  Size     : " << length << " m long, " << requiredDepth
         << " m draft" << endl;
    cout << "  Schedule : arrives " << PortTime(arrivalTime)
         << ", deadline " << PortTime(departureDeadline) << endl;
    cout << "  Status   : " << statusToString(status);
    if (berthId != -1) {
        cout << " at berth " << berthId;
    }
    cout << endl;
    cout << "  Unload   : ";
    manifest.display();
    cout << "  Load     : ";
    loadingList.display();
}

const char* Ship::statusToString(int status) {
    switch (status) {
        case SHIP_SCHEDULED:       return "SCHEDULED";
        case SHIP_WAITING:         return "WAITING";
        case SHIP_BERTHED:         return "BERTHED";
        case SHIP_OPERATING:       return "OPERATING";
        case SHIP_READY_TO_DEPART: return "READY_TO_DEPART";
        case SHIP_DEPARTED:        return "DEPARTED";
        default:                   return "UNKNOWN";
    }
}
