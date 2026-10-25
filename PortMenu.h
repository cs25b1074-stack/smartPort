#ifndef PORTMENU_H
#define PORTMENU_H

#include "PortController.h"

// Shows the menus, collects values with ConsoleInput, builds port objects
// and passes them to the PortController. Together with ConsoleInput, this
// is the only class that reads from the keyboard.
class PortMenu {
private:
    PortController& controller;     // the menu uses the controller, it does not own it

    void setupYardFromInput();
    void addBerthFromInput();
    void addCraneFromInput();
    void addShipFromInput();
    void addContainersFromInput();
    void addOneContainer(int shipId, bool exportCargo);
    void addContainerBatch(int shipId, bool exportCargo);
    void loadSampleScenario();
    void runSimulationFromInput();
    void showStatus() const;
    int chooseShip();
    int chooseContainerType();

    bool addContainer(int shipId, const Container& container, bool exportCargo);
    void reportFailure(const char* what) const;

public:
    PortMenu(PortController& controller);
    void run();
};

#endif
