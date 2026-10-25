#include "YardSlot.h"
#include <iostream>

using namespace std;

YardSlot::YardSlot() {
    slotId = 0;
    storedContainerId = -1;
    maximumWeight = 0.0;
    allowedContainerType = TYPE_STANDARD;
    isOccupied = false;
}

YardSlot::YardSlot(int id, int type, double maxWeight) {
    slotId = id;
    storedContainerId = -1;
    maximumWeight = (maxWeight > 0) ? maxWeight : 0.0;
    allowedContainerType = (type >= TYPE_STANDARD && type <= TYPE_HAZARDOUS) ? type : TYPE_STANDARD;
    isOccupied = false;
}

int YardSlot::getId() const {
    return slotId;
}

int YardSlot::getStoredContainerId() const {
    return storedContainerId;
}

int YardSlot::getAllowedType() const {
    return allowedContainerType;
}

double YardSlot::getMaximumWeight() const {
    return maximumWeight;
}

bool YardSlot::canStore(const Container& container) const {
    return !isOccupied && container.isCompatibleWith(allowedContainerType) &&
           container.getWeight() <= maximumWeight;
}

bool YardSlot::canStore(int containerType, double weight) const {
    return !isOccupied && containerType == allowedContainerType &&
           weight <= maximumWeight;
}

// Type and weight check only, ignoring whether the slot is currently free
bool YardSlot::accepts(const Container& container) const {
    return container.isCompatibleWith(allowedContainerType) &&
           container.getWeight() <= maximumWeight;
}

bool YardSlot::store(const Container& container) {
    if (!canStore(container)) {
        cout << "Error: slot " << slotId << " cannot store container "
             << container.getId() << endl;
        return false;
    }
    storedContainerId = container.getId();
    isOccupied = true;
    return true;
}

// Empties the slot and returns the ID that was stored (-1 if empty)
int YardSlot::release() {
    int released = storedContainerId;
    storedContainerId = -1;
    isOccupied = false;
    return released;
}

bool YardSlot::isAvailable() const {
    return !isOccupied;
}
