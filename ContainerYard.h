#ifndef CONTAINERYARD_H
#define CONTAINERYARD_H

#include "YardSlot.h"

class ContainerYard {
public:
    static const int MAX_TOTAL_SLOTS = 500;

private:
    YardSlot* slots;
    int slotCount;
    bool configured;            // true after a successful setup()

    int countSlotsOfType(int containerType) const;

public:
    ContainerYard();
    ContainerYard(const ContainerYard& other);
    ContainerYard& operator=(const ContainerYard& other);
    ~ContainerYard();

    bool setup(int standardSlots, int reeferSlots, int hazardSlots, double maxWeightPerSlot);
    static bool isValidSlotCounts(int standard, int reefer, int hazard);

    bool isConfigured() const;
    bool storeContainer(const Container& container);
    bool retrieveContainer(int containerId);
    bool hasSpaceFor(const Container& container) const;
    bool supports(const Container& container) const;

    int findCompatibleSlot(const Container& container) const;
    int findCompatibleSlot(int containerType, double weight) const;
    int findContainer(int containerId) const;

    int getTotalSlots() const;
    int getOccupiedSlots() const;
    int getFreeSlots() const;
    int getFreeSlots(int containerType) const;
    void displayYard() const;
};

#endif
