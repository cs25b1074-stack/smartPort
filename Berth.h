#ifndef BERTH_H
#define BERTH_H

#include "Ship.h"

class Berth {
public:
    // Validation limits
    static const double MIN_LENGTH;     // 20 m
    static const double MAX_LENGTH;     // 500 m
    static const double MIN_DEPTH;      // 2 m
    static const double MAX_DEPTH;      // 30 m

private:
    static int nextId;          // used to generate unique berth IDs

    int berthId;
    double maximumLength;       // in metres
    double availableDepth;      // water depth in metres
    int occupiedShipId;
    bool isOccupied;
    bool valid;

public:
    Berth();
    Berth(double maxLength, double depth);

    static bool isValidLength(double length);
    static bool isValidDepth(double depth);

    bool isValid() const;
    int getId() const;
    double getMaximumLength() const;
    double getAvailableDepth() const;
    int getOccupiedShipId() const;

    bool fitsDimensions(const Ship& ship) const;
    bool canAccommodate(const Ship& ship) const;
    bool assignShip(int shipId);
    void releaseShip();
    bool isAvailable() const;
    void display() const;
};

#endif
