#ifndef YARDSLOT_H
#define YARDSLOT_H

#include "Container.h"

class YardSlot {
public:
    static const double MAX_WEIGHT;     // 50 t

private:
    int slotId;
    int storedContainerId;
    double maximumWeight;       // in tonnes
    int allowedContainerType;
    bool isOccupied;
    bool valid;

public:
    YardSlot();
    YardSlot(int slotId, double maxWeight, int allowedType);

    static bool isValidMaxWeight(double weight);

    bool isValid() const;
    int getId() const;
    int getStoredContainerId() const;
    int getAllowedType() const;
    double getMaximumWeight() const;

    bool canStore(const Container& container) const;
    bool canStore(int containerType, double weight) const;
    bool accepts(const Container& container) const;
    bool store(int containerId);
    int release();
    bool isAvailable() const;
};

#endif
