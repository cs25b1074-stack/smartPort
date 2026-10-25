#include "PortTime.h"
#include <iostream>
#include <iomanip>

using namespace std;

// A time can never be negative
PortTime::PortTime(int minutes) {
    totalMinutes = (minutes >= 0) ? minutes : 0;
}

// An invalid clock value gives 00:00
PortTime::PortTime(int hours, int minutes) {
    totalMinutes = 0;
    if (isValidClock(hours, minutes)) {
        totalMinutes = hours * MINUTES_PER_HOUR + minutes;
    }
}

bool PortTime::isValidClock(int hours, int minutes) {
    return hours >= 0 && minutes >= 0 && minutes <= 59;
}

int PortTime::getMinutes() const {
    return totalMinutes;
}

int PortTime::getHour() const {
    return (totalMinutes % MINUTES_PER_DAY) / MINUTES_PER_HOUR;
}

int PortTime::getMinute() const {
    return totalMinutes % MINUTES_PER_HOUR;
}

PortTime& PortTime::operator++() {
    ++totalMinutes;
    return *this;
}

PortTime PortTime::operator++(int) {
    PortTime old = *this;
    ++totalMinutes;
    return old;
}

// Negative values are ignored so time never runs backwards
PortTime& PortTime::operator+=(int minutes) {
    if (minutes > 0) {
        totalMinutes += minutes;
    }
    return *this;
}

bool PortTime::operator<(const PortTime& other) const {
    return totalMinutes < other.totalMinutes;
}

bool PortTime::operator==(const PortTime& other) const {
    return totalMinutes == other.totalMinutes;
}

// e.g. 10:30 - 08:00 = 150
int PortTime::operator-(const PortTime& other) const {
    return totalMinutes - other.totalMinutes;
}

void PortTime::display() const {
    cout << *this;
}

// Prints HH:MM, with a "+Nd" prefix once the simulation passes midnight
ostream& operator<<(ostream& out, const PortTime& time) {
    int day = time.totalMinutes / MINUTES_PER_DAY;
    if (day > 0) {
        out << "+" << day << "d ";
    }
    char oldFill = out.fill('0');
    out << setw(2) << time.getHour() << ":" << setw(2) << time.getMinute();
    out.fill(oldFill);
    return out;
}
