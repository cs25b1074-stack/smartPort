#ifndef SHIP_H
#define SHIP_H

#include "Manifest.h"

class Ship {
public:
    // Ship states
    static const int SCHEDULED = 1;
    static const int WAITING = 2;
    static const int BERTHED = 3;
    static const int OPERATING = 4;
    static const int READY_TO_DEPART = 5;
    static const int DEPARTED = 6;

    // Validation limits
    static const int MAX_NAME_LENGTH = 49;      // leaves room for '\0'
    static const double MIN_LENGTH;             // 20 m
    static const double MAX_LENGTH;             // 400 m
    static const double MIN_DEPTH;              // 2 m
    static const double MAX_DEPTH;              // 25 m

private:
    static int nextId;          // used to generate unique ship IDs

    int shipId;
    char shipName[MAX_NAME_LENGTH + 1];
    double length;              // in metres
    double requiredDepth;       // draft in metres
    int arrivalTime;            // scheduled, minutes since 00:00
    int departureDeadline;
    int actualArrivalTime;
    int berthedTime;
    int actualDepartureTime;
    int status;
    int berthId;
    bool valid;

    Manifest manifest;          // containers to unload at this port
    Manifest loadingList;       // containers to load from the yard

public:
    Ship();
    Ship(const char* name, double length, double depth,
         int arrivalTime, int departureDeadline);

    // The compiler-generated copy constructor and assignment operator are
    // safe here because Manifest performs its own deep copy. A copy keeps
    // the same ID and valid flag, because it represents the same ship.

    static bool isValidName(const char* name);
    static bool isValidLength(double length);
    static bool isValidDepth(double depth);
    static bool isValidSchedule(int arrivalTime, int departureDeadline);

    bool isValid() const;
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
