#include "ContainerYard.h"
#include <iostream>

using namespace std;

// An empty yard; the user chooses its size later with setup()
ContainerYard::ContainerYard()
    : slots(nullptr), slotCount(0), configured(false) {
}

ContainerYard::ContainerYard(const ContainerYard& other)
    : slots(nullptr), slotCount(other.slotCount), configured(other.configured) {
    if (slotCount > 0) {
        slots = new YardSlot[slotCount];
        for (int i = 0; i < slotCount; i++) {
            slots[i] = other.slots[i];
        }
    }
}

ContainerYard& ContainerYard::operator=(const ContainerYard& other) {
    if (this == &other) {
        return *this;
    }

    YardSlot* newSlots = nullptr;
    if (other.slotCount > 0) {
        newSlots = new YardSlot[other.slotCount];
        for (int i = 0; i < other.slotCount; i++) {
            newSlots[i] = other.slots[i];
        }
    }

    delete[] slots;
    slots = newSlots;
    slotCount = other.slotCount;
    configured = other.configured;
    return *this;
}

ContainerYard::~ContainerYard() {
    delete[] slots;
}

bool ContainerYard::isValidSlotCounts(int standard, int reefer, int hazard) {
    if (standard < 0 || reefer < 0 || hazard < 0) {
        return false;
    }
    int total = standard + reefer + hazard;
    return total >= 1 && total <= MAX_TOTAL_SLOTS;
}

// Works like a validating constructor: every value is checked first, and
// the yard is left unchanged if any of them is invalid
bool ContainerYard::setup(int standardSlots, int reeferSlots, int hazardSlots,
                          double maxWeightPerSlot) {
    if (!isValidSlotCounts(standardSlots, reeferSlots, hazardSlots) ||
        !YardSlot::isValidMaxWeight(maxWeightPerSlot)) {
        return false;
    }

    delete[] slots;
    slotCount = standardSlots + reeferSlots + hazardSlots;
    slots = new YardSlot[slotCount];

    int index = 0;
    for (int i = 0; i < standardSlots; i++, index++) {
        slots[index] = YardSlot(index + 1, maxWeightPerSlot, Container::STANDARD);
    }
    for (int i = 0; i < reeferSlots; i++, index++) {
        slots[index] = YardSlot(index + 1, maxWeightPerSlot, Container::REFRIGERATED);
    }
    for (int i = 0; i < hazardSlots; i++, index++) {
        slots[index] = YardSlot(index + 1, maxWeightPerSlot, Container::HAZARDOUS);
    }
    configured = true;
    return true;
}

bool ContainerYard::isConfigured() const {
    return configured;
}

bool ContainerYard::storeContainer(const Container& container) {
    if (!container.isValid() || findContainer(container.getId()) != -1) {
        return false;
    }
    int index = findCompatibleSlot(container);
    if (index == -1) {
        return false;
    }
    return slots[index].store(container.getId());
}

bool ContainerYard::retrieveContainer(int containerId) {
    int index = findContainer(containerId);
    if (index == -1) {
        return false;
    }
    slots[index].release();
    return true;
}

bool ContainerYard::hasSpaceFor(const Container& container) const {
    return findCompatibleSlot(container) != -1;
}

// True if at least one slot could ever hold this container
bool ContainerYard::supports(const Container& container) const {
    for (int i = 0; i < slotCount; i++) {
        if (slots[i].accepts(container)) {
            return true;
        }
    }
    return false;
}

// Returns the index of the first free slot that accepts the container, or -1
int ContainerYard::findCompatibleSlot(const Container& container) const {
    if (!container.isValid()) {
        return -1;
    }
    return findCompatibleSlot(container.getType(), container.getWeight());
}

int ContainerYard::findCompatibleSlot(int containerType, double weight) const {
    for (int i = 0; i < slotCount; i++) {
        if (slots[i].canStore(containerType, weight)) {
            return i;
        }
    }
    return -1;
}

// Returns the index of the slot holding the container, or -1
int ContainerYard::findContainer(int containerId) const {
    for (int i = 0; i < slotCount; i++) {
        if (!slots[i].isAvailable() && slots[i].getStoredContainerId() == containerId) {
            return i;
        }
    }
    return -1;
}

int ContainerYard::getTotalSlots() const {
    return slotCount;
}

int ContainerYard::getOccupiedSlots() const {
    return slotCount - getFreeSlots();
}

int ContainerYard::getFreeSlots() const {
    int free = 0;
    for (int i = 0; i < slotCount; i++) {
        if (slots[i].isAvailable()) {
            free++;
        }
    }
    return free;
}

int ContainerYard::getFreeSlots(int containerType) const {
    int free = 0;
    for (int i = 0; i < slotCount; i++) {
        if (slots[i].isAvailable() && slots[i].getAllowedType() == containerType) {
            free++;
        }
    }
    return free;
}

int ContainerYard::countSlotsOfType(int containerType) const {
    int total = 0;
    for (int i = 0; i < slotCount; i++) {
        if (slots[i].getAllowedType() == containerType) {
            total++;
        }
    }
    return total;
}

void ContainerYard::displayYard() const {
    if (!configured) {
        cout << "Container yard: not set up yet" << endl;
        return;
    }
    cout << "Container yard: " << getOccupiedSlots() << "/" << slotCount
         << " slots occupied" << endl;
    for (int type = Container::STANDARD; type <= Container::HAZARDOUS; type++) {
        int total = countSlotsOfType(type);
        if (total == 0) {
            continue;
        }
        cout << "  " << Container::typeToString(type) << ": "
             << total - getFreeSlots(type) << "/" << total << " used [";
        for (int i = 0; i < slotCount; i++) {
            if (slots[i].getAllowedType() == type) {
                cout << (slots[i].isAvailable() ? '.' : '#');
            }
        }
        cout << "]" << endl;
    }
}
