#include "PortMenu.h"
#include "ConsoleInput.h"
#include <iostream>

using namespace std;

PortMenu::PortMenu(PortController& controller)
    : controller(controller) {
}

void PortMenu::run() {
    const char* options[] = {
        "Set up container yard",
        "Add berth",
        "Add crane",
        "Add ship",
        "Add containers to a ship",
        "Load sample scenario",
        "Show port status",
        "Run simulation",
        "Exit"
    };

    while (true) {
        int choice = ConsoleInput::readMenuChoice("===== SMART PORT SIMULATOR =====", options, 9);
        switch (choice) {
            case 1: setupYardFromInput();     break;
            case 2: addBerthFromInput();      break;
            case 3: addCraneFromInput();      break;
            case 4: addShipFromInput();       break;
            case 5: addContainersFromInput(); break;
            case 6: loadSampleScenario();     break;
            case 7: showStatus();             break;
            case 8: runSimulationFromInput(); break;
            case 9:
                cout << "Goodbye." << endl;
                return;
        }
    }
}

void PortMenu::reportFailure(const char* what) const {
    cout << "  " << what << ": " << controller.getLastError() << "." << endl;
}

// ---------------------------------------------------------------------------
// Setup screens
// ---------------------------------------------------------------------------

void PortMenu::setupYardFromInput() {
    cout << endl << "--- Set up container yard ---" << endl;
    if (controller.isYardConfigured()) {
        cout << "  The yard is already set up. Entering new values replaces it." << endl;
    }

    int standard;
    int reefer;
    int hazard;
    while (true) {
        standard = ConsoleInput::readInt("Standard slots: ", 0, ContainerYard::MAX_TOTAL_SLOTS);
        reefer = ConsoleInput::readInt("Refrigerated slots: ", 0, ContainerYard::MAX_TOTAL_SLOTS);
        hazard = ConsoleInput::readInt("Hazardous slots: ", 0, ContainerYard::MAX_TOTAL_SLOTS);
        if (ContainerYard::isValidSlotCounts(standard, reefer, hazard)) {
            break;
        }
        cout << "  The total must be between 1 and " << ContainerYard::MAX_TOTAL_SLOTS
             << " slots." << endl;
    }
    double maxWeight = ConsoleInput::readDouble("Maximum weight per slot (t): ",
                                                0.1, YardSlot::MAX_WEIGHT);
    int interval = ConsoleInput::readInt("Minutes between truck pickups: ",
                                         PortController::MIN_TRUCK_INTERVAL,
                                         PortController::MAX_TRUCK_INTERVAL);

    if (!controller.setupYard(standard, reefer, hazard, maxWeight)) {
        reportFailure("Yard could not be set up");
        return;
    }
    controller.setTruckInterval(interval);
    cout << "  Yard set up with " << (standard + reefer + hazard) << " slots." << endl;
}

void PortMenu::addBerthFromInput() {
    cout << endl << "--- Add berth ---" << endl;
    double length = ConsoleInput::readDouble("Maximum ship length (m): ",
                                             Berth::MIN_LENGTH, Berth::MAX_LENGTH);
    double depth = ConsoleInput::readDouble("Water depth (m): ",
                                            Berth::MIN_DEPTH, Berth::MAX_DEPTH);

    Berth berth(length, depth);         // level 2 check
    if (!berth.isValid()) {
        cout << "  Berth could not be created. Please check the values." << endl;
        return;
    }
    if (controller.registerBerth(berth)) {
        cout << "  Berth " << berth.getId() << " added." << endl;
    } else {
        reportFailure("Berth not added");
    }
}

void PortMenu::addCraneFromInput() {
    cout << endl << "--- Add crane ---" << endl;
    double lift = ConsoleInput::readDouble("Maximum lift weight (t): ",
                                           Crane::MIN_LIFT_WEIGHT, Crane::MAX_LIFT_WEIGHT);
    int minutes = ConsoleInput::readInt("Minutes per container: ",
                                        Crane::MIN_HANDLING_TIME, Crane::MAX_HANDLING_TIME);

    Crane crane(lift, minutes);         // level 2 check
    if (!crane.isValid()) {
        cout << "  Crane could not be created. Please check the values." << endl;
        return;
    }
    if (!controller.registerCrane(crane)) {
        reportFailure("Crane not added");
        return;
    }
    cout << "  Crane " << crane.getId() << " added." << endl;

    if (ConsoleInput::readYesNo("Schedule a breakdown for this crane? (y/n): ")) {
        int time = ConsoleInput::readTime("Breakdown time (HH:MM): ");
        int repair = ConsoleInput::readInt("Repair time (minutes): ", 1, Crane::MAX_REPAIR_TIME);
        if (controller.scheduleCraneBreakdown(crane.getId(), time, repair)) {
            cout << "  Breakdown scheduled at " << PortTime(time) << " for "
                 << repair << " min." << endl;
        } else {
            reportFailure("Breakdown not scheduled");
        }
    }
}

void PortMenu::addShipFromInput() {
    char name[Ship::MAX_NAME_LENGTH + 1];
    cout << endl << "--- Add ship ---" << endl;
    ConsoleInput::readText("Ship name: ", name, Ship::MAX_NAME_LENGTH + 1);

    double length = ConsoleInput::readDouble("Ship length (m): ",
                                             Ship::MIN_LENGTH, Ship::MAX_LENGTH);
    double depth = ConsoleInput::readDouble("Required water depth (m): ",
                                            Ship::MIN_DEPTH, Ship::MAX_DEPTH);

    int arrival = ConsoleInput::readTime("Arrival time (HH:MM): ");
    int deadline;
    while (true) {
        deadline = ConsoleInput::readTime("Departure deadline (HH:MM): ");
        if (Ship::isValidSchedule(arrival, deadline)) {
            break;
        }
        cout << "  The deadline must be later than the arrival time." << endl;
    }

    Ship ship(name, length, depth, arrival, deadline);   // level 2 check
    if (!ship.isValid()) {
        cout << "  Ship could not be created. Please check the values." << endl;
        return;
    }
    if (controller.registerShip(ship)) {
        cout << "  Ship " << ship.getId() << " (" << name << ") added." << endl;
    } else {
        reportFailure("Ship not added");
    }
}

// Shows the ship list as a menu, so the user never types an ID
int PortMenu::chooseShip() {
    cout << endl << "Select a ship:" << endl;
    controller.displayShipList();
    int choice = ConsoleInput::readInt("Choose an option: ", 1, controller.getShipCount());
    return controller.getShipIdAt(choice - 1);
}

int PortMenu::chooseContainerType() {
    const char* options[] = { "Standard", "Refrigerated", "Hazardous" };
    int choice = ConsoleInput::readMenuChoice("Container type:", options, 3);
    if (choice == 1) {
        return Container::STANDARD;
    }
    if (choice == 2) {
        return Container::REFRIGERATED;
    }
    return Container::HAZARDOUS;
}

void PortMenu::addContainersFromInput() {
    cout << endl << "--- Add containers ---" << endl;
    if (controller.getShipCount() == 0) {
        cout << "  Add a ship first." << endl;
        return;
    }
    int shipId = chooseShip();

    const char* directions[] = {
        "Unload at this port (import, arrives on the ship)",
        "Load onto the ship (export, waiting in the yard)"
    };
    bool exportCargo =
        ConsoleInput::readMenuChoice("Which cargo is this?", directions, 2) == 2;
    if (exportCargo && !controller.isYardConfigured()) {
        cout << "  Set up the container yard first; export containers are stored there." << endl;
        return;
    }

    const char* modes[] = { "One by one", "Several identical containers" };
    int mode = ConsoleInput::readMenuChoice("How do you want to add containers?", modes, 2);
    if (mode == 1) {
        addOneContainer(shipId, exportCargo);
    } else {
        addContainerBatch(shipId, exportCargo);
    }
}

bool PortMenu::addContainer(int shipId, const Container& container, bool exportCargo) {
    if (exportCargo) {
        return controller.addExportContainer(shipId, container);
    }
    return controller.addContainerToShip(shipId, container);
}

void PortMenu::addOneContainer(int shipId, bool exportCargo) {
    do {
        int type = chooseContainerType();
        double weight = ConsoleInput::readDouble("Weight (t): ", 0.1, Container::MAX_WEIGHT);
        int destination = ConsoleInput::readInt("Destination code: ",
                                                Container::MIN_DESTINATION,
                                                Container::MAX_DESTINATION);
        Container c(weight, type, destination);     // level 2 check
        if (!c.isValid()) {
            cout << "  Container could not be created. Please check the values." << endl;
        } else if (addContainer(shipId, c, exportCargo)) {
            cout << "  Container " << c.getId() << " added." << endl;
        } else {
            reportFailure("Container not added");
        }
    } while (ConsoleInput::readYesNo("Add another container? (y/n): "));
}

void PortMenu::addContainerBatch(int shipId, bool exportCargo) {
    int count = ConsoleInput::readInt("How many containers? ", 1, 200);
    int type = chooseContainerType();
    double weight = ConsoleInput::readDouble("Weight of each container (t): ",
                                             0.1, Container::MAX_WEIGHT);
    int destination = ConsoleInput::readInt("Destination code: ",
                                            Container::MIN_DESTINATION,
                                            Container::MAX_DESTINATION);
    int added = 0;
    for (int i = 0; i < count; i++) {
        Container c(weight, type, destination);
        if (addContainer(shipId, c, exportCargo)) {
            added++;
        } else {
            reportFailure("Stopped adding containers");
            break;
        }
    }
    cout << "  " << added << " of " << count << " containers added." << endl;
}

// ---------------------------------------------------------------------------
// Sample scenario: the version 1 test data, built through the same
// constructors and register functions as user data
// ---------------------------------------------------------------------------

void PortMenu::loadSampleScenario() {
    cout << endl << "--- Load sample scenario ---" << endl;
    bool hasData = controller.getShipCount() > 0 || controller.getBerthCount() > 0 ||
                   controller.getCraneCount() > 0 || controller.isYardConfigured();
    if (hasData && !ConsoleInput::readYesNo("This replaces the current scenario. Continue? (y/n): ")) {
        return;
    }
    controller.clearScenario();

    controller.setupYard(20, 4, 3, 35.0);
    controller.setTruckInterval(10);
    controller.registerBerth(Berth(300, 14.0));     // deep-water berth
    controller.registerBerth(Berth(220, 11.0));     // smaller berth

    Crane heavyCrane(34.0, 6);                      // fast, lifts up to 34 t
    Crane lightCrane(28.0, 8);                      // slower, lifts up to 28 t
    controller.registerCrane(heavyCrane);
    controller.registerCrane(lightCrane);
    controller.scheduleCraneBreakdown(heavyCrane.getId(), PortTime(9, 0).getMinutes(), 40);

    Ship shipA("MV Ocean Pearl", 250, 12.0, PortTime(8, 0).getMinutes(), PortTime(12, 0).getMinutes());
    Ship shipB("MV Coral Star", 180, 9.5, PortTime(8, 10).getMinutes(), PortTime(11, 30).getMinutes());
    Ship shipC("MV Blue Horizon", 280, 13.5, PortTime(8, 20).getMinutes(), PortTime(14, 0).getMinutes());
    Ship shipD("MV Titan Giant", 380, 16.0, PortTime(9, 0).getMinutes(), PortTime(15, 0).getMinutes());
    controller.registerShip(shipA);
    controller.registerShip(shipB);
    controller.registerShip(shipC);
    controller.registerShip(shipD);

    // Import cargo: the first containers of each ship are refrigerated,
    // then hazardous, then standard
    const Ship* shipList[] = { &shipA, &shipB, &shipC, &shipD };
    const int counts[] = { 20, 12, 30, 3 };
    const int reefers[] = { 3, 2, 4, 0 };
    const int hazards[] = { 1, 0, 2, 0 };
    const char* owners[] = { "Maersk Line", "MSC Shipping", "CMA CGM", "Evergreen" };
    const char* origins[] = { "Singapore", "Colombo", "Dubai", "Shanghai" };

    for (int s = 0; s < 4; s++) {
        for (int i = 0; i < counts[s]; i++) {
            int type = Container::STANDARD;
            const char* contents = "General cargo";
            if (i < reefers[s]) {
                type = Container::REFRIGERATED;
                contents = "Frozen food";
            } else if (i < reefers[s] + hazards[s]) {
                type = Container::HAZARDOUS;
                contents = "Industrial chemicals";
            }
            double weight = 10.0 + (i * 7) % 16;      // 10-25 t
            Container c(weight, type, 100, owners[s], contents, origins[s], "Chennai", "SL-0000");
            controller.addContainerToShip(shipList[s]->getId(), c);
        }
    }

    // Export cargo waiting in the yard
    const int exports[] = { 3, 2, 4 };
    const char* destinations[] = { "Singapore", "Colombo", "Rotterdam" };
    for (int s = 0; s < 3; s++) {
        for (int i = 0; i < exports[s]; i++) {
            Container c(12.0 + i * 3, Container::STANDARD, 200, owners[s], "Textiles",
                        "Chennai", destinations[s], "SL-1000");
            controller.addExportContainer(shipList[s]->getId(), c);
        }
    }

    // 33 t: only the heavy crane can lift it
    Container heavy(33.0, Container::STANDARD, 100, "CMA CGM", "Machinery parts",
                    "Dubai", "Chennai", "SL-9001");
    controller.addContainerToShip(shipC.getId(), heavy);

    // 34.8 t: a legal container, but heavier than every crane can lift
    Container oversize(34.8, Container::STANDARD, 100, "Maersk Line", "Transformer",
                       "Singapore", "Chennai", "SL-9002");
    controller.addContainerToShip(shipA.getId(), oversize);

    cout << "  Sample scenario loaded: 4 ships, 2 berths, 2 cranes, 27 yard slots." << endl;

    // Show that the validation rejects bad data
    cout << endl << "  Validation checks on the sample data:" << endl;
    Container tooHeavy(42.0, Container::STANDARD, 100);
    cout << "  - 42 t container: " << (tooHeavy.isValid() ? "accepted" : "rejected")
         << " (limit " << Container::MAX_WEIGHT << " t)" << endl;
    Ship noName("", 100, 10, 0, 60);
    cout << "  - Ship with an empty name: " << (noName.isValid() ? "accepted" : "rejected") << endl;
    Berth shallow(200, 0.5);
    cout << "  - Berth with 0.5 m depth: " << (shallow.isValid() ? "accepted" : "rejected") << endl;
    if (!controller.addContainerToShip(shipB.getId(), heavy)) {
        cout << "  - Adding container " << heavy.getId() << " twice: rejected ("
             << controller.getLastError() << ")" << endl;
    }
    Container stray(15.0, Container::STANDARD, 100);
    if (!controller.addContainerToShip(9999, stray)) {
        cout << "  - Adding cargo to unknown ship 9999: rejected ("
             << controller.getLastError() << ")" << endl;
    }
}

// ---------------------------------------------------------------------------
// Status and running
// ---------------------------------------------------------------------------

void PortMenu::showStatus() const {
    controller.displayPortStatus();
}

void PortMenu::runSimulationFromInput() {
    cout << endl << "--- Run simulation ---" << endl;
    if (!controller.isReadyToRun()) {
        cout << "  The scenario is not complete. Still needed:" << endl;
        if (!controller.isYardConfigured()) {
            cout << "  - a container yard (option 1)" << endl;
        }
        if (controller.getBerthCount() == 0) {
            cout << "  - at least one berth (option 2)" << endl;
        }
        if (controller.getCraneCount() == 0) {
            cout << "  - at least one crane (option 3)" << endl;
        }
        if (controller.getShipCount() == 0) {
            cout << "  - at least one ship (option 4)" << endl;
        }
        return;
    }
    if (controller.hasStarted() && controller.isFinished()) {
        cout << "  This scenario has already finished. Load or enter a new one." << endl;
        controller.generateFinalReport();
        return;
    }

    int from = controller.hasStarted() ? controller.getCurrentTime().getMinutes()
                                       : controller.getEarliestArrival();
    cout << "  The simulation will start from " << PortTime(from) << "." << endl;
    int endTime;
    while (true) {
        endTime = ConsoleInput::readTime("Run until (HH:MM): ");
        if (endTime > from) {
            break;
        }
        cout << "  The end time must be later than " << PortTime(from) << "." << endl;
    }

    cout << endl << "--- Simulation log ---" << endl;
    controller.runSimulation(endTime);
    controller.generateFinalReport();
    ConsoleInput::pause();
}
