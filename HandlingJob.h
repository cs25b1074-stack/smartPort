#ifndef HANDLINGJOB_H
#define HANDLINGJOB_H

class HandlingJob {
public:
    // Job types
    static const int UNLOAD = 1;        // ship -> yard
    static const int LOAD = 2;          // yard -> ship

    // Job status values
    static const int PENDING = 1;
    static const int IN_PROGRESS = 2;
    static const int COMPLETED = 3;
    static const int CANCELLED = 4;     // job can never be done (e.g. too heavy)

    // Priority: 1 is the highest, 5 the lowest
    static const int HIGHEST_PRIORITY = 1;
    static const int LOWEST_PRIORITY = 5;
    static const int EMERGENCY_PRIORITY = 1;    // always scheduled first
    static const int URGENT_PRIORITY = 2;       // ship's deadline is close
    static const int NORMAL_PRIORITY = 3;

private:
    static int nextId;          // used to generate unique job IDs

    int jobId;
    int shipId;
    int containerId;
    int jobType;
    int priority;
    int deadline;
    int estimatedDuration;      // minutes
    int assignedCraneId;
    int status;
    int startTime;
    int completionTime;
    int interruptions;          // times the job was stopped by a breakdown
    bool valid;

public:
    HandlingJob();
    HandlingJob(int shipId, int containerId, int jobType,
                int priority, int deadline, int estimatedDuration);

    static bool isValidJobType(int type);
    static bool isValidPriority(int priority);

    bool isValid() const;
    int getId() const;
    int getShipId() const;
    int getContainerId() const;
    int getJobType() const;
    int getPriority() const;
    int getDeadline() const;
    int getDuration() const;
    int getAssignedCraneId() const;
    int getStatus() const;
    int getStartTime() const;
    int getCompletionTime() const;
    int getFinishTime() const;
    int getInterruptions() const;

    bool assignCrane(int craneId);
    bool assignCrane(int craneId, int actualDuration);
    bool markStarted(int currentTime);
    bool markCompleted(int currentTime);
    bool interrupt();
    bool cancel();
    bool raisePriority(int newPriority);

    bool isPending() const;
    bool isActive() const;
    bool isCompleted() const;
    bool isFinished() const;

    bool operator<(const HandlingJob& other) const;
    void display() const;

    static const char* typeToString(int type);
    static const char* statusToString(int status);
};

#endif
