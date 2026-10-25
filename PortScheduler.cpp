#include "PortScheduler.h"
#include <iostream>

using namespace std;

// A bad capacity is corrected rather than rejected, so the scheduler is always usable
PortScheduler::PortScheduler(int initialCapacity) {
    jobCapacity = (initialCapacity >= 1) ? initialCapacity : 10;
    jobCount = 0;
    jobs = new HandlingJob[jobCapacity];
}

PortScheduler::PortScheduler(const PortScheduler& other) {
    jobCapacity = other.jobCapacity;
    jobCount = other.jobCount;
    jobs = new HandlingJob[jobCapacity];
    for (int i = 0; i < jobCount; i++) {
        jobs[i] = other.jobs[i];
    }
}

PortScheduler& PortScheduler::operator=(const PortScheduler& other) {
    if (this == &other) {
        return *this;
    }

    HandlingJob* newJobs = new HandlingJob[other.jobCapacity];
    for (int i = 0; i < other.jobCount; i++) {
        newJobs[i] = other.jobs[i];
    }

    delete[] jobs;
    jobs = newJobs;
    jobCapacity = other.jobCapacity;
    jobCount = other.jobCount;
    return *this;
}

PortScheduler::~PortScheduler() {
    delete[] jobs;
}

void PortScheduler::grow() {
    int newCapacity = jobCapacity * 2;
    HandlingJob* newJobs = new HandlingJob[newCapacity];
    for (int i = 0; i < jobCount; i++) {
        newJobs[i] = jobs[i];
    }
    delete[] jobs;
    jobs = newJobs;
    jobCapacity = newCapacity;
}

// Rejects invalid jobs and duplicate IDs
bool PortScheduler::addJob(const HandlingJob& job) {
    if (!job.isValid() || findJob(job.getId()) != nullptr) {
        return false;
    }
    if (jobCount == jobCapacity) {
        grow();
    }
    jobs[jobCount++] = job;
    return true;
}

// Returns the index of the pending job that should go next, or -1.
// Uses HandlingJob::operator< so the rule is defined in one place.
int PortScheduler::selectNextJob() const {
    int best = -1;
    for (int i = 0; i < jobCount; i++) {
        if (jobs[i].isPending() && (best == -1 || jobs[i] < jobs[best])) {
            best = i;
        }
    }
    return best;
}

// Sorts all jobs into scheduling order (insertion sort with operator<)
void PortScheduler::prioritizeJobs() {
    for (int i = 1; i < jobCount; i++) {
        HandlingJob key = jobs[i];
        int j = i - 1;
        while (j >= 0 && key < jobs[j]) {
            jobs[j + 1] = jobs[j];
            j--;
        }
        jobs[j + 1] = key;
    }
}

// Raises jobs whose deadline is within the urgency window to URGENT, then
// sorts. Returns how many jobs were newly raised.
int PortScheduler::prioritizeJobs(int currentTime, int urgencyWindow) {
    int raised = 0;
    for (int i = 0; i < jobCount; i++) {
        if (jobs[i].isPending() && jobs[i].getDeadline() - currentTime <= urgencyWindow) {
            if (jobs[i].raisePriority(HandlingJob::URGENT_PRIORITY)) {
                raised++;
            }
        }
    }
    prioritizeJobs();
    return raised;
}

// Returns nullptr for a bad index
HandlingJob* PortScheduler::getJob(int index) {
    if (index < 0 || index >= jobCount) {
        return nullptr;
    }
    return &jobs[index];
}

HandlingJob* PortScheduler::findJob(int jobId) {
    for (int i = 0; i < jobCount; i++) {
        if (jobs[i].getId() == jobId) {
            return &jobs[i];
        }
    }
    return nullptr;
}

const HandlingJob* PortScheduler::findJob(int jobId) const {
    for (int i = 0; i < jobCount; i++) {
        if (jobs[i].getId() == jobId) {
            return &jobs[i];
        }
    }
    return nullptr;
}

int PortScheduler::getJobCount() const {
    return jobCount;
}

int PortScheduler::getPendingCount() const {
    int pending = 0;
    for (int i = 0; i < jobCount; i++) {
        if (jobs[i].isPending()) {
            pending++;
        }
    }
    return pending;
}

int PortScheduler::countUnfinishedJobs(int shipId) const {
    int unfinished = 0;
    for (int i = 0; i < jobCount; i++) {
        if (jobs[i].getShipId() == shipId && !jobs[i].isFinished()) {
            unfinished++;
        }
    }
    return unfinished;
}

void PortScheduler::displayPendingJobs() const {
    int pending = getPendingCount();
    cout << "Pending jobs: " << pending << endl;
    if (pending == 0) {
        return;
    }
    int next = selectNextJob();
    for (int i = 0; i < jobCount; i++) {
        if (jobs[i].isPending()) {
            cout << (i == next ? "  -> " : "     ");
            jobs[i].display();
        }
    }
}
