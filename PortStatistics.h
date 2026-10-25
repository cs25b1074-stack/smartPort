#ifndef PORTSTATISTICS_H
#define PORTSTATISTICS_H

class PortStatistics {
private:
    int shipsCompleted;
    int delayedShips;
    int containersHandled;
    int containersUnloaded;
    int containersLoaded;
    int containersDispatched;
    int totalShipTurnaroundMinutes;
    int totalShipWaitingMinutes;
    int totalDelayMinutes;
    int totalCraneBusyMinutes;
    int totalCraneAvailableMinutes;     // crane-minutes while in service
    int totalBerthOccupiedMinutes;
    int totalBerthAvailableMinutes;
    int simulatedMinutes;
    int craneBreakdowns;
    int jobsInterrupted;

public:
    PortStatistics();

    void recordShipDeparture(int turnaround, int waiting, int delay);
    void recordContainerHandled(int jobType);
    void recordDispatch();
    void recordMinute(int busyCranes, int inServiceCranes,
                      int occupiedBerths, int totalBerths);
    void recordBreakdown(bool interruptedJob);

    int getShipsCompleted() const;
    int getDelayedShips() const;
    int getContainersHandled() const;
    int getSimulatedMinutes() const;

    double getAverageTurnaroundTime() const;
    double getAverageWaitingTime() const;
    double getDelayRate() const;
    double getCraneUtilization() const;
    double getBerthUtilization() const;

    void generateReport() const;
};

#endif
