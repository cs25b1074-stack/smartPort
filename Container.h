#ifndef CONTAINER_H
#define CONTAINER_H

class Container {
public:
    // Container types
    static const int STANDARD = 1;
    static const int REFRIGERATED = 2;
    static const int HAZARDOUS = 3;

    // Container status values
    static const int ON_SHIP = 1;
    static const int IN_YARD = 2;
    static const int ASSIGNED_TO_SHIP = 3;
    static const int DELIVERED = 4;

    // Validation limits
    static const double MAX_WEIGHT;         // 35.0 t, defined in Container.cpp
    static const int MIN_DESTINATION = 1;
    static const int MAX_DESTINATION = 999;

    // Sizes of the text fields
    static const int OWNER_NAME_SIZE = 50;
    static const int CONTENT_DESC_SIZE = 100;
    static const int PORT_NAME_SIZE = 50;
    static const int SEAL_NUMBER_SIZE = 20;

private:
    static int nextId;          // used to generate unique container IDs

    int containerId;
    double weight;              // in tonnes
    int destinationCode;
    int containerType;
    int status;
    bool valid;

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

    static bool isValidWeight(double weight);
    static bool isValidType(int type);
    static bool isValidDestination(int destination);
    static bool isValidStatus(int status);

    bool isValid() const;
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
