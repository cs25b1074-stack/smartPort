#include "HandlingJob.h"
#include "PortTime.h"
#include <iostream>

using namespace std;

int HandlingJob::nextId = 1;

bool HandlingJob::isValidJobType(int type) {
    return type == UNLOAD || type == LOAD;
}

bool HandlingJob::isValidPriority(int priority) {
    return priority >= HIGHEST_PRIORITY && priority <= LOWEST_PRIORITY;
}

// Placeholder used to fill arrays; it is invalid and does not use up an ID
HandlingJob::HandlingJob()
    : jobId(0), shipId(0), containerId(0), jobType(0), priority(0),
      deadline(0), estimatedDuration(0), assignedCraneId(-1), status(0),
      startTime(-1), completionTime(-1), interruptions(0), valid(false) {
}

// Jobs are created by PortController, but the values are still checked
HandlingJob::HandlingJob(int shipId, int containerId, int jobType,
                         int priority, int deadline, int estimatedDuration)
    : HandlingJob() {
    if (shipId <= 0 || containerId <= 0 || !isValidJobType(jobType) ||
        !isValidPriority(priority) || deadline < 0 || estimatedDuration <= 0) {
        return;
    }
    this->shipId = shipId;
    this->containerId = containerId;
    this->jobType = jobType;
    this->priority = priority;
    this->deadline = deadline;
    this->estimatedDuration = estimatedDuration;
    status = PENDING;
    jobId = nextId++;
    valid = true;
}

bool HandlingJob::isValid() const {
    return valid;
}

int HandlingJob::getId() const {
    return jobId;
}

int HandlingJob::getShipId() const {
    return shipId;
}

int HandlingJob::getContainerId() const {
    return containerId;
}

int HandlingJob::getJobType() const {
    return jobType;
}

int HandlingJob::getPriority() const {
    return priority;
}

int HandlingJob::getDeadline() const {
    return deadline;
}

int HandlingJob::getDuration() const {
    return estimatedDuration;
}

int HandlingJob::getAssignedCraneId() const {
    return assignedCraneId;
}

int HandlingJob::getStatus() const {
    return status;
}

int HandlingJob::getStartTime() const {
    return startTime;
}

int HandlingJob::getCompletionTime() const {
    return completionTime;
}

// Time at which an in-progress job will be done (-1 if not started)
int HandlingJob::getFinishTime() const {
    if (startTime < 0) {
        return -1;
    }
    return startTime + estimatedDuration;
}

int HandlingJob::getInterruptions() const {
    return interruptions;
}

bool HandlingJob::assignCrane(int craneId) {
    if (status != PENDING || craneId <= 0) {
        return false;
    }
    assignedCraneId = craneId;
    return true;
}

// Overload used when the crane's own speed replaces the estimate
bool HandlingJob::assignCrane(int craneId, int actualDuration) {
    if (actualDuration <= 0 || !assignCrane(craneId)) {
        return false;
    }
    estimatedDuration = actualDuration;
    return true;
}

bool HandlingJob::markStarted(int currentTime) {
    if (status != PENDING || assignedCraneId == -1 || currentTime < 0) {
        return false;
    }
    startTime = currentTime;
    status = IN_PROGRESS;
    return true;
}

bool HandlingJob::markCompleted(int currentTime) {
    if (status != IN_PROGRESS || currentTime < startTime) {
        return false;
    }
    completionTime = currentTime;
    status = COMPLETED;
    return true;
}

// Puts an in-progress job back in the queue (e.g. after a crane breakdown)
bool HandlingJob::interrupt() {
    if (status != IN_PROGRESS) {
        return false;
    }
    status = PENDING;
    assignedCraneId = -1;
    startTime = -1;
    interruptions++;
    return true;
}

bool HandlingJob::cancel() {
    if (status != PENDING) {
        return false;
    }
    status = CANCELLED;
    return true;
}

// Priority can only move towards 1 (more urgent), never away from it
bool HandlingJob::raisePriority(int newPriority) {
    if (!isValidPriority(newPriority) || newPriority >= priority) {
        return false;
    }
    priority = newPriority;
    return true;
}

bool HandlingJob::isPending() const {
    return status == PENDING;
}

bool HandlingJob::isActive() const {
    return status == IN_PROGRESS;
}

bool HandlingJob::isCompleted() const {
    return status == COMPLETED;
}

bool HandlingJob::isFinished() const {
    return status == COMPLETED || status == CANCELLED;
}

// Scheduling rule: "a < b" means a should be handled before b.
//   1. Emergency jobs (priority 1) first
//   2. Earlier departure deadline first
//   3. Higher priority (smaller number) breaks ties
//   4. Lower job ID (created earlier) breaks remaining ties
bool HandlingJob::operator<(const HandlingJob& other) const {
    bool thisEmergency = (priority == EMERGENCY_PRIORITY);
    bool otherEmergency = (other.priority == EMERGENCY_PRIORITY);
    if (thisEmergency != otherEmergency) {
        return thisEmergency;
    }
    if (deadline != other.deadline) {
        return deadline < other.deadline;
    }
    if (priority != other.priority) {
        return priority < other.priority;
    }
    return jobId < other.jobId;
}

void HandlingJob::display() const {
    if (!valid) {
        cout << "Job (invalid placeholder)" << endl;
        return;
    }
    cout << "Job #" << jobId << " " << typeToString(jobType)
         << " container " << containerId << " (ship " << shipId
         << ", deadline " << PortTime(deadline) << ", priority " << priority
         << "): " << statusToString(status);
    if (assignedCraneId != -1) {
        cout << " on crane " << assignedCraneId;
    }
    cout << endl;
}

const char* HandlingJob::typeToString(int type) {
    switch (type) {
        case UNLOAD: return "UNLOAD";
        case LOAD:   return "LOAD";
        default:     return "UNKNOWN";
    }
}

const char* HandlingJob::statusToString(int status) {
    switch (status) {
        case PENDING:     return "PENDING";
        case IN_PROGRESS: return "IN_PROGRESS";
        case COMPLETED:   return "COMPLETED";
        case CANCELLED:   return "CANCELLED";
        default:          return "UNKNOWN";
    }
}
