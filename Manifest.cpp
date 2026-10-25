#include "Manifest.h"
#include <iostream>

using namespace std;

Manifest::Manifest(int initialCapacity) {
    capacity = (initialCapacity >= 1) ? initialCapacity : 10;   // never leave a null array
    count = 0;
    containerIds = new int[capacity];
}

// Deep copy so the two manifests never share the same array
Manifest::Manifest(const Manifest& other) {
    capacity = other.capacity;
    count = other.count;
    containerIds = new int[capacity];
    for (int i = 0; i < count; i++) {
        containerIds[i] = other.containerIds[i];
    }
}

Manifest& Manifest::operator=(const Manifest& other) {
    if (this == &other) {
        return *this;
    }

    int* newIds = new int[other.capacity];
    for (int i = 0; i < other.count; i++) {
        newIds[i] = other.containerIds[i];
    }

    delete[] containerIds;
    containerIds = newIds;
    capacity = other.capacity;
    count = other.count;
    return *this;
}

Manifest::~Manifest() {
    delete[] containerIds;
}

// Doubles the array when it is full
void Manifest::grow() {
    int newCapacity = capacity * 2;
    int* newIds = new int[newCapacity];
    for (int i = 0; i < count; i++) {
        newIds[i] = containerIds[i];
    }
    delete[] containerIds;
    containerIds = newIds;
    capacity = newCapacity;
}

// Rejects non-positive and duplicate IDs so a container cannot be listed twice
bool Manifest::addContainer(int containerId) {
    if (containerId <= 0 || contains(containerId)) {
        return false;
    }
    if (count == capacity) {
        grow();
    }
    containerIds[count++] = containerId;
    return true;
}

// Removes an ID and shifts the rest down, keeping the original order
bool Manifest::removeContainer(int containerId) {
    for (int i = 0; i < count; i++) {
        if (containerIds[i] == containerId) {
            for (int j = i; j < count - 1; j++) {
                containerIds[j] = containerIds[j + 1];
            }
            count--;
            return true;
        }
    }
    return false;
}

bool Manifest::contains(int containerId) const {
    for (int i = 0; i < count; i++) {
        if (containerIds[i] == containerId) {
            return true;
        }
    }
    return false;
}

// Returns -1 for an out-of-range index
int Manifest::operator[](int index) const {
    if (index < 0 || index >= count) {
        return -1;
    }
    return containerIds[index];
}

int Manifest::getCount() const {
    return count;
}

bool Manifest::isEmpty() const {
    return count == 0;
}

void Manifest::display() const {
    cout << "Manifest (" << count << " containers):";
    if (count == 0) {
        cout << " empty";
    }
    for (int i = 0; i < count; i++) {
        cout << " " << containerIds[i];
    }
    cout << endl;
}
