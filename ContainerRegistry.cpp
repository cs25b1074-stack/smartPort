#include "ContainerRegistry.h"
#include <iostream>
#include <iomanip>

using namespace std;

ContainerRegistry::ContainerRegistry(int initialCapacity) {
    capacity = (initialCapacity > 0) ? initialCapacity : 20;
    count = 0;
    containers = new Container*[capacity];
}

ContainerRegistry::ContainerRegistry(const ContainerRegistry& other) {
    containers = nullptr;
    copyFrom(other);
}

ContainerRegistry& ContainerRegistry::operator=(const ContainerRegistry& other) {
    if (this != &other) {
        clear();
        copyFrom(other);
    }
    return *this;
}

ContainerRegistry::~ContainerRegistry() {
    clear();
}

// Deep copy: each record is duplicated, not just the pointers
void ContainerRegistry::copyFrom(const ContainerRegistry& other) {
    capacity = other.capacity;
    count = other.count;
    containers = new Container*[capacity];
    for (int i = 0; i < count; i++) {
        containers[i] = new Container(*other.containers[i]);
    }
}

void ContainerRegistry::clear() {
    for (int i = 0; i < count; i++) {
        delete containers[i];
    }
    delete[] containers;
    containers = nullptr;
    count = 0;
}

void ContainerRegistry::grow() {
    int newCapacity = capacity * 2;
    Container** newArray = new Container*[newCapacity];
    for (int i = 0; i < count; i++) {
        newArray[i] = containers[i];
    }
    delete[] containers;
    containers = newArray;
    capacity = newCapacity;
}

bool ContainerRegistry::addContainer(const Container& container) {
    if (exists(container.getId())) {
        cout << "Error: container " << container.getId()
             << " is already registered" << endl;
        return false;
    }
    if (count == capacity) {
        grow();
    }
    containers[count++] = new Container(container);
    return true;
}

Container* ContainerRegistry::findContainer(int containerId) {
    for (int i = 0; i < count; i++) {
        if (containers[i]->getId() == containerId) {
            return containers[i];
        }
    }
    return nullptr;
}

const Container* ContainerRegistry::findContainer(int containerId) const {
    for (int i = 0; i < count; i++) {
        if (containers[i]->getId() == containerId) {
            return containers[i];
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
        const Container* c = containers[i];
        cout << "  " << setw(6) << c->getId()
             << setw(14) << Container::typeToString(c->getType())
             << setw(9) << c->getWeight()
             << setw(18) << Container::statusToString(c->getStatus())
             << c->getContentDescription() << endl;
    }
    cout << right;
}
