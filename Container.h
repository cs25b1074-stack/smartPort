#ifndef CONTAINER_H
#define CONTAINER_H

// Container types
const int TYPE_STANDARD = 0;
const int TYPE_REFRIGERATED = 1;
const int TYPE_HAZARDOUS = 2;

// Container status values
const int ON_SHIP = 0;
const int IN_YARD = 1;
const int ASSIGNED_TO_SHIP = 2;
const int DELIVERED = 3;

// Sizes of the text fields
const int OWNER_NAME_SIZE = 50;
const int CONTENT_DESC_SIZE = 100;
const int PORT_NAME_SIZE = 50;
const int SEAL_NUMBER_SIZE = 20;

class Container {
private:
    static int nextId;          // used to generate unique container IDs

    int containerId;
    double weight;              // in tonnes
    int destinationCode;
    int containerType;
    int status;

    // Additional descriptive text fields
    char ownerName[OWNER_NAME_SIZE];        // shipping company / owner
    char contentDescription[CONTENT_DESC_SIZE];
    char originPort[PORT_NAME_SIZE];
    char destinationPort[PORT_NAME_SIZE];
    char sealNumber[SEAL_NUMBER_SIZE];

    static void copyText(char* target, const char* source, int size);

public:
    Container();
    Container(double weight, int type, int destination);
    Container(double weight, int type, int destination,
              const char* owner, const char* contents,
              const char* origin, const char* destPort,
              const char* seal);

    int getId() const;
    double getWeight() const;
    int getDestinationCode() const;
    int getType() const;
    int getStatus() const;
    const char* getOwnerName() const;
    const char* getContentDescription() const;
    const char* getOriginPort() const;
    const char* getDestinationPort() const;
    const char* getSealNumber() const;

    bool setStatus(int newStatus);
    void setOwnerName(const char* owner);
    void setContentDescription(const char* contents);
    void setOriginPort(const char* origin);
    void setDestinationPort(const char* destPort);
    void setSealNumber(const char* seal);

    bool isCompatibleWith(int slotType) const;
    void display() const;

    static const char* typeToString(int type);
    static const char* statusToString(int status);
    static int getNextId();
};

#endif
