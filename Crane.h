#ifndef CRANE_H
#define CRANE_H

#include "Container.h"

class Crane {
public:
    // Crane status values
    static const int AVAILABLE = 1;
    static const int BUSY = 2;
    static const int OUT_OF_SERVICE = 3;

    // Validation limits
    static const double MIN_LIFT_WEIGHT;        // 1 t
    static const double MAX_LIFT_WEIGHT;        // 100 t
    static const int MIN_HANDLING_TIME = 1;     // minutes per container
    static const int MAX_HANDLING_TIME = 60;
    static const int MAX_REPAIR_TIME = 1440;    // minutes

private:
    static int nextId;          // used to generate unique crane IDs

    int craneId;
    double maximumLiftWeight;   // in tonnes
    int currentJobId;
    int status;
    int handlingMinutesPerContainer;
    int busyMinutes;            // total time spent on finished jobs
    bool valid;

    int breakdownTime;          // planned breakdown, -1 if none
    int repairMinutes;          // repair time for the planned breakdown
    int backInServiceTime;      // -1 unless out of service with a known repair time

public:
    Crane();
    Crane(double maxLiftWeight, int minutesPerContainer);

    static bool isValidLiftWeight(double weight);
    static bool isValidHandlingTime(int minutes);
    static bool isValidRepairTime(int minutes);

    bool isValid() const;
    int getId() const;
    double getMaximumLiftWeight() const;
    int getCurrentJobId() const;
    int getStatus() const;
    int getHandlingTime() const;
    int getBusyMinutes() const;
    int getRepairMinutes() const;
    int getBackInServiceTime() const;

    bool canHandle(const Container& container) const;
    bool isAvailable() const;
    bool isInService() const;
    bool assignJob(int jobId);
    void completeJob(int minutesWorked);
    void setOutOfService(int backInServiceAt = -1);
    void repair();

    bool scheduleBreakdown(int time, int repairDuration);
    bool isBreakdownDue(int currentTime) const;
    bool isRepairDue(int currentTime) const;

    void display() const;

    static const char* statusToString(int status);
};

#endif
