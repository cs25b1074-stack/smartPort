#ifndef BERTH_H
#define BERTH_H

#include "Ship.h"

class Berth {
private:
    int berthId;
    double maximumLength;       // in metres
    double availableDepth;      // water depth in metres
    int occupiedShipId;
    bool isOccupied;

public:
    Berth();
    Berth(int id, double maxLength, double depth);

    int getId() const;
    double getMaximumLength() const;
    double getAvailableDepth() const;
    int getOccupiedShipId() const;

    bool fitsDimensions(const Ship& ship) const;
    bool canAccommodate(const Ship& ship) const;
    bool assignShip(const Ship& ship);
    void releaseShip();
    bool isAvailable() const;
    void display() const;
};

#endif
