#ifndef PORTTIME_H
#define PORTTIME_H

#include <iostream>

const int MINUTES_PER_HOUR = 60;
const int MINUTES_PER_DAY = 1440;

// Simulation time stored as minutes since 00:00 on day 1
class PortTime {
private:
    int totalMinutes;

public:
    PortTime(int minutes = 0);
    PortTime(int hours, int minutes);

    int getMinutes() const;
    int getHour() const;
    int getMinute() const;

    PortTime& operator++();          // prefix: advance one minute
    PortTime operator++(int);        // postfix
    PortTime& operator+=(int minutes);
    bool operator<(const PortTime& other) const;
    bool operator==(const PortTime& other) const;
    int operator-(const PortTime& other) const;   // difference in minutes

    void display() const;

    friend std::ostream& operator<<(std::ostream& out, const PortTime& time);
};

#endif
