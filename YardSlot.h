#ifndef YARDSLOT_H
#define YARDSLOT_H

#include "Container.h"

class YardSlot {
private:
    int slotId;
    int storedContainerId;
    double maximumWeight;       // in tonnes
    int allowedContainerType;
    bool isOccupied;

public:
    YardSlot();
    YardSlot(int id, int type, double maxWeight);

    int getId() const;
    int getStoredContainerId() const;
    int getAllowedType() const;
    double getMaximumWeight() const;

    bool canStore(const Container& container) const;
    bool canStore(int containerType, double weight) const;
    bool accepts(const Container& container) const;
    bool store(const Container& container);
    int release();
    bool isAvailable() const;
};

#endif
