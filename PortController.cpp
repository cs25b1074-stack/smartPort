#include "PortController.h"
#include <iostream>
#include <iomanip>

using namespace std;

PortController::PortController(int startMinutes, int yardCapacity,
                               int maxBerths, int maxCranes)
    : registry(20), yard(yardCapacity), scheduler(20),
      currentTime(startMinutes), importQueue(20) {
    shipCapacity = 5;
    shipCount = 0;
    ships = new Ship[shipCapacity];

    berthCapacity = (maxBerths > 0) ? maxBerths : 1;
    berthCount = 0;
    berths = new Berth[berthCapacity];

    craneCapacity = (maxCranes > 0) ? maxCranes : 1;
    craneCount = 0;
    cranes = new Crane[craneCapacity];

    truckInterval = 5;
    urgencyWindow = 60;
    yardBlockReported = false;
}

PortController::~PortController() {
    delete[] ships;
    delete[] berths;
    delete[] cranes;
}

// ---------------------------------------------------------------------------
// Setup
// ---------------------------------------------------------------------------

bool PortController::addBerth(double maxLength, double depth) {
    if (berthCount == berthCapacity) {
        cout << "Error: the port has no room for another berth" << endl;
        return false;
    }
    berths[berthCount] = Berth(berthCount + 1, maxLength, depth);
    berthCount++;
    return true;
}

bool PortController::addCrane(double maxLift, int minutesPerContainer) {
    if (craneCount == craneCapacity) {
        cout << "Error: the port has no room for another crane" << endl;
        return false;
    }
    cranes[craneCount] = Crane(craneCount + 1, maxLift, minutesPerContainer);
    craneCount++;
    return true;
}

int PortController::addYardSlots(int count, int type, double maxWeight) {
    return yard.addSlots(count, type, maxWeight);
}

void PortController::setTruckInterval(int minutes) {
    truckInterval = (minutes > 0) ? minutes : 1;
}

void PortController::setUrgencyWindow(int minutes) {
    urgencyWindow = (minutes >= 0) ? minutes : 0;
}

void PortController::growShips() {
    int newCapacity = shipCapacity * 2;
    Ship* newShips = new Ship[newCapacity];
    for (int i = 0; i < shipCount; i++) {
        newShips[i] = ships[i];     // Ship assignment deep-copies its manifests
    }
    delete[] ships;
    ships = newShips;
    shipCapacity = newCapacity;
}

bool PortController::registerShip(const Ship& ship) {
    if (ship.getId() == 0) {
        cout << "Error: cannot register a placeholder ship" << endl;
        return false;
    }
    if (findShip(ship.getId()) != nullptr) {
        cout << "Error: ship " << ship.getId() << " is already registered" << endl;
        return false;
    }
    if (shipCount == shipCapacity) {
        growShips();
    }
    ships[shipCount++] = ship;
    return true;
}

bool PortController::registerContainer(const Container& container) {
    return registry.addContainer(container);
}

// Adds a container that arrives on the ship and must be unloaded here
bool PortController::assignImportCargo(int shipId, int containerId) {
    Ship* ship = findShip(shipId);
    if (ship == nullptr) {
        cout << "Error: unknown ship ID " << shipId << endl;
        return false;
    }
    const Container* container = registry.findContainer(containerId);
    if (container == nullptr) {
        cout << "Error: unknown container ID " << containerId << endl;
        return false;
    }
    if (container->getStatus() != ON_SHIP) {
        cout << "Error: container " << containerId << " is not aboard a ship" << endl;
        return false;
    }
    if (isOnAnyShip(containerId)) {
        cout << "Error: container " << containerId
             << " is already assigned to a ship" << endl;
        return false;
    }
    return ship->addContainer(containerId);
}

// Places a container (delivered by road) in the yard and adds it to the
// ship's loading list. New containers start as ON_SHIP, so they are
// moved straight into the yard here.
bool PortController::assignExportCargo(int shipId, int containerId) {
    Ship* ship = findShip(shipId);
    if (ship == nullptr) {
        cout << "Error: unknown ship ID " << shipId << endl;
        return false;
    }
    Container* container = registry.findContainer(containerId);
    if (container == nullptr) {
        cout << "Error: unknown container ID " << containerId << endl;
        return false;
    }
    if (isOnAnyShip(containerId)) {
        cout << "Error: container " << containerId
             << " is already assigned to a ship" << endl;
        return false;
    }
    if (container->getStatus() != IN_YARD) {
        if (!yard.storeContainer(*container)) {
            cout << "Error: no yard slot for export container " << containerId << endl;
            return false;
        }
        container->setStatus(IN_YARD);
    }
    return ship->addLoadContainer(containerId);
}

bool PortController::scheduleCraneBreakdown(int craneId, int time, int repairMinutes) {
    Crane* crane = findCrane(craneId);
    if (crane == nullptr) {
        cout << "Error: unknown crane ID " << craneId << endl;
        return false;
    }
    crane->scheduleBreakdown(time, repairMinutes);
    return true;
}

// Manual departure request; rejected while handling work is unfinished
bool PortController::requestDeparture(int shipId) {
    Ship* ship = findShip(shipId);
    if (ship == nullptr) {
        cout << "Error: unknown ship ID " << shipId << endl;
        return false;
    }
    int unfinished = scheduler.countUnfinishedJobs(shipId);
    if (unfinished > 0) {
        logTime();
        cout << "Departure of " << ship->getName() << " refused: "
             << unfinished << " handling jobs still unfinished" << endl;
        return false;
    }
    if (ship->getStatus() == SHIP_OPERATING) {
        ship->markReadyToDepart();
    }
    if (ship->getStatus() != SHIP_READY_TO_DEPART) {
        logTime();
        cout << "Departure of " << ship->getName() << " refused: ship is "
             << Ship::statusToString(ship->getStatus()) << endl;
        return false;
    }
    departShip(*ship);
    return true;
}

// ---------------------------------------------------------------------------
// Lookup helpers
// ---------------------------------------------------------------------------

Ship* PortController::findShip(int shipId) {
    for (int i = 0; i < shipCount; i++) {
        if (ships[i].getId() == shipId) {
            return &ships[i];
        }
    }
    return nullptr;
}

const Ship* PortController::findShip(int shipId) const {
    for (int i = 0; i < shipCount; i++) {
        if (ships[i].getId() == shipId) {
            return &ships[i];
        }
    }
    return nullptr;
}

Crane* PortController::findCrane(int craneId) {
    for (int i = 0; i < craneCount; i++) {
        if (cranes[i].getId() == craneId) {
            return &cranes[i];
        }
    }
    return nullptr;
}

Berth* PortController::findBerth(int berthId) {
    for (int i = 0; i < berthCount; i++) {
        if (berths[i].getId() == berthId) {
            return &berths[i];
        }
    }
    return nullptr;
}

// Best fit: the smallest free berth that can take the ship, so large
// berths stay free for large ships. Returns an index or -1.
int PortController::findBestBerth(const Ship& ship) const {
    int best = -1;
    for (int i = 0; i < berthCount; i++) {
        if (berths[i].canAccommodate(ship) &&
            (best == -1 || berths[i].getMaximumLength() < berths[best].getMaximumLength())) {
            best = i;
        }
    }
    return best;
}

bool PortController::anyBerthFits(const Ship& ship) const {
    for (int i = 0; i < berthCount; i++) {
        if (berths[i].fitsDimensions(ship)) {
            return true;
        }
    }
    return false;
}

bool PortController::isOnAnyShip(int containerId) const {
    for (int i = 0; i < shipCount; i++) {
        if (ships[i].getManifest().contains(containerId) ||
            ships[i].getLoadingList().contains(containerId)) {
            return true;
        }
    }
    return false;
}

// Fastest available crane that can lift the container. Returns an index or -1.
int PortController::findBestCrane(const Container& container) const {
    int best = -1;
    for (int i = 0; i < craneCount; i++) {
        if (cranes[i].isAvailable() && cranes[i].canHandle(container) &&
            (best == -1 || cranes[i].getHandlingMinutes() < cranes[best].getHandlingMinutes())) {
            best = i;
        }
    }
    return best;
}

void PortController::logTime() const {
    cout << "[" << currentTime << "] ";
}

// ---------------------------------------------------------------------------
// Simulation steps
// ---------------------------------------------------------------------------

void PortController::processArrivals() {
    for (int i = 0; i < shipCount; i++) {
        Ship& ship = ships[i];
        if (ship.getStatus() != SHIP_SCHEDULED ||
            currentTime < PortTime(ship.getArrivalTime())) {
            continue;
        }
        ship.markArrived(currentTime.getMinutes());
        logTime();
        cout << ship.getName() << " arrived (" << ship.getManifest().getCount()
             << " to unload, " << ship.getLoadingList().getCount()
             << " to load, deadline " << PortTime(ship.getDeadline()) << ")" << endl;

        if (!anyBerthFits(ship)) {
            logTime();
            cout << "  No berth can accommodate " << ship.getName() << " ("
                 << ship.getLength() << " m long, " << ship.getRequiredDepth()
                 << " m draft); it stays at anchor" << endl;
        } else if (findBestBerth(ship) == -1) {
            logTime();
            cout << "  All compatible berths are occupied; " << ship.getName()
                 << " is waiting" << endl;
        }
    }
}

// Repeatedly gives a berth to the waiting ship with the earliest deadline
void PortController::allocateBerths() {
    bool assigned = true;
    while (assigned) {
        assigned = false;
        int bestShip = -1;
        for (int i = 0; i < shipCount; i++) {
            if (ships[i].getStatus() == SHIP_WAITING && findBestBerth(ships[i]) != -1 &&
                (bestShip == -1 || ships[i].getDeadline() < ships[bestShip].getDeadline())) {
                bestShip = i;
            }
        }
        if (bestShip == -1) {
            break;
        }

        Ship& ship = ships[bestShip];
        Berth& berth = berths[findBestBerth(ship)];
        if (berth.assignShip(ship) &&
            ship.markBerthed(berth.getId(), currentTime.getMinutes())) {
            logTime();
            cout << ship.getName() << " berthed at berth " << berth.getId();
            if (ship.getWaitingTime() > 0) {
                cout << " after waiting " << ship.getWaitingTime() << " min";
            }
            cout << endl;
            assigned = true;
        }
    }
}

// Creates one job per container for every newly berthed ship
void PortController::createHandlingJobs() {
    for (int i = 0; i < shipCount; i++) {
        Ship& ship = ships[i];
        if (ship.getStatus() != SHIP_BERTHED) {
            continue;
        }

        int created = 0;
        for (int pass = 0; pass < 2; pass++) {
            int jobType = (pass == 0) ? JOB_UNLOAD : JOB_LOAD;
            const Manifest& list = (pass == 0) ? ship.getManifest() : ship.getLoadingList();

            for (int k = 0; k < list.getCount(); k++) {
                int containerId = list[k];
                const Container* container = registry.findContainer(containerId);
                if (container == nullptr) {
                    logTime();
                    cout << "  Skipping unknown container " << containerId << endl;
                    continue;
                }

                HandlingJob job(ship.getId(), containerId, jobType, ship.getDeadline());

                bool liftable = false;
                for (int c = 0; c < craneCount; c++) {
                    if (cranes[c].canHandle(*container)) {
                        liftable = true;
                    } else {
                        logTime();
                        cout << "  Crane " << cranes[c].getId() << " cannot lift container "
                             << containerId << " (" << container->getWeight() << " t > "
                             << cranes[c].getMaximumLiftWeight() << " t limit)" << endl;
                    }
                }
                bool storable = (jobType == JOB_LOAD) || yard.supports(*container);

                if (!liftable || !storable) {
                    job.cancel();
                    logTime();
                    cout << "  Job #" << job.getId() << " cancelled: "
                         << (!liftable ? "no crane can lift" : "no yard slot can ever hold")
                         << " container " << containerId << endl;
                }
                scheduler.addJob(job);
                created++;
            }
        }

        ship.markOperating();
        logTime();
        cout << "  " << created << " handling jobs created for " << ship.getName() << endl;
    }
}

// Goes through jobs in scheduling order and gives each one the fastest
// suitable free crane. Jobs whose resources are not ready are skipped.
void PortController::assignCraneJobs() {
    int raised = scheduler.prioritizeJobs(currentTime.getMinutes(), urgencyWindow);
    if (raised > 0) {
        logTime();
        cout << raised << " jobs raised to URGENT (deadline within "
             << urgencyWindow << " min)" << endl;
    }

    bool yardBlocked = false;
    for (int i = 0; i < scheduler.getJobCount(); i++) {
        HandlingJob* job = scheduler.getJobAt(i);
        if (!job->isPending()) {
            continue;
        }

        Container* container = registry.findContainer(job->getContainerId());
        if (container == nullptr) {
            continue;
        }

        if (job->getType() == JOB_UNLOAD && !yard.hasSpaceFor(*container)) {
            if (!yardBlockReported) {
                logTime();
                cout << "Yard has no free " << Container::typeToString(container->getType())
                     << " slot; unloading of container " << container->getId()
                     << " paused" << endl;
                yardBlockReported = true;
            }
            yardBlocked = true;
            continue;
        }

        int craneIndex = findBestCrane(*container);
        if (craneIndex == -1) {
            continue;
        }
        startJob(*job, cranes[craneIndex], *container);
    }

    if (!yardBlocked) {
        yardBlockReported = false;
    }
}

void PortController::startJob(HandlingJob& job, Crane& crane, Container& container) {
    if (!job.assignCrane(crane.getId(), crane.getHandlingMinutes())) {
        return;
    }
    if (job.getType() == JOB_UNLOAD) {
        // Reserve the yard slot now so it cannot be taken mid-lift
        if (!yard.storeContainer(container)) {
            return;
        }
    } else {
        container.setStatus(ASSIGNED_TO_SHIP);
    }
    crane.assignJob(job.getId());
    job.markStarted(currentTime.getMinutes());
}

void PortController::finishJob(HandlingJob& job, Crane& crane) {
    Container* container = registry.findContainer(job.getContainerId());
    crane.completeJob();
    job.markCompleted(currentTime.getMinutes());

    if (job.getType() == JOB_UNLOAD) {
        container->setStatus(IN_YARD);
        importQueue.addContainer(container->getId());
    } else {
        yard.retrieveContainer(container->getId());
        container->setStatus(ON_SHIP);
    }
    statistics.recordContainerHandled(job.getType());
}

void PortController::processCraneEvents() {
    for (int i = 0; i < craneCount; i++) {
        Crane& crane = cranes[i];

        if (crane.isRepairDue(currentTime.getMinutes())) {
            crane.repair();
            logTime();
            cout << "Crane " << crane.getId() << " repaired and back in service" << endl;
        }

        if (!crane.isBreakdownDue(currentTime.getMinutes())) {
            continue;
        }
        // 0 = use the repair time given to scheduleBreakdown()
        int interruptedId = crane.setOutOfService(currentTime.getMinutes(), 0);
        int repairAt = crane.getBackInServiceTime();
        logTime();
        cout << "BREAKDOWN: crane " << crane.getId() << " out of service until "
             << PortTime(repairAt) << endl;

        HandlingJob* job = scheduler.findJob(interruptedId);
        statistics.recordBreakdown(job != nullptr);
        if (job == nullptr) {
            continue;
        }

        // Undo the partial work so the job can be safely restarted
        Container* container = registry.findContainer(job->getContainerId());
        if (job->getType() == JOB_UNLOAD) {
            yard.retrieveContainer(container->getId());
        } else {
            container->setStatus(IN_YARD);
        }
        job->interrupt();
        logTime();
        cout << "  Job #" << job->getId() << " (" << HandlingJob::typeToString(job->getType())
             << " container " << container->getId() << ") returned to the queue" << endl;
    }
}

void PortController::progressJobs() {
    for (int i = 0; i < craneCount; i++) {
        Crane& crane = cranes[i];
        if (crane.getStatus() != CRANE_BUSY) {
            continue;
        }
        HandlingJob* job = scheduler.findJob(crane.getCurrentJobId());
        if (job != nullptr && currentTime.getMinutes() >= job->getFinishTime()) {
            finishJob(*job, crane);
        }
    }
}

void PortController::departShip(Ship& ship) {
    Berth* berth = findBerth(ship.getBerthId());
    if (!ship.markDeparted(currentTime.getMinutes())) {
        return;
    }
    if (berth != nullptr) {
        berth->releaseShip();
    }
    statistics.recordShipDeparture(ship.getTurnaroundTime(), ship.getWaitingTime(),
                                   ship.getDelay());
    logTime();
    cout << ship.getName() << " departed (turnaround " << ship.getTurnaroundTime()
         << " min)";
    if (ship.isLate()) {
        cout << " -- LATE by " << ship.getDelay() << " min";
    }
    cout << endl;
}

// A ship leaves as soon as all of its jobs are finished
void PortController::processDepartures() {
    for (int i = 0; i < shipCount; i++) {
        Ship& ship = ships[i];
        if (ship.getStatus() == SHIP_OPERATING &&
            scheduler.countUnfinishedJobs(ship.getId()) == 0) {
            ship.markReadyToDepart();
            departShip(ship);
        }
    }
}

// Every truckInterval minutes one imported container leaves by road
void PortController::dispatchTrucks() {
    if (importQueue.isEmpty() || currentTime.getMinutes() % truckInterval != 0) {
        return;
    }
    int containerId = importQueue[0];
    Container* container = registry.findContainer(containerId);
    if (container != nullptr && yard.retrieveContainer(containerId)) {
        container->setStatus(DELIVERED);
        statistics.recordDispatch();
    }
    importQueue.removeContainer(containerId);
}

void PortController::recordUtilization() {
    int busy = 0;
    int inService = 0;
    for (int i = 0; i < craneCount; i++) {
        if (cranes[i].getStatus() == CRANE_BUSY) {
            busy++;
        }
        if (cranes[i].isInService()) {
            inService++;
        }
    }
    int occupied = 0;
    for (int i = 0; i < berthCount; i++) {
        if (!berths[i].isAvailable()) {
            occupied++;
        }
    }
    statistics.recordMinute(busy, inService, occupied, berthCount);
}

// One simulated minute
void PortController::processNextEvent() {
    processCraneEvents();
    progressJobs();
    processDepartures();
    processArrivals();
    allocateBerths();
    createHandlingJobs();
    dispatchTrucks();
    assignCraneJobs();
    recordUtilization();
    ++currentTime;
}

void PortController::updateSimulation(int duration) {
    for (int i = 0; i < duration; i++) {
        processNextEvent();
    }
}

// Runs until no more progress is possible. Returns minutes simulated.
int PortController::runUntilComplete(int maxDuration) {
    int minutes = 0;
    while (!isFinished() && minutes < maxDuration) {
        processNextEvent();
        minutes++;
    }
    if (!isFinished()) {
        logTime();
        cout << "Simulation stopped after the " << maxDuration
             << " minute limit" << endl;
    }
    return minutes;
}

// Finished when every ship has departed, except ships that no berth can
// ever accommodate (those would wait forever)
bool PortController::isFinished() const {
    for (int i = 0; i < shipCount; i++) {
        int status = ships[i].getStatus();
        if (status == SHIP_DEPARTED) {
            continue;
        }
        if (status == SHIP_WAITING && !anyBerthFits(ships[i])) {
            continue;
        }
        return false;
    }
    return true;
}

const PortTime& PortController::getCurrentTime() const {
    return currentTime;
}

// ---------------------------------------------------------------------------
// Reports
// ---------------------------------------------------------------------------

void PortController::displayPortStatus() const {
    cout << endl << "========== Port status at " << currentTime << " ==========" << endl;
    cout << "Ships:" << endl;
    for (int i = 0; i < shipCount; i++) {
        const Ship& ship = ships[i];
        cout << "  " << left << setw(18) << ship.getName() << right
             << Ship::statusToString(ship.getStatus());
        if (ship.getBerthId() != -1) {
            cout << " at berth " << ship.getBerthId();
        }
        int unfinished = scheduler.countUnfinishedJobs(ship.getId());
        if (unfinished > 0) {
            cout << ", " << unfinished << " jobs left";
        }
        cout << endl;
    }
    cout << "Berths:" << endl;
    for (int i = 0; i < berthCount; i++) {
        cout << "  ";
        berths[i].display();
    }
    cout << "Cranes:" << endl;
    for (int i = 0; i < craneCount; i++) {
        cout << "  ";
        cranes[i].display();
    }
    yard.displayYard();
    cout << "Containers waiting for trucks: " << importQueue.getCount() << endl;
    cout << "Pending jobs: " << scheduler.countPendingJobs() << endl;
    cout << "==========================================" << endl << endl;
}

void PortController::generateFinalReport() const {
    cout << endl << "================ FINAL REPORT ================" << endl;
    cout << left << setw(4) << "ID" << setw(18) << "Ship" << right
         << setw(8) << "Arrived" << setw(8) << "Berthed" << setw(9) << "Departed"
         << setw(9) << "Deadline" << setw(6) << "Wait" << setw(11) << "Turnaround"
         << "  Result" << endl;

    for (int i = 0; i < shipCount; i++) {
        const Ship& ship = ships[i];
        cout << left << setw(4) << ship.getId() << setw(18) << ship.getName() << right;

        if (ship.getActualArrivalTime() >= 0) {
            cout << "   " << PortTime(ship.getActualArrivalTime());
        } else {
            cout << setw(8) << "-";
        }
        if (ship.getBerthedTime() >= 0) {
            cout << "   " << PortTime(ship.getBerthedTime());
        } else {
            cout << setw(8) << "-";
        }
        if (ship.getStatus() == SHIP_DEPARTED) {
            cout << "    " << PortTime(ship.getActualDepartureTime());
        } else {
            cout << setw(9) << "-";
        }
        cout << "    " << PortTime(ship.getDeadline());

        if (ship.getWaitingTime() >= 0) {
            cout << setw(6) << ship.getWaitingTime();
        } else {
            cout << setw(6) << "-";
        }
        if (ship.getStatus() == SHIP_DEPARTED) {
            cout << setw(11) << ship.getTurnaroundTime();
        } else {
            cout << setw(11) << "-";
        }

        if (ship.getStatus() != SHIP_DEPARTED) {
            cout << "  NOT DEPARTED (" << Ship::statusToString(ship.getStatus()) << ")";
        } else if (ship.isLate()) {
            cout << "  LATE +" << ship.getDelay() << " min";
        } else {
            cout << "  ON TIME";
        }
        cout << endl;
    }
    cout << "(times in minutes; ships not departed are excluded from averages)" << endl;
    cout << endl;
    statistics.generateReport();
    cout << endl;
    yard.displayYard();
    cout << "==============================================" << endl;
}
