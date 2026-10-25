#include "Ship.h"
#include "PortTime.h"
#include <iostream>
#include <cstring>

using namespace std;

const double Ship::MIN_LENGTH = 20.0;
const double Ship::MAX_LENGTH = 400.0;
const double Ship::MIN_DEPTH = 2.0;
const double Ship::MAX_DEPTH = 25.0;
int Ship::nextId = 1;

bool Ship::isValidName(const char* name) {
    if (name == nullptr || name[0] == '\0') {
        return false;
    }
    return strlen(name) <= (size_t)MAX_NAME_LENGTH;
}

bool Ship::isValidLength(double length) {
    return length >= MIN_LENGTH && length <= MAX_LENGTH;
}

bool Ship::isValidDepth(double depth) {
    return depth >= MIN_DEPTH && depth <= MAX_DEPTH;
}

bool Ship::isValidSchedule(int arrivalTime, int departureDeadline) {
    return arrivalTime >= 0 && departureDeadline > arrivalTime;
}

// Placeholder used to fill arrays; it is invalid and does not use up an ID
Ship::Ship()
    : shipId(0), length(0.0), requiredDepth(0.0), arrivalTime(0),
      departureDeadline(0), actualArrivalTime(-1), berthedTime(-1),
      actualDepartureTime(-1), status(0), berthId(-1), valid(false),
      manifest(10), loadingList(10) {
    shipName[0] = '\0';
}

// If any value is illegal the object stays an invalid placeholder
Ship::Ship(const char* name, double length, double depth,
           int arrivalTime, int departureDeadline)
    : Ship() {
    if (!isValidName(name) || !isValidLength(length) || !isValidDepth(depth) ||
        !isValidSchedule(arrivalTime, departureDeadline)) {
        return;
    }
    strcpy(shipName, name);
    this->length = length;
    requiredDepth = depth;
    this->arrivalTime = arrivalTime;
    this->departureDeadline = departureDeadline;
    status = SCHEDULED;
    shipId = nextId++;          // only valid ships use up an ID
    valid = true;
}

bool Ship::isValid() const {
    return valid;
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

// A container cannot be on both the unloading and the loading list
bool Ship::addContainer(int containerId) {
    if (!valid || loadingList.contains(containerId)) {
        return false;
    }
    return manifest.addContainer(containerId);
}

bool Ship::addLoadContainer(int containerId) {
    if (!valid || manifest.contains(containerId)) {
        return false;
    }
    return loadingList.addContainer(containerId);
}

// State changes follow:
// SCHEDULED -> WAITING -> BERTHED -> OPERATING -> READY_TO_DEPART -> DEPARTED
// Each function returns false if the ship is not in the right previous state.
bool Ship::markArrived(int currentTime) {
    if (status != SCHEDULED || currentTime < 0) {
        return false;
    }
    actualArrivalTime = currentTime;
    status = WAITING;
    return true;
}

bool Ship::markBerthed(int berth, int currentTime) {
    if (status != WAITING || berth <= 0 || currentTime < actualArrivalTime) {
        return false;
    }
    berthId = berth;
    berthedTime = currentTime;
    status = BERTHED;
    return true;
}

bool Ship::markOperating() {
    if (status != BERTHED) {
        return false;
    }
    status = OPERATING;
    return true;
}

bool Ship::markReadyToDepart() {
    if (status != OPERATING) {
        return false;
    }
    status = READY_TO_DEPART;
    return true;
}

bool Ship::markDeparted(int currentTime) {
    if (status != READY_TO_DEPART || currentTime < berthedTime) {
        return false;
    }
    actualDepartureTime = currentTime;
    status = DEPARTED;
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
    if (status != DEPARTED) {
        return -1;
    }
    return actualDepartureTime - actualArrivalTime;
}

int Ship::getDelay() const {
    if (status != DEPARTED || actualDepartureTime <= departureDeadline) {
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
    if (!valid) {
        cout << "Ship (invalid placeholder)" << endl;
        return;
    }
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
        case SCHEDULED:       return "SCHEDULED";
        case WAITING:         return "WAITING";
        case BERTHED:         return "BERTHED";
        case OPERATING:       return "OPERATING";
        case READY_TO_DEPART: return "READY_TO_DEPART";
        case DEPARTED:        return "DEPARTED";
        default:              return "UNKNOWN";
    }
}
