// settings.cpp : everything you set before typing a message.

#include <iostream>
#include <string>
#include <cctype>
#include <cstdlib>
#include "settings.h"
#include "machine.h"

// What the user picked in menu(). setup() reads these.
int rotor1, rotor2, rotor3;   // which rotor (0-4 means I-V) sits in slots 1, 2, 3
int start1, start2, start3;   // window start value (0-25) for slots 1, 2, 3

const std::string ROTOR_NAME[5] = {"I", "II", "III", "IV", "V"};

// Asks the question until the answer is a number from low to high.
int askNumber(const std::string& question, int low, int high) {
    int n;
    while (true) {
        std::cout << question;
        if (std::cin >> n) {
            std::cin.ignore(10000, '\n');          // drop the rest of the line
            if (n >= low && n <= high) return n;
        } else {
            if (std::cin.eof()) std::exit(0);      // input closed: quit
            std::cin.clear();                      // recover from a letter typed as a number
            std::cin.ignore(10000, '\n');
        }
        std::cout << "Please type a number from " << low << " to " << high << ".\n";
    }
}

// Asks the question until the answer is a letter that is still free.
char askFreeLetter(const std::string& question, const std::string& freeLetters) {
    std::string answer;
    while (true) {
        std::cout << question;
        if (!(std::cin >> answer)) std::exit(0);   // input closed: quit
        std::cin.ignore(10000, '\n');

        char letter = (char)std::toupper((unsigned char)answer[0]);
        if (freeLetters.find(letter) != std::string::npos) return letter;

        std::cout << "Pick one of the free letters: " << freeLetters << "\n";
    }
}

// Asks which 3 rotors to use, in what order, and the start value for each.
void menu() {
    std::cout << "=== ENIGMA MACHINE ===\n";
    std::cout << "Rotors: 1 = I, 2 = II, 3 = III, 4 = IV, 5 = V\n";
    std::cout << "Slot 1 is the right-hand rotor, the one that turns on every key.\n\n";

    rotor1 = askNumber("Rotor for slot 1 (1-5): ", 1, 5) - 1;

    rotor2 = askNumber("Rotor for slot 2 (1-5): ", 1, 5) - 1;
    while (rotor2 == rotor1) {
        std::cout << "That rotor is already in the machine. Pick a different one.\n";
        rotor2 = askNumber("Rotor for slot 2 (1-5): ", 1, 5) - 1;
    }

    rotor3 = askNumber("Rotor for slot 3 (1-5): ", 1, 5) - 1;
    while (rotor3 == rotor1 || rotor3 == rotor2) {
        std::cout << "That rotor is already in the machine. Pick a different one.\n";
        rotor3 = askNumber("Rotor for slot 3 (1-5): ", 1, 5) - 1;
    }

    start1 = askNumber("Start value for slot 1 (0-25): ", 0, 25);
    start2 = askNumber("Start value for slot 2 (0-25): ", 0, 25);
    start3 = askNumber("Start value for slot 3 (0-25): ", 0, 25);
}

// Builds the machine from the answers. Run it again with the same
// settings and the machine is back at the start, ready to decrypt.
void setup() {
    // Copy the chosen rotors, so the originals in ROTOR stay clean
    for (int k = 0; k < 26; k++) {
        r1[k] = ROTOR[rotor1][k];
        r2[k] = ROTOR[rotor2][k];
        r3[k] = ROTOR[rotor3][k];
        plug[k] = k;                    // no cables yet: every letter is itself
    }

    // Turn each rotor to its start value
    for (int k = 0; k < start1; k++) turn(r1);
    for (int k = 0; k < start2; k++) turn(r2);
    for (int k = 0; k < start3; k++) turn(r3);
    p1 = start1;
    p2 = start2;
    p3 = start3;

    notch1 = NOTCH[rotor1];
    notch2 = NOTCH[rotor2];

    std::cout << "\nRotors, left to right: " << ROTOR_NAME[rotor3] << " "
              << ROTOR_NAME[rotor2] << " " << ROTOR_NAME[rotor1] << "\n";
    std::cout << "Window, left to right: " << (char)('A' + p3) << " "
              << (char)('A' + p2) << " " << (char)('A' + p1) << "\n";
}

// Asks how many pairs (0-10), then each pair.
void setPlugboard() {
    std::string freeLetters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    int pairs = askNumber("\nHow many plugboard pairs (0-10)? ", 0, MAX_PAIRS);

    for (int n = 1; n <= pairs; n++) {
        std::cout << "\nPair " << n << ". Free letters: " << freeLetters << "\n";

        char a = askFreeLetter("  Letter: ", freeLetters);
        freeLetters.erase(freeLetters.find(a), 1);     // a is used now

        char b = askFreeLetter("  Swap it with: ", freeLetters);
        freeLetters.erase(freeLetters.find(b), 1);     // so is b

        plug[a - 'A'] = b - 'A';                       // store the pair both ways
        plug[b - 'A'] = a - 'A';
    }

    std::cout << "\nPlugboard:";
    if (pairs == 0) std::cout << " no pairs";
    for (int k = 0; k < 26; k++) {
        if (plug[k] > k) std::cout << " " << (char)('A' + k) << (char)('A' + plug[k]);
    }
    std::cout << "\n";
}