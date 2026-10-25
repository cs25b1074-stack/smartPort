#ifndef HANDLINGJOB_H
#define HANDLINGJOB_H

// Job types
const int JOB_UNLOAD = 0;       // ship -> yard
const int JOB_LOAD = 1;         // yard -> ship

// Job status values
const int JOB_PENDING = 0;
const int JOB_IN_PROGRESS = 1;
const int JOB_COMPLETED = 2;
const int JOB_CANCELLED = 3;

// Priority levels (higher number = more urgent)
const int PRIORITY_NORMAL = 1;
const int PRIORITY_URGENT = 2;      // ship's deadline is close
const int PRIORITY_EMERGENCY = 3;   // always scheduled first

class HandlingJob {
private:
    static int nextId;          // used to generate unique job IDs

    int jobId;
    int shipId;
    int containerId;
    int jobType;
    int priority;
    int deadline;
    int estimatedDuration;      // minutes, set when a crane is assigned
    int assignedCraneId;
    int status;
    int startTime;
    int completionTime;
    int interruptions;          // times the job was stopped by a breakdown

public:
    HandlingJob();
    HandlingJob(int shipId, int containerId, int jobType, int deadline,
                int priority = PRIORITY_NORMAL);

    int getId() const;
    int getShipId() const;
    int getContainerId() const;
    int getType() const;
    int getPriority() const;
    int getDeadline() const;
    int getDuration() const;
    int getAssignedCraneId() const;
    int getStatus() const;
    int getStartTime() const;
    int getCompletionTime() const;
    int getFinishTime() const;
    int getInterruptions() const;

    bool assignCrane(int craneId, int duration);
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
