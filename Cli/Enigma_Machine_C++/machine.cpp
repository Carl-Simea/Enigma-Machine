// machine.cpp : the machine itself. No input or output happens here.

#include <cctype>
#include "machine.h"
#include "settings.h"

// Working state (declared extern in machine.h)
int r1[26], r2[26], r3[26];
int p1, p2, p3;
int notch1, notch2;
int plug[26];

// Shifts the array left one place, so r[0] moves to the end.
void rotate(int r[26]) {
    for (int i = 0; i < 25; i++) {
        int temp = r[i];
        r[i] = r[i + 1];
        r[i + 1] = temp;
    }
}

// Subtracts 1 from every value. On a real rotor the exit ends
// of the wires turn too, and this is what moves them.
void minus_1(int r[26]) {
    for (int i = 0; i < 26; i++) {
        r[i] = r[i] - 1;
        if (r[i] < 0) r[i] = 25;
    }
}

// One click of one rotor.
void turn(int r[26]) {
    rotate(r);
    if (STRONGER) minus_1(r);
}

// Runs before every letter. Rotor 1 always turns. A rotor sitting
// at its notch also turns the next rotor.
void step() {
    // Check both notches before turning anything
    bool rotor1AtNotch = (p1 == notch1);
    bool rotor2AtNotch = (p2 == notch2);

    if (rotor2AtNotch) {
        // The double step: rotor 2 turns again and takes rotor 3 with it
        turn(r2);
        p2 = (p2 + 1) % 26;
        turn(r3);
        p3 = (p3 + 1) % 26;
    } else if (rotor1AtNotch) {
        turn(r2);
        p2 = (p2 + 1) % 26;
    }

    turn(r1);
    p1 = (p1 + 1) % 26;
}

// Returns the position where x sits in r. The way back uses it.
int find(int r[26], int x) {
    for (int k = 0; k < 26; k++) {
        if (r[k] == x) return k;
    }
    return -1;
}

// One keypress: the rotors move, then the letter makes the full trip.
// The same function encrypts and decrypts. c must be a letter.
char encrypt(char c) {
    step();

    int i = std::toupper((unsigned char)c) - 'A';   // letter to 0-25

    i = plug[i];            // plugboard, way in
    i = r1[i];              // no + p here: the arrays already moved
    i = r2[i];
    i = r3[i];
    i = REFLECTOR_B[i];
    i = find(r3, i);        // way back: search the current arrays
    i = find(r2, i);
    i = find(r1, i);
    i = plug[i];            // plugboard, way out

    return (char)('A' + i); // 0-25 back to a letter
}