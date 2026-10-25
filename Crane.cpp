#include "Crane.h"
#include "PortTime.h"
#include <iostream>

using namespace std;

const double Crane::MIN_LIFT_WEIGHT = 1.0;
const double Crane::MAX_LIFT_WEIGHT = 100.0;
int Crane::nextId = 1;

bool Crane::isValidLiftWeight(double weight) {
    return weight >= MIN_LIFT_WEIGHT && weight <= MAX_LIFT_WEIGHT;
}

bool Crane::isValidHandlingTime(int minutes) {
    return minutes >= MIN_HANDLING_TIME && minutes <= MAX_HANDLING_TIME;
}

bool Crane::isValidRepairTime(int minutes) {
    return minutes >= 1 && minutes <= MAX_REPAIR_TIME;
}

// Placeholder used to fill arrays; it is invalid and does not use up an ID
Crane::Crane()
    : craneId(0), maximumLiftWeight(0.0), currentJobId(-1), status(0),
      handlingMinutesPerContainer(0), busyMinutes(0), valid(false),
      breakdownTime(-1), repairMinutes(0), backInServiceTime(-1) {
}

// If either value is illegal the object stays an invalid placeholder
Crane::Crane(double maxLiftWeight, int minutesPerContainer)
    : Crane() {
    if (!isValidLiftWeight(maxLiftWeight) || !isValidHandlingTime(minutesPerContainer)) {
        return;
    }
    maximumLiftWeight = maxLiftWeight;
    handlingMinutesPerContainer = minutesPerContainer;
    status = AVAILABLE;
    craneId = nextId++;
    valid = true;
}

bool Crane::isValid() const {
    return valid;
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

int Crane::getHandlingTime() const {
    return handlingMinutesPerContainer;
}

int Crane::getBusyMinutes() const {
    return busyMinutes;
}

int Crane::getRepairMinutes() const {
    return repairMinutes;
}

int Crane::getBackInServiceTime() const {
    return backInServiceTime;
}

bool Crane::canHandle(const Container& container) const {
    return valid && container.isValid() &&
           container.getWeight() <= maximumLiftWeight;
}

bool Crane::isAvailable() const {
    return valid && status == AVAILABLE;
}

bool Crane::isInService() const {
    return valid && status != OUT_OF_SERVICE;
}

// A crane only ever works on one job at a time
bool Crane::assignJob(int jobId) {
    if (!isAvailable() || jobId <= 0) {
        return false;
    }
    currentJobId = jobId;
    status = BUSY;
    return true;
}

void Crane::completeJob(int minutesWorked) {
    if (status != BUSY) {
        return;
    }
    if (minutesWorked > 0) {
        busyMinutes += minutesWorked;
    }
    currentJobId = -1;
    status = AVAILABLE;
}

// Stops the crane, dropping any job it was working on. The caller reads
// getCurrentJobId() first so it can put that job back in the queue.
// backInServiceAt = -1 means "until repair() is called".
void Crane::setOutOfService(int backInServiceAt) {
    if (!valid) {
        return;
    }
    currentJobId = -1;
    status = OUT_OF_SERVICE;
    backInServiceTime = backInServiceAt;
    breakdownTime = -1;
}

void Crane::repair() {
    if (status != OUT_OF_SERVICE) {
        return;
    }
    status = AVAILABLE;
    backInServiceTime = -1;
}

bool Crane::scheduleBreakdown(int time, int repairDuration) {
    if (!valid || time < 0 || !isValidRepairTime(repairDuration)) {
        return false;
    }
    breakdownTime = time;
    repairMinutes = repairDuration;
    return true;
}

bool Crane::isBreakdownDue(int currentTime) const {
    return breakdownTime != -1 && currentTime >= breakdownTime &&
           status != OUT_OF_SERVICE;
}

bool Crane::isRepairDue(int currentTime) const {
    return status == OUT_OF_SERVICE && backInServiceTime != -1 &&
           currentTime >= backInServiceTime;
}

void Crane::display() const {
    if (!valid) {
        cout << "Crane (invalid placeholder)" << endl;
        return;
    }
    cout << "Crane " << craneId << " (lift " << maximumLiftWeight << " t, "
         << handlingMinutesPerContainer << " min/container): "
         << statusToString(status);
    if (status == BUSY) {
        cout << " on job " << currentJobId;
    } else if (status == OUT_OF_SERVICE && backInServiceTime != -1) {
        cout << " until " << PortTime(backInServiceTime);
    }
    cout << ", " << busyMinutes << " min worked";
    if (breakdownTime != -1) {
        cout << ", breakdown planned at " << PortTime(breakdownTime);
    }
    cout << endl;
}

const char* Crane::statusToString(int status) {
    switch (status) {
        case AVAILABLE:      return "AVAILABLE";
        case BUSY:           return "BUSY";
        case OUT_OF_SERVICE: return "OUT_OF_SERVICE";
        default:             return "UNKNOWN";
    }
}
