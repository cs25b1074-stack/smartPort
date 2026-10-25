#include "Berth.h"
#include <iostream>

using namespace std;

Berth::Berth() {
    berthId = 0;
    maximumLength = 0.0;
    availableDepth = 0.0;
    occupiedShipId = -1;
    isOccupied = false;
}

Berth::Berth(int id, double maxLength, double depth) {
    berthId = id;
    maximumLength = (maxLength > 0) ? maxLength : 0.0;
    availableDepth = (depth > 0) ? depth : 0.0;
    occupiedShipId = -1;
    isOccupied = false;
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
    return ship.getLength() <= maximumLength &&
           ship.getRequiredDepth() <= availableDepth;
}

bool Berth::canAccommodate(const Ship& ship) const {
    return !isOccupied && fitsDimensions(ship);
}

bool Berth::assignShip(const Ship& ship) {
    if (isOccupied) {
        cout << "Error: berth " << berthId << " is already occupied by ship "
             << occupiedShipId << endl;
        return false;
    }
    if (!fitsDimensions(ship)) {
        cout << "Error: berth " << berthId << " cannot fit " << ship.getName()
             << " (" << ship.getLength() << " m / " << ship.getRequiredDepth()
             << " m draft)" << endl;
        return false;
    }
    occupiedShipId = ship.getId();
    isOccupied = true;
    return true;
}

void Berth::releaseShip() {
    occupiedShipId = -1;
    isOccupied = false;
}

bool Berth::isAvailable() const {
    return !isOccupied;
}

void Berth::display() const {
    cout << "Berth " << berthId << " (max " << maximumLength << " m, depth "
         << availableDepth << " m): ";
    if (isOccupied) {
        cout << "occupied by ship " << occupiedShipId;
    } else {
        cout << "free";
    }
    cout << endl;
}
