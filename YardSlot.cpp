#include "YardSlot.h"

const double YardSlot::MAX_WEIGHT = 50.0;

bool YardSlot::isValidMaxWeight(double weight) {
    return weight > 0.0 && weight <= MAX_WEIGHT;
}

// Placeholder used to fill arrays; it is invalid
YardSlot::YardSlot()
    : slotId(0), storedContainerId(-1), maximumWeight(0.0),
      allowedContainerType(0), isOccupied(false), valid(false) {
}

// The type is checked with Container's own rule, so it lives in one place
YardSlot::YardSlot(int slotId, double maxWeight, int allowedType)
    : YardSlot() {
    if (slotId <= 0 || !isValidMaxWeight(maxWeight) || !Container::isValidType(allowedType)) {
        return;
    }
    this->slotId = slotId;
    maximumWeight = maxWeight;
    allowedContainerType = allowedType;
    valid = true;
}

bool YardSlot::isValid() const {
    return valid;
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
    return container.isValid() && canStore(container.getType(), container.getWeight());
}

bool YardSlot::canStore(int containerType, double weight) const {
    return isAvailable() && containerType == allowedContainerType &&
           weight <= maximumWeight;
}

// Type and weight check only, ignoring whether the slot is currently free
bool YardSlot::accepts(const Container& container) const {
    return valid && container.isCompatibleWith(allowedContainerType) &&
           container.getWeight() <= maximumWeight;
}

// The caller checks canStore() first; this only guards the slot's own state
bool YardSlot::store(int containerId) {
    if (!isAvailable() || containerId <= 0) {
        return false;
    }
    storedContainerId = containerId;
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
    return valid && !isOccupied;
}
