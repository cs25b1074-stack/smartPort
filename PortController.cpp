#include "PortController.h"
#include <iostream>
#include <iomanip>
#include <cstring>

using namespace std;

const int INITIAL_CAPACITY = 4;

PortController::PortController()
    : ships(nullptr), shipCount(0), shipCapacity(0),
      berths(nullptr), berthCount(0), berthCapacity(0),
      cranes(nullptr), craneCount(0), craneCapacity(0),
      currentTime(0), startTime(0), started(false),
      truckInterval(DEFAULT_TRUCK_INTERVAL), yardBlockReported(false) {
    lastError[0] = '\0';
    allocateArrays();
}

PortController::~PortController() {
    delete[] ships;
    delete[] berths;
    delete[] cranes;
}

void PortController::allocateArrays() {
    shipCapacity = INITIAL_CAPACITY;
    berthCapacity = INITIAL_CAPACITY;
    craneCapacity = INITIAL_CAPACITY;
    ships = new Ship[shipCapacity];
    berths = new Berth[berthCapacity];
    cranes = new Crane[craneCapacity];
}

void PortController::setError(const char* message) {
    strncpy(lastError, message, ERROR_SIZE - 1);
    lastError[ERROR_SIZE - 1] = '\0';
}

// ---------------------------------------------------------------------------
// Growing arrays
// ---------------------------------------------------------------------------

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

void PortController::growBerths() {
    int newCapacity = berthCapacity * 2;
    Berth* newBerths = new Berth[newCapacity];
    for (int i = 0; i < berthCount; i++) {
        newBerths[i] = berths[i];
    }
    delete[] berths;
    berths = newBerths;
    berthCapacity = newCapacity;
}

void PortController::growCranes() {
    int newCapacity = craneCapacity * 2;
    Crane* newCranes = new Crane[newCapacity];
    for (int i = 0; i < craneCount; i++) {
        newCranes[i] = cranes[i];
    }
    delete[] cranes;
    cranes = newCranes;
    craneCapacity = newCapacity;
}

// ---------------------------------------------------------------------------
// Setup
// ---------------------------------------------------------------------------

bool PortController::registerShip(const Ship& ship) {
    if (!ship.isValid()) {
        setError("the ship has invalid values");
        return false;
    }
    if (findShipIndex(ship.getId()) != -1) {
        setError("this ship is already registered");
        return false;
    }
    if (ship.getStatus() != Ship::SCHEDULED) {
        setError("only ships that have not arrived yet can be registered");
        return false;
    }
    if (shipCount == shipCapacity) {
        growShips();
    }
    ships[shipCount++] = ship;
    return true;
}

bool PortController::registerBerth(const Berth& berth) {
    if (!berth.isValid()) {
        setError("the berth has invalid values");
        return false;
    }
    for (int i = 0; i < berthCount; i++) {
        if (berths[i].getId() == berth.getId()) {
            setError("this berth is already registered");
            return false;
        }
    }
    if (berthCount == berthCapacity) {
        growBerths();
    }
    berths[berthCount++] = berth;
    return true;
}

bool PortController::registerCrane(const Crane& crane) {
    if (!crane.isValid()) {
        setError("the crane has invalid values");
        return false;
    }
    if (findCrane(crane.getId()) != nullptr) {
        setError("this crane is already registered");
        return false;
    }
    if (craneCount == craneCapacity) {
        growCranes();
    }
    cranes[craneCount++] = crane;
    return true;
}

// The yard can only be resized while it is empty
bool PortController::setupYard(int standard, int reefer, int hazard, double maxWeight) {
    if (yard.isConfigured() && yard.getOccupiedSlots() > 0) {
        setError("the yard already holds containers and cannot be resized");
        return false;
    }
    if (!yard.setup(standard, reefer, hazard, maxWeight)) {
        setError("invalid slot counts or slot weight limit");
        return false;
    }
    return true;
}

bool PortController::setTruckInterval(int minutes) {
    if (minutes < MIN_TRUCK_INTERVAL || minutes > MAX_TRUCK_INTERVAL) {
        setError("truck interval is out of range");
        return false;
    }
    truckInterval = minutes;
    return true;
}

// Jobs are created when a ship berths, so cargo can only be added before that
bool PortController::canStillTakeCargo(const Ship& ship) const {
    return ship.getStatus() == Ship::SCHEDULED || ship.getStatus() == Ship::WAITING;
}

// Checks everything first, so nothing needs to be undone on failure:
// 1. the ship exists and the container is valid and new
// 2. the container is added to the registry
// 3. its ID is added to the ship's manifest
bool PortController::addContainerToShip(int shipId, const Container& container) {
    int index = findShipIndex(shipId);
    if (index == -1) {
        setError("unknown ship ID");
        return false;
    }
    if (!container.isValid()) {
        setError("the container has invalid values");
        return false;
    }
    if (registry.exists(container.getId())) {
        setError("a container with this ID is already registered");
        return false;
    }
    if (container.getStatus() != Container::ON_SHIP) {
        setError("the container is not aboard a ship");
        return false;
    }
    if (!canStillTakeCargo(ships[index])) {
        setError("the ship is already berthed; cargo can no longer be added");
        return false;
    }
    if (!registry.addContainer(container)) {
        setError("the registry rejected the container");
        return false;
    }
    return ships[index].addContainer(container.getId());
}

// An export container is delivered to the yard by road and loaded onto the
// ship later. New containers start as ON_SHIP, so it is moved into the yard here.
bool PortController::addExportContainer(int shipId, const Container& container) {
    int index = findShipIndex(shipId);
    if (index == -1) {
        setError("unknown ship ID");
        return false;
    }
    if (!container.isValid()) {
        setError("the container has invalid values");
        return false;
    }
    if (registry.exists(container.getId())) {
        setError("a container with this ID is already registered");
        return false;
    }
    if (!canStillTakeCargo(ships[index])) {
        setError("the ship is already berthed; cargo can no longer be added");
        return false;
    }
    if (!yard.isConfigured() || !yard.hasSpaceFor(container)) {
        setError("no free compatible yard slot for this container");
        return false;
    }

    registry.addContainer(container);
    Container* stored = registry.findContainer(container.getId());
    yard.storeContainer(*stored);
    stored->setStatus(Container::IN_YARD);
    return ships[index].addLoadContainer(container.getId());
}

bool PortController::scheduleCraneBreakdown(int craneId, int time, int repairMinutes) {
    Crane* crane = findCrane(craneId);
    if (crane == nullptr) {
        setError("unknown crane ID");
        return false;
    }
    if (!crane->scheduleBreakdown(time, repairMinutes)) {
        setError("invalid breakdown time or repair duration");
        return false;
    }
    return true;
}

// Removes everything so a new scenario can be entered. Uses the
// assignment operators of the member objects to reset them.
void PortController::clearScenario() {
    delete[] ships;
    delete[] berths;
    delete[] cranes;
    shipCount = 0;
    berthCount = 0;
    craneCount = 0;
    allocateArrays();

    registry = ContainerRegistry();
    yard = ContainerYard();
    scheduler = PortScheduler();
    statistics = PortStatistics();
    importQueue = Manifest();
    currentTime = PortTime(0);
    startTime = PortTime(0);
    started = false;
    truckInterval = DEFAULT_TRUCK_INTERVAL;
    yardBlockReported = false;
    lastError[0] = '\0';
}

// Manual departure request; rejected while handling work is unfinished
bool PortController::requestDeparture(int shipId) {
    int index = findShipIndex(shipId);
    if (index == -1) {
        setError("unknown ship ID");
        return false;
    }
    Ship& ship = ships[index];
    if (scheduler.countUnfinishedJobs(shipId) > 0) {
        setError("the ship still has unfinished handling jobs");
        return false;
    }
    if (ship.getStatus() == Ship::OPERATING) {
        ship.markReadyToDepart();
    }
    if (ship.getStatus() != Ship::READY_TO_DEPART) {
        setError("the ship is not in a state where it can depart");
        return false;
    }
    departShip(ship);
    return true;
}

// ---------------------------------------------------------------------------
// Read-only information
// ---------------------------------------------------------------------------

const char* PortController::getLastError() const {
    return lastError;
}

int PortController::getShipCount() const {
    return shipCount;
}

int PortController::getBerthCount() const {
    return berthCount;
}

int PortController::getCraneCount() const {
    return craneCount;
}

bool PortController::isYardConfigured() const {
    return yard.isConfigured();
}

bool PortController::isReadyToRun() const {
    return yard.isConfigured() && berthCount > 0 && craneCount > 0 && shipCount > 0;
}

bool PortController::hasStarted() const {
    return started;
}

int PortController::getShipIdAt(int index) const {
    if (index < 0 || index >= shipCount) {
        return -1;
    }
    return ships[index].getId();
}

int PortController::getEarliestArrival() const {
    int earliest = -1;
    for (int i = 0; i < shipCount; i++) {
        if (earliest == -1 || ships[i].getArrivalTime() < earliest) {
            earliest = ships[i].getArrivalTime();
        }
    }
    return earliest;
}

int PortController::getTruckInterval() const {
    return truckInterval;
}

const PortTime& PortController::getCurrentTime() const {
    return currentTime;
}

void PortController::displayShipList() const {
    for (int i = 0; i < shipCount; i++) {
        cout << "  " << (i + 1) << ". " << ships[i].getName()
             << " (ID " << ships[i].getId() << ", "
             << Ship::statusToString(ships[i].getStatus()) << ", "
             << ships[i].getManifest().getCount() << " to unload, "
             << ships[i].getLoadingList().getCount() << " to load)" << endl;
    }
}

// ---------------------------------------------------------------------------
// Lookup helpers
// ---------------------------------------------------------------------------

int PortController::findShipIndex(int shipId) const {
    for (int i = 0; i < shipCount; i++) {
        if (ships[i].getId() == shipId) {
            return i;
        }
    }
    return -1;
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

// Fastest available crane that can lift the container. Returns an index or -1.
int PortController::findBestCrane(const Container& container) const {
    int best = -1;
    for (int i = 0; i < craneCount; i++) {
        if (cranes[i].isAvailable() && cranes[i].canHandle(container) &&
            (best == -1 || cranes[i].getHandlingTime() < cranes[best].getHandlingTime())) {
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
        if (ship.getStatus() != Ship::SCHEDULED ||
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
    while (true) {
        int bestShip = -1;
        for (int i = 0; i < shipCount; i++) {
            if (ships[i].getStatus() == Ship::WAITING && findBestBerth(ships[i]) != -1 &&
                (bestShip == -1 || ships[i].getDeadline() < ships[bestShip].getDeadline())) {
                bestShip = i;
            }
        }
        if (bestShip == -1) {
            return;
        }

        Ship& ship = ships[bestShip];
        Berth& berth = berths[findBestBerth(ship)];
        if (!berth.assignShip(ship.getId()) ||
            !ship.markBerthed(berth.getId(), currentTime.getMinutes())) {
            return;
        }
        statistics.recordWaitingTime(ship.getWaitingTime());
        logTime();
        cout << ship.getName() << " berthed at berth " << berth.getId();
        if (ship.getWaitingTime() > 0) {
            cout << " after waiting " << ship.getWaitingTime() << " min";
        }
        cout << endl;
    }
}

// Creates one job per container for every newly berthed ship
void PortController::createHandlingJobs() {
    for (int i = 0; i < shipCount; i++) {
        Ship& ship = ships[i];
        if (ship.getStatus() != Ship::BERTHED) {
            continue;
        }

        int created = 0;
        for (int pass = 0; pass < 2; pass++) {
            int jobType = HandlingJob::UNLOAD;
            const Manifest* list = &ship.getManifest();
            if (pass == 1) {
                jobType = HandlingJob::LOAD;
                list = &ship.getLoadingList();
            }

            for (int k = 0; k < list->getCount(); k++) {
                int containerId = (*list)[k];
                const Container* container = registry.findContainer(containerId);
                if (container == nullptr) {
                    logTime();
                    cout << "  Skipping unknown container " << containerId << endl;
                    continue;
                }

                // Estimate with the fastest crane that can lift it; the real
                // duration is set when a crane is assigned
                int estimate = 0;
                for (int c = 0; c < craneCount; c++) {
                    if (cranes[c].canHandle(*container)) {
                        if (estimate == 0 || cranes[c].getHandlingTime() < estimate) {
                            estimate = cranes[c].getHandlingTime();
                        }
                    } else {
                        logTime();
                        cout << "  Crane " << cranes[c].getId() << " cannot lift container "
                             << containerId << " (" << container->getWeight() << " t > "
                             << cranes[c].getMaximumLiftWeight() << " t limit)" << endl;
                    }
                }
                bool liftable = (estimate > 0);
                bool storable = (jobType == HandlingJob::LOAD) || yard.supports(*container);

                HandlingJob job(ship.getId(), containerId, jobType,
                                HandlingJob::NORMAL_PRIORITY, ship.getDeadline(),
                                liftable ? estimate : 1);
                if (!liftable || !storable) {
                    job.cancel();
                    logTime();
                    cout << "  Job #" << job.getId() << " cancelled: "
                         << (!liftable ? "no crane can lift" : "no yard slot can ever hold")
                         << " container " << containerId << endl;
                }
                if (scheduler.addJob(job)) {
                    created++;
                }
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
    int raised = scheduler.prioritizeJobs(currentTime.getMinutes(), URGENCY_WINDOW);
    if (raised > 0) {
        logTime();
        cout << raised << " jobs raised to URGENT (deadline within "
             << URGENCY_WINDOW << " min)" << endl;
    }

    bool yardBlocked = false;
    for (int i = 0; i < scheduler.getJobCount(); i++) {
        HandlingJob* job = scheduler.getJob(i);
        if (!job->isPending()) {
            continue;
        }

        Container* container = registry.findContainer(job->getContainerId());
        if (container == nullptr) {
            continue;
        }

        if (job->getJobType() == HandlingJob::UNLOAD && !yard.hasSpaceFor(*container)) {
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
    if (job.getJobType() == HandlingJob::UNLOAD) {
        // Reserve the yard slot now so it cannot be taken mid-lift
        if (!yard.storeContainer(container)) {
            return;
        }
    } else {
        container.setStatus(Container::ASSIGNED_TO_SHIP);
    }
    job.assignCrane(crane.getId(), crane.getHandlingTime());
    crane.assignJob(job.getId());
    job.markStarted(currentTime.getMinutes());
}

void PortController::finishJob(HandlingJob& job, Crane& crane) {
    Container* container = registry.findContainer(job.getContainerId());
    crane.completeJob(job.getDuration());
    job.markCompleted(currentTime.getMinutes());

    if (job.getJobType() == HandlingJob::UNLOAD) {
        container->setStatus(Container::IN_YARD);
        importQueue.addContainer(container->getId());
    } else {
        yard.retrieveContainer(container->getId());
        container->setStatus(Container::ON_SHIP);
    }
    statistics.recordContainerHandled(job.getJobType());
}

// Takes a crane out of service now and safely returns its job to the queue
bool PortController::breakDownCrane(int craneId, int repairMinutes) {
    Crane* crane = findCrane(craneId);
    if (crane == nullptr || !crane->isInService() || !Crane::isValidRepairTime(repairMinutes)) {
        setError("unknown crane, crane already out of service, or bad repair time");
        return false;
    }

    int interruptedId = (crane->getStatus() == Crane::BUSY) ? crane->getCurrentJobId() : -1;
    crane->setOutOfService(currentTime.getMinutes() + repairMinutes);
    logTime();
    cout << "BREAKDOWN: crane " << crane->getId() << " out of service until "
         << PortTime(crane->getBackInServiceTime()) << endl;

    HandlingJob* job = scheduler.findJob(interruptedId);
    statistics.recordBreakdown(job != nullptr);
    if (job == nullptr) {
        return true;
    }

    // Undo the partial work so the job can be safely restarted
    Container* container = registry.findContainer(job->getContainerId());
    if (job->getJobType() == HandlingJob::UNLOAD) {
        yard.retrieveContainer(container->getId());
    } else {
        container->setStatus(Container::IN_YARD);
    }
    job->interrupt();
    logTime();
    cout << "  Job #" << job->getId() << " (" << HandlingJob::typeToString(job->getJobType())
         << " container " << container->getId() << ") returned to the queue" << endl;
    return true;
}

void PortController::processCraneEvents() {
    for (int i = 0; i < craneCount; i++) {
        Crane& crane = cranes[i];
        if (crane.isRepairDue(currentTime.getMinutes())) {
            crane.repair();
            logTime();
            cout << "Crane " << crane.getId() << " repaired and back in service" << endl;
        }
        if (crane.isBreakdownDue(currentTime.getMinutes())) {
            breakDownCrane(crane.getId(), crane.getRepairMinutes());
        }
    }
}

void PortController::progressJobs() {
    for (int i = 0; i < craneCount; i++) {
        Crane& crane = cranes[i];
        if (crane.getStatus() != Crane::BUSY) {
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
    statistics.recordShipDeparture(ship.getTurnaroundTime(), ship.isLate());
    statistics.recordDelay(ship.getDelay());
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
        if (ship.getStatus() == Ship::OPERATING &&
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
        container->setStatus(Container::DELIVERED);
        statistics.recordDispatch();
    }
    importQueue.removeContainer(containerId);
}

void PortController::recordUtilization() {
    int busy = 0;
    int inService = 0;
    for (int i = 0; i < craneCount; i++) {
        if (cranes[i].getStatus() == Crane::BUSY) {
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
    statistics.recordCraneTime(busy, inService);
    statistics.recordBerthTime(occupied, berthCount);
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

// Runs until every ship is done or the end time is reached. The first run
// starts the clock at the earliest ship arrival; later runs continue from
// where the previous one stopped.
void PortController::runSimulation(int endTimeMinutes) {
    if (!isReadyToRun()) {
        setError("the scenario is not complete");
        return;
    }
    if (!started) {
        currentTime = PortTime(getEarliestArrival());
        startTime = currentTime;
        started = true;
    }

    PortTime endTime(endTimeMinutes);
    while (currentTime < endTime && !isFinished()) {
        processNextEvent();
    }

    logTime();
    if (isFinished()) {
        cout << "Simulation complete: no further work is possible" << endl;
    } else {
        cout << "Simulation paused at the chosen end time" << endl;
    }
}

// Finished when every ship has departed, except ships that no berth can
// ever accommodate (those would wait forever)
bool PortController::isFinished() const {
    for (int i = 0; i < shipCount; i++) {
        int status = ships[i].getStatus();
        if (status == Ship::DEPARTED) {
            continue;
        }
        if (status == Ship::WAITING && !anyBerthFits(ships[i])) {
            continue;
        }
        return false;
    }
    return true;
}

// ---------------------------------------------------------------------------
// Reports
// ---------------------------------------------------------------------------

void PortController::displayPortStatus() const {
    cout << endl << "========== Port status";
    if (started) {
        cout << " at " << currentTime;
    } else {
        cout << " (simulation not started)";
    }
    cout << " ==========" << endl;

    cout << "Ships:" << (shipCount == 0 ? " none" : "") << endl;
    for (int i = 0; i < shipCount; i++) {
        const Ship& ship = ships[i];
        cout << "  " << left << setw(20) << ship.getName() << right
             << Ship::statusToString(ship.getStatus());
        if (ship.getBerthId() != -1) {
            cout << " at berth " << ship.getBerthId();
        }
        cout << " (arrives " << PortTime(ship.getArrivalTime()) << ", deadline "
             << PortTime(ship.getDeadline()) << ", " << ship.getManifest().getCount()
             << " to unload, " << ship.getLoadingList().getCount() << " to load)";
        int unfinished = scheduler.countUnfinishedJobs(ship.getId());
        if (unfinished > 0) {
            cout << ", " << unfinished << " jobs left";
        }
        cout << endl;
    }
    cout << "Berths:" << (berthCount == 0 ? " none" : "") << endl;
    for (int i = 0; i < berthCount; i++) {
        cout << "  ";
        berths[i].display();
    }
    cout << "Cranes:" << (craneCount == 0 ? " none" : "") << endl;
    for (int i = 0; i < craneCount; i++) {
        cout << "  ";
        cranes[i].display();
    }
    yard.displayYard();
    cout << "Truck pickup every " << truckInterval << " min, "
         << importQueue.getCount() << " containers waiting" << endl;
    cout << "Pending jobs: " << scheduler.getPendingCount() << endl;
    cout << "==========================================" << endl;
}

void PortController::generateFinalReport() const {
    cout << endl << "================ FINAL REPORT ================" << endl;
    cout << left << setw(4) << "ID" << setw(20) << "Ship" << right
         << setw(8) << "Arrived" << setw(8) << "Berthed" << setw(9) << "Departed"
         << setw(9) << "Deadline" << setw(6) << "Wait" << setw(11) << "Turnaround"
         << "  Result" << endl;

    for (int i = 0; i < shipCount; i++) {
        const Ship& ship = ships[i];
        cout << left << setw(4) << ship.getId() << setw(20) << ship.getName() << right;

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
        if (ship.getStatus() == Ship::DEPARTED) {
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
        if (ship.getStatus() == Ship::DEPARTED) {
            cout << setw(11) << ship.getTurnaroundTime();
        } else {
            cout << setw(11) << "-";
        }

        if (ship.getStatus() != Ship::DEPARTED) {
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
    if (started) {
        cout << "Simulated period: " << startTime << " to " << currentTime
             << " (" << (currentTime - startTime) << " min)" << endl;
    }
    statistics.generateReport();
    cout << endl;
    yard.displayYard();
    cout << "==============================================" << endl;
}
