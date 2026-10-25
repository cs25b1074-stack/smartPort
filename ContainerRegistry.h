#ifndef CONTAINERREGISTRY_H
#define CONTAINERREGISTRY_H

#include "Container.h"

// Owns the one real copy of every container record. Other classes
// refer to containers by ID only.
class ContainerRegistry {
private:
    Container* containers;
    int count;
    int capacity;

    void grow();

public:
    ContainerRegistry(int initialCapacity = 10);
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
