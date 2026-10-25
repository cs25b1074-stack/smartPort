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
    PortScheduler(int initialCapacity = 10);
    PortScheduler(const PortScheduler& other);
    PortScheduler& operator=(const PortScheduler& other);
    ~PortScheduler();

    bool addJob(const HandlingJob& job);
    int selectNextJob() const;
    void prioritizeJobs();
    int prioritizeJobs(int currentTime, int urgencyWindow);

    HandlingJob* getJob(int index);
    HandlingJob* findJob(int jobId);
    const HandlingJob* findJob(int jobId) const;

    int getJobCount() const;
    int getPendingCount() const;
    int countUnfinishedJobs(int shipId) const;
    void displayPendingJobs() const;
};

#endif
