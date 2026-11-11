/* main.c : calls each function in order, then runs the typing loop.

   Compile:  gcc main.c machine.c settings.c -o enigma */

#include <stdio.h>
#include <ctype.h>
#include "machine.h"
#include "settings.h"

#ifdef _WIN32
#include <conio.h>        /* _getch */
#else
#include <termios.h>      /* tcgetattr, tcsetattr */
#include <sys/ioctl.h>    /* ioctl */
#endif

/* ---- Keyboard helpers: one key at a time, without waiting for Enter ---- */

#ifndef _WIN32
struct termios normalTerminal;   /* the terminal's own settings, to put back later */
bool terminalChanged = false;
#endif

/* Mac/Linux: tell the terminal to hand over each key right away and not
   to print it. Windows needs nothing here, because _getch() already does both. */
void keyboardOn(void) {
#ifndef _WIN32
    struct termios live;
    if (tcgetattr(0, &normalTerminal) != 0) return;   /* input is not a terminal */
    live = normalTerminal;
    live.c_lflag &= ~(ICANON | ECHO | ISIG);          /* no line buffering, no echo, Ctrl+C comes in as a key */
    live.c_cc[VMIN] = 1;
    live.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &live);
    terminalChanged = true;
#endif
}

/* Puts the terminal back the way it was. */
void keyboardOff(void) {
#ifndef _WIN32
    if (terminalChanged) tcsetattr(0, TCSANOW, &normalTerminal);
    terminalChanged = false;
#endif
}

/* Returns the next key pressed. Returns 0 for a key to ignore (arrows,
   function keys) and '\n' when Ctrl+C is pressed or the input is closed. */
int readKey(void) {
#ifdef _WIN32
    int key = _getch();
    if (key == 0 || key == 224) {     /* arrow and function keys send two codes */
        _getch();                     /* throw the second one away */
        return 0;
    }
#else
    int key = getchar();
    if (key == 27) {                  /* Esc: arrow keys send Esc [ A in one burst */
        int waiting = 0;
        ioctl(0, FIONREAD, &waiting);
        while (waiting > 0) {         /* throw the rest of the burst away */
            getchar();
            waiting--;
        }
        return 0;
    }
#endif
    if (key == EOF || key == 3) return '\n';
    return key;
}

int main(void) {
    int key;

    setvbuf(stdin, NULL, _IONBF, 0);   /* no hidden input buffer: keys arrive one at a time */

    menu();            /* 1. pick the rotors and start values */
    setup();           /* 2. build the machine */
    setPlugboard();    /* 3. plug in the cables */

    printf("\nType your message. Press Enter when you are done.\n> ");
    fflush(stdout);

    keyboardOn();
    key = readKey();
    while (key != '\r' && key != '\n') {
        if (key < 128 && isalpha(key)) {
            putchar(encrypt((char)key));   /* a letter goes through the machine */
        } else if (key >= 32 && key < 127) {
            putchar(key);                  /* spaces and punctuation print as they are */
        }
        /* anything else (Backspace, arrows) is ignored: the rotors can't turn back */
        fflush(stdout);
        key = readKey();
    }
    keyboardOff();

    printf("\n\nWindow now, left to right: %c %c %c\n", 'A' + p3, 'A' + p2, 'A' + p1);
    printf("To decrypt, run the program again with the same settings and type the letters above.\n");
    return 0;
}