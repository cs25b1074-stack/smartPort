#include "Container.h"
#include <iostream>
#include <cstring>

using namespace std;

const double Container::MAX_WEIGHT = 35.0;
int Container::nextId = 1001;

// Copies text safely into a fixed-size char array
void Container::copyText(char* target, const char* source, int size) {
    if (source == nullptr) {
        target[0] = '\0';
        return;
    }
    strncpy(target, source, size - 1);
    target[size - 1] = '\0';
}

bool Container::isValidWeight(double weight) {
    return weight > 0.0 && weight <= MAX_WEIGHT;
}

bool Container::isValidType(int type) {
    return type == STANDARD || type == REFRIGERATED || type == HAZARDOUS;
}

bool Container::isValidDestination(int destination) {
    return destination >= MIN_DESTINATION && destination <= MAX_DESTINATION;
}

bool Container::isValidStatus(int status) {
    return status >= ON_SHIP && status <= DELIVERED;
}

// Placeholder used to fill arrays; it is invalid and does not use up an ID
Container::Container()
    : containerId(0), weight(0.0), destinationCode(0),
      containerType(0), status(0), valid(false) {
    ownerName[0] = '\0';
    contentDescription[0] = '\0';
    originPort[0] = '\0';
    destinationPort[0] = '\0';
    sealNumber[0] = '\0';
}

// If any value is illegal the object stays an invalid placeholder
Container::Container(double weight, int type, int destination)
    : Container() {
    if (!isValidWeight(weight) || !isValidType(type) || !isValidDestination(destination)) {
        return;
    }
    this->weight = weight;
    containerType = type;
    destinationCode = destination;
    status = ON_SHIP;

    copyText(ownerName, "Unknown", OWNER_NAME_SIZE);
    copyText(contentDescription, "General cargo", CONTENT_DESC_SIZE);
    copyText(originPort, "Unknown", PORT_NAME_SIZE);
    copyText(destinationPort, "Unknown", PORT_NAME_SIZE);
    copyText(sealNumber, "N/A", SEAL_NUMBER_SIZE);

    containerId = nextId++;     // only valid containers use up an ID
    valid = true;
}

Container::Container(double weight, int type, int destination,
                     const char* owner, const char* contents,
                     const char* origin, const char* destPort,
                     const char* seal)
    : Container(weight, type, destination) {
    if (!valid) {
        return;
    }
    // A null text argument keeps the default text
    if (owner != nullptr)    copyText(ownerName, owner, OWNER_NAME_SIZE);
    if (contents != nullptr) copyText(contentDescription, contents, CONTENT_DESC_SIZE);
    if (origin != nullptr)   copyText(originPort, origin, PORT_NAME_SIZE);
    if (destPort != nullptr) copyText(destinationPort, destPort, PORT_NAME_SIZE);
    if (seal != nullptr)     copyText(sealNumber, seal, SEAL_NUMBER_SIZE);
}

bool Container::isValid() const {
    return valid;
}

int Container::getId() const {
    return containerId;
}

double Container::getWeight() const {
    return weight;
}

int Container::getDestinationCode() const {
    return destinationCode;
}

int Container::getType() const {
    return containerType;
}

int Container::getStatus() const {
    return status;
}

const char* Container::getOwnerName() const {
    return ownerName;
}

const char* Container::getContentDescription() const {
    return contentDescription;
}

const char* Container::getOriginPort() const {
    return originPort;
}

const char* Container::getDestinationPort() const {
    return destinationPort;
}

const char* Container::getSealNumber() const {
    return sealNumber;
}

// Only allows valid status transitions:
// ON_SHIP -> IN_YARD (unloaded)
// IN_YARD -> ASSIGNED_TO_SHIP (picked for loading) or DELIVERED (dispatched by road/rail)
// ASSIGNED_TO_SHIP -> ON_SHIP (loaded) or IN_YARD (loading cancelled)
bool Container::setStatus(int newStatus) {
    if (!valid || !isValidStatus(newStatus)) {
        return false;
    }

    bool allowed = false;
    switch (status) {
        case ON_SHIP:
            allowed = (newStatus == IN_YARD);
            break;
        case IN_YARD:
            allowed = (newStatus == ASSIGNED_TO_SHIP || newStatus == DELIVERED);
            break;
        case ASSIGNED_TO_SHIP:
            allowed = (newStatus == ON_SHIP || newStatus == IN_YARD);
            break;
        case DELIVERED:
            allowed = false;    // delivered is final
            break;
    }

    if (!allowed) {
        return false;
    }
    status = newStatus;
    return true;
}

void Container::setOwnerName(const char* owner) {
    copyText(ownerName, owner, OWNER_NAME_SIZE);
}

void Container::setContentDescription(const char* contents) {
    copyText(contentDescription, contents, CONTENT_DESC_SIZE);
}

void Container::setOriginPort(const char* origin) {
    copyText(originPort, origin, PORT_NAME_SIZE);
}

void Container::setDestinationPort(const char* destPort) {
    copyText(destinationPort, destPort, PORT_NAME_SIZE);
}

void Container::setSealNumber(const char* seal) {
    copyText(sealNumber, seal, SEAL_NUMBER_SIZE);
}

// A container can only go into a slot built for its own type
bool Container::isCompatibleWith(int slotType) const {
    return valid && containerType == slotType;
}

void Container::display() const {
    if (!valid) {
        cout << "Container (invalid placeholder)" << endl;
        return;
    }
    cout << "Container #" << containerId << endl;
    cout << "  Type        : " << typeToString(containerType) << endl;
    cout << "  Weight      : " << weight << " t" << endl;
    cout << "  Status      : " << statusToString(status) << endl;
    cout << "  Owner       : " << ownerName << endl;
    cout << "  Contents    : " << contentDescription << endl;
    cout << "  Route       : " << originPort << " -> " << destinationPort
         << " (code " << destinationCode << ")" << endl;
    cout << "  Seal number : " << sealNumber << endl;
}

const char* Container::typeToString(int type) {
    switch (type) {
        case STANDARD:     return "Standard";
        case REFRIGERATED: return "Refrigerated";
        case HAZARDOUS:    return "Hazardous";
        default:           return "Unknown";
    }
}

const char* Container::statusToString(int status) {
    switch (status) {
        case ON_SHIP:          return "ON_SHIP";
        case IN_YARD:          return "IN_YARD";
        case ASSIGNED_TO_SHIP: return "ASSIGNED_TO_SHIP";
        case DELIVERED:        return "DELIVERED";
        default:               return "UNKNOWN";
    }
}

int Container::getNextId() {
    return nextId;
}
