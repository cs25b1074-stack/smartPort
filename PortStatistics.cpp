#include "PortStatistics.h"
#include "HandlingJob.h"
#include <iostream>
#include <iomanip>

using namespace std;

PortStatistics::PortStatistics()
    : shipsCompleted(0), delayedShips(0), containersHandled(0),
      containersUnloaded(0), containersLoaded(0), containersDispatched(0),
      totalShipTurnaroundMinutes(0), totalShipWaitingMinutes(0),
      totalDelayMinutes(0), totalCraneBusyMinutes(0),
      totalCraneAvailableMinutes(0), totalBerthOccupiedMinutes(0),
      totalBerthAvailableMinutes(0), craneBreakdowns(0), jobsInterrupted(0) {
}

// Negative values are ignored so a bad call cannot corrupt the totals
void PortStatistics::recordShipDeparture(int turnaroundMinutes, bool wasLate) {
    if (turnaroundMinutes < 0) {
        return;
    }
    shipsCompleted++;
    totalShipTurnaroundMinutes += turnaroundMinutes;
    if (wasLate) {
        delayedShips++;
    }
}

void PortStatistics::recordWaitingTime(int minutes) {
    if (minutes > 0) {
        totalShipWaitingMinutes += minutes;
    }
}

void PortStatistics::recordDelay(int minutes) {
    if (minutes > 0) {
        totalDelayMinutes += minutes;
    }
}

void PortStatistics::recordContainerHandled() {
    containersHandled++;
}

// Overload that also keeps the unloaded/loaded split
void PortStatistics::recordContainerHandled(int jobType) {
    recordContainerHandled();
    if (jobType == HandlingJob::UNLOAD) {
        containersUnloaded++;
    } else if (jobType == HandlingJob::LOAD) {
        containersLoaded++;
    }
}

void PortStatistics::recordDispatch() {
    containersDispatched++;
}

void PortStatistics::recordCraneTime(int busyMinutes, int availableMinutes) {
    if (busyMinutes < 0 || availableMinutes < 0) {
        return;
    }
    totalCraneBusyMinutes += busyMinutes;
    totalCraneAvailableMinutes += availableMinutes;
}

void PortStatistics::recordBerthTime(int occupiedMinutes, int availableMinutes) {
    if (occupiedMinutes < 0 || availableMinutes < 0) {
        return;
    }
    totalBerthOccupiedMinutes += occupiedMinutes;
    totalBerthAvailableMinutes += availableMinutes;
}

void PortStatistics::recordBreakdown(bool interruptedJob) {
    craneBreakdowns++;
    if (interruptedJob) {
        jobsInterrupted++;
    }
}

int PortStatistics::getShipsCompleted() const {
    return shipsCompleted;
}

int PortStatistics::getDelayedShips() const {
    return delayedShips;
}

int PortStatistics::getContainersHandled() const {
    return containersHandled;
}

// Only ships that have departed are counted
double PortStatistics::getAverageTurnaroundTime() const {
    if (shipsCompleted == 0) {
        return 0.0;
    }
    return (double)totalShipTurnaroundMinutes / shipsCompleted;
}

double PortStatistics::getAverageWaitingTime() const {
    if (shipsCompleted == 0) {
        return 0.0;
    }
    return (double)totalShipWaitingMinutes / shipsCompleted;
}

double PortStatistics::getDelayRate() const {
    if (shipsCompleted == 0) {
        return 0.0;
    }
    return 100.0 * delayedShips / shipsCompleted;
}

double PortStatistics::getCraneUtilization() const {
    if (totalCraneAvailableMinutes == 0) {
        return 0.0;
    }
    return 100.0 * totalCraneBusyMinutes / totalCraneAvailableMinutes;
}

double PortStatistics::getBerthUtilization() const {
    if (totalBerthAvailableMinutes == 0) {
        return 0.0;
    }
    return 100.0 * totalBerthOccupiedMinutes / totalBerthAvailableMinutes;
}

void PortStatistics::generateReport() const {
    cout << fixed << setprecision(1);
    cout << "Performance statistics" << endl;
    cout << "  Ships completed         : " << shipsCompleted << endl;
    cout << "  Ships departed late     : " << delayedShips
         << " (" << getDelayRate() << "%)";
    if (delayedShips > 0) {
        cout << ", " << totalDelayMinutes << " min total delay";
    }
    cout << endl;
    cout << "  Average waiting time    : " << getAverageWaitingTime() << " min" << endl;
    cout << "  Average turnaround time : " << getAverageTurnaroundTime() << " min" << endl;
    cout << "  Containers handled      : " << containersHandled
         << " (" << containersUnloaded << " unloaded, "
         << containersLoaded << " loaded)" << endl;
    cout << "  Dispatched by road      : " << containersDispatched << endl;
    cout << "  Crane breakdowns        : " << craneBreakdowns
         << " (" << jobsInterrupted << " jobs interrupted)" << endl;
    cout << "  Crane utilization       : " << getCraneUtilization() << "%" << endl;
    cout << "  Berth utilization       : " << getBerthUtilization() << "%" << endl;
    cout.unsetf(ios::fixed);
    cout << setprecision(6);
}
