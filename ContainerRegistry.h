#ifndef CONTAINERREGISTRY_H
#define CONTAINERREGISTRY_H

#include "Container.h"

// Owns the one real copy of every container record. Other classes
// refer to containers by ID only.
class ContainerRegistry {
private:
    // An array of pointers is used instead of Container* because
    // new Container[n] would call the default constructor n times and
    // use up container IDs for empty placeholder records.
    Container** containers;
    int count;
    int capacity;

    void grow();
    void copyFrom(const ContainerRegistry& other);
    void clear();

public:
    ContainerRegistry(int initialCapacity = 20);
    ContainerRegistry(const ContainerRegistry& other);
    ContainerRegistry& operator=(const ContainerRegistry& other);
    ~ContainerRegistry();

    bool addContainer(const Container& container);
    Container* findContainer(int containerId);
    const Container* findContainer(int containerId) const;
    bool exists(int containerId) const;
    int getCount() const;
    void displayAll() const;
};

#endif
