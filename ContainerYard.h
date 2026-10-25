#ifndef CONTAINERYARD_H
#define CONTAINERYARD_H

#include "YardSlot.h"

class ContainerYard {
private:
    YardSlot* slots;
    int slotCount;
    int slotCapacity;

public:
    ContainerYard(int capacity = 50);
    ContainerYard(const ContainerYard& other);
    ContainerYard& operator=(const ContainerYard& other);
    ~ContainerYard();

    bool addSlot(int type, double maxWeight);
    int addSlots(int count, int type, double maxWeight);

    bool storeContainer(const Container& container);
    bool retrieveContainer(int containerId);
    bool hasSpaceFor(const Container& container) const;
    bool supports(const Container& container) const;

    int findCompatibleSlot(const Container& container) const;
    int findCompatibleSlot(int containerType, double weight) const;
    int findContainer(int containerId) const;

    int getSlotCount() const;
    int getOccupiedSlots() const;
    int getFreeSlots() const;
    int getFreeSlots(int containerType) const;
    void displayYard() const;
};

#endif
