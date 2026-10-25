#include "PortScheduler.h"
#include <iostream>

using namespace std;

PortScheduler::PortScheduler(int initialCapacity) {
    jobCapacity = (initialCapacity > 0) ? initialCapacity : 20;
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

void PortScheduler::addJob(const HandlingJob& job) {
    if (findJob(job.getId()) != nullptr) {
        cout << "Error: job " << job.getId() << " is already scheduled" << endl;
        return;
    }
    if (jobCount == jobCapacity) {
        grow();
    }
    jobs[jobCount++] = job;
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

// Raises jobs whose deadline is within the urgency window to URGENT,
// then sorts all jobs into scheduling order (insertion sort with operator<).
// Returns how many jobs were newly raised.
int PortScheduler::prioritizeJobs(int currentTime, int urgencyWindow) {
    int raised = 0;
    for (int i = 0; i < jobCount; i++) {
        if (jobs[i].isPending() && jobs[i].getDeadline() - currentTime <= urgencyWindow) {
            if (jobs[i].raisePriority(PRIORITY_URGENT)) {
                raised++;
            }
        }
    }

    for (int i = 1; i < jobCount; i++) {
        HandlingJob key = jobs[i];
        int j = i - 1;
        while (j >= 0 && key < jobs[j]) {
            jobs[j + 1] = jobs[j];
            j--;
        }
        jobs[j + 1] = key;
    }
    return raised;
}

HandlingJob* PortScheduler::getJobAt(int index) {
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

int PortScheduler::countPendingJobs() const {
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

bool PortScheduler::hasJobsForShip(int shipId) const {
    for (int i = 0; i < jobCount; i++) {
        if (jobs[i].getShipId() == shipId) {
            return true;
        }
    }
    return false;
}

void PortScheduler::displayPendingJobs() const {
    int pending = countPendingJobs();
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
