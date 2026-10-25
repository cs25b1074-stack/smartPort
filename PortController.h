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
public:
    static const int MIN_TRUCK_INTERVAL = 1;
    static const int MAX_TRUCK_INTERVAL = 120;
    static const int DEFAULT_TRUCK_INTERVAL = 10;
    static const int URGENCY_WINDOW = 60;       // minutes before a deadline
    static const int ERROR_SIZE = 120;

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

    PortTime startTime;         // when the first run began
    bool started;
    Manifest importQueue;       // unloaded containers waiting for a truck
    int truckInterval;          // minutes between road dispatches
    bool yardBlockReported;     // avoids repeating the "yard full" message
    char lastError[ERROR_SIZE]; // reason the last setup call failed

    void growShips();
    void growBerths();
    void growCranes();
    void allocateArrays();
    void setError(const char* message);

    int findShipIndex(int shipId) const;
    Crane* findCrane(int craneId);
    Berth* findBerth(int berthId);
    int findBestBerth(const Ship& ship) const;
    bool anyBerthFits(const Ship& ship) const;
    int findBestCrane(const Container& container) const;
    bool canStillTakeCargo(const Ship& ship) const;

    void logTime() const;
    void startJob(HandlingJob& job, Crane& crane, Container& container);
    void finishJob(HandlingJob& job, Crane& crane);
    void departShip(Ship& ship);

    void processCraneEvents();
    void progressJobs();
    void processDepartures();
    void dispatchTrucks();
    void recordUtilization();

    // Copying a whole port is not needed, so copying is disabled by
    // declaring these private and never defining them
    PortController(const PortController&);
    PortController& operator=(const PortController&);

public:
    PortController();
    ~PortController();

    // Setup: every function returns false and changes nothing for invalid input
    bool registerShip(const Ship& ship);
    bool registerBerth(const Berth& berth);
    bool registerCrane(const Crane& crane);
    bool setupYard(int standard, int reefer, int hazard, double maxWeight);
    bool setTruckInterval(int minutes);
    bool addContainerToShip(int shipId, const Container& container);
    bool addExportContainer(int shipId, const Container& container);
    bool scheduleCraneBreakdown(int craneId, int time, int repairMinutes);
    void clearScenario();

    // Read-only information for the menus
    const char* getLastError() const;
    int getShipCount() const;
    int getBerthCount() const;
    int getCraneCount() const;
    bool isYardConfigured() const;
    bool isReadyToRun() const;
    bool hasStarted() const;
    int getShipIdAt(int index) const;
    int getEarliestArrival() const;
    int getTruckInterval() const;
    const PortTime& getCurrentTime() const;
    void displayShipList() const;

    // Simulation
    void processArrivals();
    void allocateBerths();
    void createHandlingJobs();
    void assignCraneJobs();
    void processNextEvent();
    void updateSimulation(int duration);
    void runSimulation(int endTimeMinutes);
    bool isFinished() const;
    bool breakDownCrane(int craneId, int repairMinutes);
    bool requestDeparture(int shipId);

    void displayPortStatus() const;
    void generateFinalReport() const;
};

#endif
