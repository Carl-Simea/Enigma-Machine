// main.cpp : calls each function in order, then runs the typing loop.
//
// Compile:  g++ main.cpp machine.cpp settings.cpp -o enigma

#include <iostream>
#include <cstdio>
#include <cctype>
#include "machine.h"
#include "settings.h"

#ifdef _WIN32
#include <conio.h>        // _getch
#else
#include <termios.h>      // tcgetattr, tcsetattr
#include <sys/ioctl.h>    // ioctl
#endif

// ---- Keyboard helpers: one key at a time, without waiting for Enter ----

#ifndef _WIN32
termios normalTerminal;          // the terminal's own settings, to put back later
bool    terminalChanged = false;
#endif

// Mac/Linux: tell the terminal to hand over each key right away and not
// to print it. Windows needs nothing here, because _getch() already does both.
void keyboardOn() {
#ifndef _WIN32
    if (tcgetattr(0, &normalTerminal) != 0) return;   // input is not a terminal
    termios live = normalTerminal;
    live.c_lflag &= ~(ICANON | ECHO | ISIG);          // no line buffering, no echo, Ctrl+C comes in as a key
    live.c_cc[VMIN] = 1;
    live.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &live);
    terminalChanged = true;
#endif
}

// Puts the terminal back the way it was.
void keyboardOff() {
#ifndef _WIN32
    if (terminalChanged) tcsetattr(0, TCSANOW, &normalTerminal);
    terminalChanged = false;
#endif
}

// Returns the next key pressed. Returns 0 for a key to ignore (arrows,
// function keys) and '\n' when Ctrl+C is pressed or the input is closed.
int readKey() {
#ifdef _WIN32
    int key = _getch();
    if (key == 0 || key == 224) {     // arrow and function keys send two codes
        _getch();                     // throw the second one away
        return 0;
    }
#else
    int key = std::getchar();
    if (key == 27) {                  // Esc: arrow keys send Esc [ A in one burst
        int waiting = 0;
        ioctl(0, FIONREAD, &waiting);
        while (waiting > 0) {         // throw the rest of the burst away
            std::getchar();
            waiting--;
        }
        return 0;
    }
#endif
    if (key == EOF || key == 3) return '\n';
    return key;
}

int main() {
    std::setvbuf(stdin, NULL, _IONBF, 0);   // no hidden input buffer: keys arrive one at a time

    menu();            // 1. pick the rotors and start values
    setup();           // 2. build the machine
    setPlugboard();    // 3. plug in the cables

    std::cout << "\nType your message. Press Enter when you are done.\n> " << std::flush;

    keyboardOn();
    int key = readKey();
    while (key != '\r' && key != '\n') {
        if (key < 128 && std::isalpha(key)) {
            std::cout << encrypt((char)key) << std::flush;   // a letter goes through the machine
        } else if (key >= 32 && key < 127) {
            std::cout << (char)key << std::flush;            // spaces and punctuation print as they are
        }
        // anything else (Backspace, arrows) is ignored: the rotors can't turn back
        key = readKey();
    }
    keyboardOff();

    std::cout << "\n\nWindow now, left to right: " << (char)('A' + p3) << " "
              << (char)('A' + p2) << " " << (char)('A' + p1) << "\n";
    std::cout << "To decrypt, run the program again with the same settings and type the letters above.\n";
    return 0;
}