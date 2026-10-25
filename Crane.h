#ifndef CRANE_H
#define CRANE_H

#include "Container.h"

// Crane status values
const int CRANE_AVAILABLE = 0;
const int CRANE_BUSY = 1;
const int CRANE_OUT_OF_SERVICE = 2;

class Crane {
private:
    int craneId;
    double maximumLiftWeight;   // in tonnes
    int currentJobId;
    int status;
    int handlingMinutesPerContainer;

    int breakdownTime;          // planned breakdown, -1 if none
    int repairMinutes;
    int backInServiceTime;      // -1 unless out of service

public:
    Crane();
    Crane(int id, double maxLift, int minutesPerContainer);

    int getId() const;
    double getMaximumLiftWeight() const;
    int getCurrentJobId() const;
    int getStatus() const;
    int getHandlingMinutes() const;
    int getBackInServiceTime() const;

    bool canHandle(const Container& container) const;
    bool isAvailable() const;
    bool isInService() const;
    bool assignJob(int jobId);
    int completeJob();

    void scheduleBreakdown(int time, int repairDuration);
    bool isBreakdownDue(int currentTime) const;
    bool isRepairDue(int currentTime) const;
    int setOutOfService(int currentTime, int repairDuration);
    bool repair();

    void display() const;

    static const char* statusToString(int status);
};

#endif
