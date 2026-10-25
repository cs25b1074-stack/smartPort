#include "ContainerRegistry.h"
#include <iostream>
#include <iomanip>

using namespace std;

// A bad capacity is corrected rather than rejected, so the registry is always usable.
// new Container[n] is safe because Container() is an invalid placeholder that
// does not use up an ID.
ContainerRegistry::ContainerRegistry(int initialCapacity) {
    capacity = (initialCapacity >= 1) ? initialCapacity : 10;
    count = 0;
    containers = new Container[capacity];
}

ContainerRegistry::ContainerRegistry(const ContainerRegistry& other) {
    capacity = other.capacity;
    count = other.count;
    containers = new Container[capacity];
    for (int i = 0; i < count; i++) {
        containers[i] = other.containers[i];
    }
}

ContainerRegistry& ContainerRegistry::operator=(const ContainerRegistry& other) {
    if (this == &other) {
        return *this;
    }

    Container* newArray = new Container[other.capacity];
    for (int i = 0; i < other.count; i++) {
        newArray[i] = other.containers[i];
    }

    delete[] containers;
    containers = newArray;
    capacity = other.capacity;
    count = other.count;
    return *this;
}

ContainerRegistry::~ContainerRegistry() {
    delete[] containers;
}

void ContainerRegistry::grow() {
    int newCapacity = capacity * 2;
    Container* newArray = new Container[newCapacity];
    for (int i = 0; i < count; i++) {
        newArray[i] = containers[i];
    }
    delete[] containers;
    containers = newArray;
    capacity = newCapacity;
}

// Rejects invalid containers and duplicate IDs
bool ContainerRegistry::addContainer(const Container& container) {
    if (!container.isValid() || exists(container.getId())) {
        return false;
    }
    if (count == capacity) {
        grow();
    }
    containers[count++] = container;
    return true;
}

Container* ContainerRegistry::findContainer(int containerId) {
    for (int i = 0; i < count; i++) {
        if (containers[i].getId() == containerId) {
            return &containers[i];
        }
    }
    return nullptr;
}

const Container* ContainerRegistry::findContainer(int containerId) const {
    for (int i = 0; i < count; i++) {
        if (containers[i].getId() == containerId) {
            return &containers[i];
        }
    }
    return nullptr;
}

bool ContainerRegistry::exists(int containerId) const {
    return findContainer(containerId) != nullptr;
}

int ContainerRegistry::getCount() const {
    return count;
}

void ContainerRegistry::displayAll() const {
    cout << "Container registry (" << count << " records)" << endl;
    cout << "  " << left << setw(6) << "ID" << setw(14) << "Type"
         << setw(9) << "Weight" << setw(18) << "Status" << "Contents" << endl;
    for (int i = 0; i < count; i++) {
        const Container& c = containers[i];
        cout << "  " << setw(6) << c.getId()
             << setw(14) << Container::typeToString(c.getType())
             << setw(9) << c.getWeight()
             << setw(18) << Container::statusToString(c.getStatus())
             << c.getContentDescription() << endl;
    }
    cout << right;
}
