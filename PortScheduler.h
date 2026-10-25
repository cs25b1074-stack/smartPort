#ifndef PORTSCHEDULER_H
#define PORTSCHEDULER_H

#include "HandlingJob.h"

class PortScheduler {
private:
    HandlingJob* jobs;
    int jobCount;
    int jobCapacity;

    void grow();

public:
    PortScheduler(int initialCapacity = 20);
    PortScheduler(const PortScheduler& other);
    PortScheduler& operator=(const PortScheduler& other);
    ~PortScheduler();

    void addJob(const HandlingJob& job);
    int selectNextJob() const;
    int prioritizeJobs(int currentTime, int urgencyWindow);

    HandlingJob* getJobAt(int index);
    HandlingJob* findJob(int jobId);
    const HandlingJob* findJob(int jobId) const;

    int getJobCount() const;
    int countPendingJobs() const;
    int countUnfinishedJobs(int shipId) const;
    bool hasJobsForShip(int shipId) const;
    void displayPendingJobs() const;
};

#endif
