#include "ContainerYard.h"
#include <iostream>

using namespace std;

ContainerYard::ContainerYard(int capacity) {
    slotCapacity = (capacity > 0) ? capacity : 50;
    slotCount = 0;
    slots = new YardSlot[slotCapacity];
}

ContainerYard::ContainerYard(const ContainerYard& other) {
    slotCapacity = other.slotCapacity;
    slotCount = other.slotCount;
    slots = new YardSlot[slotCapacity];
    for (int i = 0; i < slotCount; i++) {
        slots[i] = other.slots[i];
    }
}

ContainerYard& ContainerYard::operator=(const ContainerYard& other) {
    if (this == &other) {
        return *this;
    }

    YardSlot* newSlots = new YardSlot[other.slotCapacity];
    for (int i = 0; i < other.slotCount; i++) {
        newSlots[i] = other.slots[i];
    }

    delete[] slots;
    slots = newSlots;
    slotCapacity = other.slotCapacity;
    slotCount = other.slotCount;
    return *this;
}

ContainerYard::~ContainerYard() {
    delete[] slots;
}

// The yard has a fixed physical capacity, so slots cannot be added past it
bool ContainerYard::addSlot(int type, double maxWeight) {
    if (slotCount == slotCapacity) {
        cout << "Error: yard is at its maximum of " << slotCapacity
             << " slots" << endl;
        return false;
    }
    slots[slotCount] = YardSlot(slotCount + 1, type, maxWeight);
    slotCount++;
    return true;
}

// Returns how many slots were actually added
int ContainerYard::addSlots(int count, int type, double maxWeight) {
    int added = 0;
    for (int i = 0; i < count; i++) {
        if (!addSlot(type, maxWeight)) {
            break;
        }
        added++;
    }
    return added;
}

bool ContainerYard::storeContainer(const Container& container) {
    if (findContainer(container.getId()) != -1) {
        cout << "Error: container " << container.getId()
             << " is already stored in the yard" << endl;
        return false;
    }
    int index = findCompatibleSlot(container);
    if (index == -1) {
        return false;
    }
    return slots[index].store(container);
}

bool ContainerYard::retrieveContainer(int containerId) {
    int index = findContainer(containerId);
    if (index == -1) {
        cout << "Error: container " << containerId << " is not in the yard" << endl;
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

int ContainerYard::getSlotCount() const {
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

void ContainerYard::displayYard() const {
    cout << "Container yard: " << getOccupiedSlots() << "/" << slotCount
         << " slots occupied" << endl;
    for (int type = TYPE_STANDARD; type <= TYPE_HAZARDOUS; type++) {
        int total = 0;
        for (int i = 0; i < slotCount; i++) {
            if (slots[i].getAllowedType() == type) {
                total++;
            }
        }
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
