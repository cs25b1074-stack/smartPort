#include "HandlingJob.h"
#include "PortTime.h"
#include <iostream>

using namespace std;

int HandlingJob::nextId = 1;

// Placeholder job used to fill arrays; it does not use up an ID
HandlingJob::HandlingJob() {
    jobId = 0;
    shipId = -1;
    containerId = -1;
    jobType = JOB_UNLOAD;
    priority = PRIORITY_NORMAL;
    deadline = 0;
    estimatedDuration = 0;
    assignedCraneId = -1;
    status = JOB_PENDING;
    startTime = -1;
    completionTime = -1;
    interruptions = 0;
}

HandlingJob::HandlingJob(int shipId, int containerId, int jobType, int deadline,
                         int priority) {
    jobId = nextId++;
    this->shipId = shipId;
    this->containerId = containerId;
    this->jobType = (jobType == JOB_LOAD) ? JOB_LOAD : JOB_UNLOAD;
    this->deadline = deadline;
    this->priority = (priority >= PRIORITY_NORMAL && priority <= PRIORITY_EMERGENCY)
                         ? priority : PRIORITY_NORMAL;
    estimatedDuration = 0;
    assignedCraneId = -1;
    status = JOB_PENDING;
    startTime = -1;
    completionTime = -1;
    interruptions = 0;
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

int HandlingJob::getType() const {
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

bool HandlingJob::assignCrane(int craneId, int duration) {
    if (status != JOB_PENDING) {
        cout << "Error: job " << jobId << " is " << statusToString(status)
             << " and cannot be given a crane" << endl;
        return false;
    }
    assignedCraneId = craneId;
    estimatedDuration = (duration > 0) ? duration : 1;
    return true;
}

bool HandlingJob::markStarted(int currentTime) {
    if (status != JOB_PENDING || assignedCraneId == -1) {
        cout << "Error: job " << jobId << " cannot start without a crane" << endl;
        return false;
    }
    startTime = currentTime;
    status = JOB_IN_PROGRESS;
    return true;
}

bool HandlingJob::markCompleted(int currentTime) {
    if (status != JOB_IN_PROGRESS) {
        cout << "Error: job " << jobId << " cannot complete while "
             << statusToString(status) << endl;
        return false;
    }
    completionTime = currentTime;
    status = JOB_COMPLETED;
    return true;
}

// Puts an in-progress job back in the queue (e.g. after a crane breakdown)
bool HandlingJob::interrupt() {
    if (status != JOB_IN_PROGRESS) {
        return false;
    }
    status = JOB_PENDING;
    assignedCraneId = -1;
    startTime = -1;
    estimatedDuration = 0;
    interruptions++;
    return true;
}

bool HandlingJob::cancel() {
    if (status != JOB_PENDING) {
        cout << "Error: job " << jobId << " cannot be cancelled while "
             << statusToString(status) << endl;
        return false;
    }
    status = JOB_CANCELLED;
    return true;
}

// Priority can only go up, never down
bool HandlingJob::raisePriority(int newPriority) {
    if (newPriority <= priority || newPriority > PRIORITY_EMERGENCY) {
        return false;
    }
    priority = newPriority;
    return true;
}

bool HandlingJob::isPending() const {
    return status == JOB_PENDING;
}

bool HandlingJob::isActive() const {
    return status == JOB_IN_PROGRESS;
}

bool HandlingJob::isCompleted() const {
    return status == JOB_COMPLETED;
}

bool HandlingJob::isFinished() const {
    return status == JOB_COMPLETED || status == JOB_CANCELLED;
}

// Scheduling rule: "a < b" means a should be handled before b.
//   1. Emergency jobs first
//   2. Earlier departure deadline first
//   3. Higher priority breaks ties
//   4. Lower job ID (created earlier) breaks remaining ties
bool HandlingJob::operator<(const HandlingJob& other) const {
    bool thisEmergency = (priority == PRIORITY_EMERGENCY);
    bool otherEmergency = (other.priority == PRIORITY_EMERGENCY);
    if (thisEmergency != otherEmergency) {
        return thisEmergency;
    }
    if (deadline != other.deadline) {
        return deadline < other.deadline;
    }
    if (priority != other.priority) {
        return priority > other.priority;
    }
    return jobId < other.jobId;
}

void HandlingJob::display() const {
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
        case JOB_UNLOAD: return "UNLOAD";
        case JOB_LOAD:   return "LOAD";
        default:         return "UNKNOWN";
    }
}

const char* HandlingJob::statusToString(int status) {
    switch (status) {
        case JOB_PENDING:     return "PENDING";
        case JOB_IN_PROGRESS: return "IN_PROGRESS";
        case JOB_COMPLETED:   return "COMPLETED";
        case JOB_CANCELLED:   return "CANCELLED";
        default:              return "UNKNOWN";
    }
}
