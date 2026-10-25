#ifndef MANIFEST_H
#define MANIFEST_H

// Dynamic list of container IDs. It stores IDs only; the actual
// container records are owned by the ContainerRegistry.
class Manifest {
private:
    int* containerIds;
    int count;
    int capacity;

    void grow();

public:
    Manifest(int initialCapacity = 10);
    Manifest(const Manifest& other);
    Manifest& operator=(const Manifest& other);
    ~Manifest();

    bool addContainer(int containerId);
    bool removeContainer(int containerId);
    bool contains(int containerId) const;
    int operator[](int index) const;
    int getCount() const;
    bool isEmpty() const;
    void display() const;
};

#endif
