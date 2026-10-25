#ifndef CONSOLEINPUT_H
#define CONSOLEINPUT_H

// Safe keyboard input. Every function keeps asking until the value is
// acceptable, and always consumes the whole line (including Enter).
// It knows nothing about ships or ports.
class ConsoleInput {
public:
    static int readInt(const char* prompt, int min, int max);
    static double readDouble(const char* prompt, double min, double max);
    static void readText(const char* prompt, char* buffer, int bufferSize);
    static int readTime(const char* prompt);          // reads HH:MM, returns minutes
    static int readMenuChoice(const char* title, const char* options[], int optionCount);
    static bool readYesNo(const char* prompt);
    static void pause();

private:
    static void clearLine();                          // discard the rest of the input line
    static void exitIfInputEnded();
    static bool onlySpacesLeft();
};

#endif
