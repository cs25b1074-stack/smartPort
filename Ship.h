#ifndef SHIP_H
#define SHIP_H

#include "Manifest.h"

// Ship states
const int SHIP_SCHEDULED = 0;
const int SHIP_WAITING = 1;
const int SHIP_BERTHED = 2;
const int SHIP_OPERATING = 3;
const int SHIP_READY_TO_DEPART = 4;
const int SHIP_DEPARTED = 5;

const int SHIP_NAME_SIZE = 50;

class Ship {
private:
    static int nextId;          // used to generate unique ship IDs

    int shipId;
    char shipName[SHIP_NAME_SIZE];
    double length;              // in metres
    double requiredDepth;       // draft in metres
    int arrivalTime;            // scheduled, minutes since 00:00
    int departureDeadline;
    int actualArrivalTime;
    int berthedTime;
    int actualDepartureTime;
    int status;
    int berthId;

    Manifest manifest;          // containers to unload at this port
    Manifest loadingList;       // containers to load from the yard

public:
    Ship();
    Ship(const char* name, double length, double depth);
    Ship(const char* name, double length, double depth,
         int arrival, int deadline);

    // The compiler-generated copy constructor and assignment operator are
    // safe here because Manifest performs its own deep copy.

    int getId() const;
    const char* getName() const;
    double getLength() const;
    double getRequiredDepth() const;
    int getArrivalTime() const;
    int getDeadline() const;
    int getActualArrivalTime() const;
    int getBerthedTime() const;
    int getActualDepartureTime() const;
    int getStatus() const;
    int getBerthId() const;
    const Manifest& getManifest() const;
    const Manifest& getLoadingList() const;

    void setSchedule(int arrival, int deadline);
    bool addContainer(int containerId);
    bool addLoadContainer(int containerId);

    bool markArrived(int currentTime);
    bool markBerthed(int berth, int currentTime);
    bool markOperating();
    bool markReadyToDepart();
    bool markDeparted(int currentTime);

    int getWaitingTime() const;
    int getTurnaroundTime() const;
    int getDelay() const;
    bool isLate() const;

    bool operator==(const Ship& other) const;   // same ship ID
    void display() const;

    static const char* statusToString(int status);
};

#endif
