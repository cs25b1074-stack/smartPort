#ifndef PORTCONTROLLER_H
#define PORTCONTROLLER_H

#include "Ship.h"
#include "Berth.h"
#include "Crane.h"
#include "ContainerRegistry.h"
#include "ContainerYard.h"
#include "PortScheduler.h"
#include "PortStatistics.h"
#include "PortTime.h"
#include "Manifest.h"

class PortController {
private:
    Ship* ships;
    int shipCount;
    int shipCapacity;

    Berth* berths;
    int berthCount;
    int berthCapacity;

    Crane* cranes;
    int craneCount;
    int craneCapacity;

    ContainerRegistry registry;
    ContainerYard yard;
    PortScheduler scheduler;
    PortStatistics statistics;
    PortTime currentTime;

    Manifest importQueue;       // unloaded containers waiting for a truck
    int truckInterval;          // minutes between road dispatches
    int urgencyWindow;          // minutes before a deadline that jobs become urgent
    bool yardBlockReported;     // avoids repeating the "yard full" message

    Ship* findShip(int shipId);
    const Ship* findShip(int shipId) const;
    Crane* findCrane(int craneId);
    Berth* findBerth(int berthId);
    int findBestBerth(const Ship& ship) const;
    bool anyBerthFits(const Ship& ship) const;
    bool isOnAnyShip(int containerId) const;
    int findBestCrane(const Container& container) const;

    void growShips();
    void logTime() const;
    void startJob(HandlingJob& job, Crane& crane, Container& container);
    void finishJob(HandlingJob& job, Crane& crane);
    void departShip(Ship& ship);

    void processCraneEvents();
    void progressJobs();
    void processDepartures();
    void dispatchTrucks();
    void recordUtilization();

public:
    PortController(int startMinutes, int yardCapacity,
                   int maxBerths, int maxCranes);
    ~PortController();

    // A port owns unique physical resources, so copying is not allowed
    PortController(const PortController&) = delete;
    PortController& operator=(const PortController&) = delete;

    bool addBerth(double maxLength, double depth);
    bool addCrane(double maxLift, int minutesPerContainer);
    int addYardSlots(int count, int type, double maxWeight);
    void setTruckInterval(int minutes);
    void setUrgencyWindow(int minutes);

    bool registerShip(const Ship& ship);
    bool registerContainer(const Container& container);
    bool assignImportCargo(int shipId, int containerId);
    bool assignExportCargo(int shipId, int containerId);
    bool scheduleCraneBreakdown(int craneId, int time, int repairMinutes);
    bool requestDeparture(int shipId);

    void processArrivals();
    void allocateBerths();
    void createHandlingJobs();
    void assignCraneJobs();
    void processNextEvent();
    void updateSimulation(int duration);
    int runUntilComplete(int maxDuration);
    bool isFinished() const;

    const PortTime& getCurrentTime() const;
    void displayPortStatus() const;
    void generateFinalReport() const;
};

#endif
