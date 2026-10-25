#include "Crane.h"
#include "PortTime.h"
#include <iostream>

using namespace std;

Crane::Crane() {
    craneId = 0;
    maximumLiftWeight = 0.0;
    currentJobId = -1;
    status = CRANE_AVAILABLE;
    handlingMinutesPerContainer = 1;
    breakdownTime = -1;
    repairMinutes = 0;
    backInServiceTime = -1;
}

Crane::Crane(int id, double maxLift, int minutesPerContainer) {
    craneId = id;
    maximumLiftWeight = (maxLift > 0) ? maxLift : 0.0;
    currentJobId = -1;
    status = CRANE_AVAILABLE;
    handlingMinutesPerContainer = (minutesPerContainer > 0) ? minutesPerContainer : 1;
    breakdownTime = -1;
    repairMinutes = 0;
    backInServiceTime = -1;
}

int Crane::getId() const {
    return craneId;
}

double Crane::getMaximumLiftWeight() const {
    return maximumLiftWeight;
}

int Crane::getCurrentJobId() const {
    return currentJobId;
}

int Crane::getStatus() const {
    return status;
}

int Crane::getHandlingMinutes() const {
    return handlingMinutesPerContainer;
}

int Crane::getBackInServiceTime() const {
    return backInServiceTime;
}

bool Crane::canHandle(const Container& container) const {
    return container.getWeight() <= maximumLiftWeight;
}

bool Crane::isAvailable() const {
    return status == CRANE_AVAILABLE;
}

bool Crane::isInService() const {
    return status != CRANE_OUT_OF_SERVICE;
}

// A crane only ever works on one job at a time
bool Crane::assignJob(int jobId) {
    if (status != CRANE_AVAILABLE) {
        cout << "Error: crane " << craneId << " is " << statusToString(status)
             << " and cannot take job " << jobId << endl;
        return false;
    }
    currentJobId = jobId;
    status = CRANE_BUSY;
    return true;
}

// Returns the finished job's ID, or -1 if the crane was not busy
int Crane::completeJob() {
    if (status != CRANE_BUSY) {
        return -1;
    }
    int finished = currentJobId;
    currentJobId = -1;
    status = CRANE_AVAILABLE;
    return finished;
}

void Crane::scheduleBreakdown(int time, int repairDuration) {
    breakdownTime = time;
    repairMinutes = (repairDuration > 0) ? repairDuration : 1;
}

bool Crane::isBreakdownDue(int currentTime) const {
    return breakdownTime != -1 && currentTime >= breakdownTime &&
           status != CRANE_OUT_OF_SERVICE;
}

bool Crane::isRepairDue(int currentTime) const {
    return status == CRANE_OUT_OF_SERVICE && currentTime >= backInServiceTime;
}

// Takes the crane out of service and returns the job it was working on
// (-1 if it was idle) so the caller can put that job back in the queue.
// A repairDuration of 0 uses the duration given to scheduleBreakdown().
int Crane::setOutOfService(int currentTime, int repairDuration) {
    if (repairDuration <= 0) {
        repairDuration = (repairMinutes > 0) ? repairMinutes : 1;
    }
    int interruptedJob = (status == CRANE_BUSY) ? currentJobId : -1;
    currentJobId = -1;
    status = CRANE_OUT_OF_SERVICE;
    backInServiceTime = currentTime + repairDuration;
    breakdownTime = -1;
    return interruptedJob;
}

bool Crane::repair() {
    if (status != CRANE_OUT_OF_SERVICE) {
        return false;
    }
    status = CRANE_AVAILABLE;
    backInServiceTime = -1;
    return true;
}

void Crane::display() const {
    cout << "Crane " << craneId << " (lift " << maximumLiftWeight << " t, "
         << handlingMinutesPerContainer << " min/container): "
         << statusToString(status);
    if (status == CRANE_BUSY) {
        cout << " on job " << currentJobId;
    } else if (status == CRANE_OUT_OF_SERVICE) {
        cout << " until " << PortTime(backInServiceTime);
    }
    cout << endl;
}

const char* Crane::statusToString(int status) {
    switch (status) {
        case CRANE_AVAILABLE:      return "AVAILABLE";
        case CRANE_BUSY:           return "BUSY";
        case CRANE_OUT_OF_SERVICE: return "OUT_OF_SERVICE";
        default:                   return "UNKNOWN";
    }
}
