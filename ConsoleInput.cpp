#include "ConsoleInput.h"
#include <iostream>
#include <cstring>
#include <cstdio>
#include <cstdlib>

using namespace std;

void ConsoleInput::clearLine() {
    cin.clear();
    cin.ignore(10000, '\n');
}

// If the input stream has ended (Ctrl+D, or a piped file ran out), asking
// again would loop forever, so the program stops instead
void ConsoleInput::exitIfInputEnded() {
    if (cin.eof()) {
        cout << endl << "Input ended. Exiting." << endl;
        exit(0);
    }
}

// Skips spaces after a number; true if only the end of the line remains
bool ConsoleInput::onlySpacesLeft() {
    while (cin.peek() == ' ' || cin.peek() == '\t') {
        cin.get();
    }
    return cin.peek() == '\n' || cin.peek() == '\r';
}

int ConsoleInput::readInt(const char* prompt, int min, int max) {
    int value;
    while (true) {
        cout << prompt;
        cin >> value;
        exitIfInputEnded();
        // fail() catches letters; onlySpacesLeft() catches "12abc" and "3.5"
        if (cin.fail() || !onlySpacesLeft()) {
            clearLine();
            cout << "  Invalid input. Please enter a whole number." << endl;
        } else if (value < min || value > max) {
            clearLine();
            cout << "  Please enter a value between " << min << " and " << max << "." << endl;
        } else {
            clearLine();
            return value;
        }
    }
}

double ConsoleInput::readDouble(const char* prompt, double min, double max) {
    double value;
    while (true) {
        cout << prompt;
        cin >> value;
        exitIfInputEnded();
        if (cin.fail() || !onlySpacesLeft()) {
            clearLine();
            cout << "  Invalid input. Please enter a number." << endl;
        } else if (value < min || value > max) {
            clearLine();
            cout << "  Please enter a value between " << min << " and " << max << "." << endl;
        } else {
            clearLine();
            return value;
        }
    }
}

void ConsoleInput::readText(const char* prompt, char* buffer, int bufferSize) {
    while (true) {
        cout << prompt;
        cin.getline(buffer, bufferSize);
        if (cin.fail()) {
            exitIfInputEnded();
            clearLine();                    // line was longer than the buffer
            cout << "  Too long. Maximum " << (bufferSize - 1) << " characters." << endl;
            continue;
        }
        // Remove a trailing '\r' left by Windows line endings
        int len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\r') {
            buffer[len - 1] = '\0';
        }
        if (buffer[0] == '\0') {
            cout << "  This field cannot be empty." << endl;
        } else {
            return;
        }
    }
}

int ConsoleInput::readTime(const char* prompt) {
    char text[20];
    int hours;
    int minutes;
    char extra;
    while (true) {
        readText(prompt, text, 20);
        // exactly two numbers separated by ':' and nothing after them
        if (sscanf(text, "%d:%d%c", &hours, &minutes, &extra) == 2 &&
            hours >= 0 && hours <= 99 && minutes >= 0 && minutes <= 59) {
            return hours * 60 + minutes;
        }
        cout << "  Please use the format HH:MM, for example 08:30." << endl;
    }
}

int ConsoleInput::readMenuChoice(const char* title, const char* options[], int optionCount) {
    cout << endl << title << endl;
    for (int i = 0; i < optionCount; i++) {
        cout << "  " << (i + 1) << ". " << options[i] << endl;
    }
    return readInt("Choose an option: ", 1, optionCount);
}

bool ConsoleInput::readYesNo(const char* prompt) {
    char text[10];
    while (true) {
        readText(prompt, text, 10);
        if (strcmp(text, "y") == 0 || strcmp(text, "Y") == 0) {
            return true;
        }
        if (strcmp(text, "n") == 0 || strcmp(text, "N") == 0) {
            return false;
        }
        cout << "  Please enter y or n." << endl;
    }
}

void ConsoleInput::pause() {
    cout << endl << "Press Enter to continue...";
    cin.ignore(10000, '\n');
    exitIfInputEnded();
}
