#include "Berth.h"
#include <iostream>

using namespace std;

const double Berth::MIN_LENGTH = 20.0;
const double Berth::MAX_LENGTH = 500.0;
const double Berth::MIN_DEPTH = 2.0;
const double Berth::MAX_DEPTH = 30.0;
int Berth::nextId = 1;

bool Berth::isValidLength(double length) {
    return length >= MIN_LENGTH && length <= MAX_LENGTH;
}

bool Berth::isValidDepth(double depth) {
    return depth >= MIN_DEPTH && depth <= MAX_DEPTH;
}

// Placeholder used to fill arrays; it is invalid and does not use up an ID
Berth::Berth()
    : berthId(0), maximumLength(0.0), availableDepth(0.0),
      occupiedShipId(0), isOccupied(false), valid(false) {
}

// If either value is illegal the object stays an invalid placeholder
Berth::Berth(double maxLength, double depth)
    : Berth() {
    if (!isValidLength(maxLength) || !isValidDepth(depth)) {
        return;
    }
    maximumLength = maxLength;
    availableDepth = depth;
    berthId = nextId++;
    valid = true;
}

bool Berth::isValid() const {
    return valid;
}

int Berth::getId() const {
    return berthId;
}

double Berth::getMaximumLength() const {
    return maximumLength;
}

double Berth::getAvailableDepth() const {
    return availableDepth;
}

int Berth::getOccupiedShipId() const {
    return occupiedShipId;
}

// Checks only length and depth, ignoring whether the berth is free
bool Berth::fitsDimensions(const Ship& ship) const {
    return valid && ship.getLength() <= maximumLength &&
           ship.getRequiredDepth() <= availableDepth;
}

bool Berth::canAccommodate(const Ship& ship) const {
    return isAvailable() && fitsDimensions(ship);
}

// Fails if the berth is invalid, already occupied, or the ID is not real
bool Berth::assignShip(int shipId) {
    if (!isAvailable() || shipId <= 0) {
        return false;
    }
    occupiedShipId = shipId;
    isOccupied = true;
    return true;
}

void Berth::releaseShip() {
    occupiedShipId = 0;
    isOccupied = false;
}

bool Berth::isAvailable() const {
    return valid && !isOccupied;
}

void Berth::display() const {
    if (!valid) {
        cout << "Berth (invalid placeholder)" << endl;
        return;
    }
    cout << "Berth " << berthId << " (max " << maximumLength << " m, depth "
         << availableDepth << " m): ";
    if (isOccupied) {
        cout << "occupied by ship " << occupiedShipId;
    } else {
        cout << "free";
    }
    cout << endl;
}
