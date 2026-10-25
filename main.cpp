#include <iostream>
#include "Container.h"
#include "Manifest.h"
#include "Ship.h"
#include "HandlingJob.h"
#include "PortTime.h"
#include "PortController.h"

using namespace std;

// Creates containers for a ship, registers them and adds them to its manifest.
// The first 'reefers' containers are refrigerated, the next 'hazardous' are
// hazardous, and the rest are standard.
void addImportCargo(PortController& port, const Ship& ship, int count,
                    int reefers, int hazardous, const char* owner, const char* origin) {
    for (int i = 0; i < count; i++) {
        int type = TYPE_STANDARD;
        const char* contents = "General cargo";
        if (i < reefers) {
            type = TYPE_REFRIGERATED;
            contents = "Frozen food";
        } else if (i < reefers + hazardous) {
            type = TYPE_HAZARDOUS;
            contents = "Industrial chemicals";
        }
        double weight = 10.0 + (i * 7) % 16;      // 10-25 t
        Container c(weight, type, 100, owner, contents, origin, "Chennai", "SL-0000");
        port.registerContainer(c);
        port.assignImportCargo(ship.getId(), c.getId());
    }
}

// Creates containers waiting in the yard to be loaded onto a ship
void addExportCargo(PortController& port, const Ship& ship, int count,
                    const char* owner, const char* destination) {
    for (int i = 0; i < count; i++) {
        Container c(12.0 + i * 3, TYPE_STANDARD, 200, owner, "Textiles",
                    "Chennai", destination, "SL-1000");
        port.registerContainer(c);
        port.assignExportCargo(ship.getId(), c.getId());
    }
}

void runComponentChecks() {
    cout << "===== Component checks =====" << endl << endl;

    // Manifest deep copy (Rule of Three)
    Manifest original;
    original.addContainer(1);
    original.addContainer(2);
    original.addContainer(2);            // duplicate, rejected
    Manifest copy = original;            // copy constructor
    copy.addContainer(3);
    Manifest assigned;
    assigned = copy;                     // assignment operator
    assigned.removeContainer(1);
    cout << "original: ";
    original.display();
    cout << "copy:     ";
    copy.display();
    cout << "assigned: ";
    assigned.display();
    cout << "copy[2] = " << copy[2] << endl << endl;

    // PortTime operators
    PortTime t1(8, 0);
    PortTime t2(10, 30);
    PortTime t3 = t1;
    t3 += 45;
    ++t3;
    cout << "t1 = " << t1 << ", t2 = " << t2 << ", t1 + 46 min = " << t3 << endl;
    cout << "t2 - t1 = " << (t2 - t1) << " min, t1 < t2: "
         << (t1 < t2 ? "true" : "false") << endl << endl;

    // HandlingJob ordering: emergency first, then deadline, then priority
    HandlingJob normalEarly(1, 1, JOB_UNLOAD, PortTime(11, 0).getMinutes());
    HandlingJob normalLate(1, 2, JOB_UNLOAD, PortTime(12, 0).getMinutes());
    HandlingJob emergencyLate(2, 3, JOB_UNLOAD, PortTime(14, 0).getMinutes(),
                              PRIORITY_EMERGENCY);
    cout << "early-deadline job before late one?   "
         << (normalEarly < normalLate ? "yes" : "no") << endl;
    cout << "emergency job before early-deadline?  "
         << (emergencyLate < normalEarly ? "yes" : "no") << endl << endl;

    // Ship state validation
    Ship test;                           // placeholder ship, does not use an ID
    cout << "Trying to depart a ship that has not arrived:" << endl << "  ";
    test.markDeparted(10);

    // Container state validation
    Container box(15.0, TYPE_STANDARD, 1);
    cout << "Trying ON_SHIP -> DELIVERED:" << endl << "  ";
    box.setStatus(DELIVERED);
    cout << endl;
}

int main() {
    runComponentChecks();

    cout << "===== Smart Port Simulation =====" << endl << endl;

    // Port starts at 07:55 with a 30-slot yard, 2 berths and 2 cranes
    PortController port(PortTime(7, 55).getMinutes(), 30, 2, 2);

    port.addBerth(300, 14.0);           // berth 1: deep-water
    port.addBerth(220, 11.0);           // berth 2: smaller
    port.addCrane(35.0, 6);             // crane 1: heavy, fast
    port.addCrane(28.0, 8);             // crane 2: lighter, slower

    port.addYardSlots(20, TYPE_STANDARD, 35.0);
    port.addYardSlots(4, TYPE_REFRIGERATED, 30.0);
    port.addYardSlots(3, TYPE_HAZARDOUS, 30.0);

    port.setTruckInterval(10);          // one container leaves by road every 10 min
    port.setUrgencyWindow(60);

    Ship shipA("MV Ocean Pearl", 250, 12.0, PortTime(8, 0).getMinutes(), PortTime(12, 0).getMinutes());
    Ship shipB("MV Coral Star", 180, 9.5, PortTime(8, 10).getMinutes(), PortTime(11, 30).getMinutes());
    Ship shipC("MV Blue Horizon", 280, 13.5, PortTime(8, 20).getMinutes(), PortTime(14, 0).getMinutes());
    Ship shipD("MV Titan Giant", 380, 16.0, PortTime(9, 0).getMinutes(), PortTime(15, 0).getMinutes());

    port.registerShip(shipA);
    port.registerShip(shipB);
    port.registerShip(shipC);
    port.registerShip(shipD);
    port.registerShip(shipA);           // duplicate ship, rejected

    addImportCargo(port, shipA, 20, 3, 1, "Maersk Line", "Singapore");
    addImportCargo(port, shipB, 12, 2, 0, "MSC Shipping", "Colombo");
    addImportCargo(port, shipC, 30, 4, 2, "CMA CGM", "Dubai");
    addImportCargo(port, shipD, 3, 0, 0, "Evergreen", "Shanghai");

    addExportCargo(port, shipA, 3, "Maersk Line", "Singapore");
    addExportCargo(port, shipB, 2, "MSC Shipping", "Colombo");
    addExportCargo(port, shipC, 4, "CMA CGM", "Rotterdam");

    // A 32 t container: crane 2 (28 t limit) cannot lift it, crane 1 can
    Container heavy(32.0, TYPE_STANDARD, 100, "CMA CGM", "Machinery parts",
                    "Dubai", "Chennai", "SL-9001");
    port.registerContainer(heavy);
    port.assignImportCargo(shipC.getId(), heavy.getId());

    // A 42 t container that no crane in the port can lift
    Container oversize(42.0, TYPE_STANDARD, 100, "Maersk Line", "Transformer",
                       "Singapore", "Chennai", "SL-9002");
    port.registerContainer(oversize);
    port.assignImportCargo(shipA.getId(), oversize.getId());

    cout << endl << "--- Invalid input checks ---" << endl;
    port.registerContainer(heavy);                      // duplicate container ID
    port.assignImportCargo(shipB.getId(), 99999);       // unknown container ID
    port.assignImportCargo(shipB.getId(), heavy.getId()); // already on ship C
    cout << endl;

    // Crane 1 breaks down at 09:00 for 40 minutes
    port.scheduleCraneBreakdown(1, PortTime(9, 0).getMinutes(), 40);

    cout << "--- Simulation log ---" << endl;
    port.updateSimulation(PortTime(8, 30) - port.getCurrentTime());

    // Ship B is still working, so this request must be refused
    port.requestDeparture(shipB.getId());

    port.updateSimulation(PortTime(9, 15) - port.getCurrentTime());
    port.displayPortStatus();

    port.runUntilComplete(24 * MINUTES_PER_HOUR);
    port.generateFinalReport();

    return 0;
}
