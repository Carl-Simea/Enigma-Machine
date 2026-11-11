/* settings.c : everything you set before typing a message. */

#define _CRT_SECURE_NO_WARNINGS   /* lets Visual Studio accept scanf */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "settings.h"
#include "machine.h"

/* What the user picked in menu(). setup() reads these. */
int rotor1, rotor2, rotor3;   /* which rotor (0-4 means I-V) sits in slots 1, 2, 3 */
int start1, start2, start3;   /* window start value (0-25) for slots 1, 2, 3 */

const char *ROTOR_NAME[5] = {"I", "II", "III", "IV", "V"};

/* Asks the question until the answer is a number from low to high. */
int askNumber(const char *question, int low, int high) {
    int n, got, ch;
    while (1) {
        printf("%s", question);
        fflush(stdout);
        got = scanf("%d", &n);
        if (got == EOF) exit(0);                  /* input closed: quit */
        do {                                      /* drop the rest of the line */
            ch = getchar();
        } while (ch != '\n' && ch != EOF);
        if (got == 1 && n >= low && n <= high) return n;
        printf("Please type a number from %d to %d.\n", low, high);
    }
}

/* Asks the question until the answer is a letter that is still free. */
char askFreeLetter(const char *question, const char *freeLetters) {
    char answer[100];
    char letter;
    int ch;
    while (1) {
        printf("%s", question);
        fflush(stdout);
        if (scanf("%99s", answer) != 1) exit(0);  /* input closed: quit */
        do {
            ch = getchar();
        } while (ch != '\n' && ch != EOF);

        letter = (char)toupper((unsigned char)answer[0]);
        if (strchr(freeLetters, letter) != NULL) return letter;

        printf("Pick one of the free letters: %s\n", freeLetters);
    }
}

/* Takes one letter out of the list by sliding the rest left over it. */
void removeLetter(char *freeLetters, char letter) {
    char *spot = strchr(freeLetters, letter);
    memmove(spot, spot + 1, strlen(spot));        /* strlen(spot) also moves the ending 0 */
}

/* Asks which 3 rotors to use, in what order, and the start value for each. */
void menu(void) {
    printf("=== ENIGMA MACHINE ===\n");
    printf("Rotors: 1 = I, 2 = II, 3 = III, 4 = IV, 5 = V\n");
    printf("Slot 1 is the right-hand rotor, the one that turns on every key.\n\n");

    rotor1 = askNumber("Rotor for slot 1 (1-5): ", 1, 5) - 1;

    rotor2 = askNumber("Rotor for slot 2 (1-5): ", 1, 5) - 1;
    while (rotor2 == rotor1) {
        printf("That rotor is already in the machine. Pick a different one.\n");
        rotor2 = askNumber("Rotor for slot 2 (1-5): ", 1, 5) - 1;
    }

    rotor3 = askNumber("Rotor for slot 3 (1-5): ", 1, 5) - 1;
    while (rotor3 == rotor1 || rotor3 == rotor2) {
        printf("That rotor is already in the machine. Pick a different one.\n");
        rotor3 = askNumber("Rotor for slot 3 (1-5): ", 1, 5) - 1;
    }

    start1 = askNumber("Start value for slot 1 (0-25): ", 0, 25);
    start2 = askNumber("Start value for slot 2 (0-25): ", 0, 25);
    start3 = askNumber("Start value for slot 3 (0-25): ", 0, 25);
}

/* Builds the machine from the answers. Run it again with the same
   settings and the machine is back at the start, ready to decrypt. */
void setup(void) {
    int k;

    /* Copy the chosen rotors, so the originals in ROTOR stay clean */
    for (k = 0; k < 26; k++) {
        r1[k] = ROTOR[rotor1][k];
        r2[k] = ROTOR[rotor2][k];
        r3[k] = ROTOR[rotor3][k];
        plug[k] = k;                    /* no cables yet: every letter is itself */
    }

    /* Turn each rotor to its start value */
    for (k = 0; k < start1; k++) turn(r1);
    for (k = 0; k < start2; k++) turn(r2);
    for (k = 0; k < start3; k++) turn(r3);
    p1 = start1;
    p2 = start2;
    p3 = start3;

    notch1 = NOTCH[rotor1];
    notch2 = NOTCH[rotor2];

    printf("\nRotors, left to right: %s %s %s\n",
           ROTOR_NAME[rotor3], ROTOR_NAME[rotor2], ROTOR_NAME[rotor1]);
    printf("Window, left to right: %c %c %c\n", 'A' + p3, 'A' + p2, 'A' + p1);
}

/* Asks how many pairs (0-10), then each pair. */
void setPlugboard(void) {
    char freeLetters[27] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    char a, b;
    int pairs, n, k;

    pairs = askNumber("\nHow many plugboard pairs (0-10)? ", 0, MAX_PAIRS);

    for (n = 1; n <= pairs; n++) {
        printf("\nPair %d. Free letters: %s\n", n, freeLetters);

        a = askFreeLetter("  Letter: ", freeLetters);
        removeLetter(freeLetters, a);              /* a is used now */

        b = askFreeLetter("  Swap it with: ", freeLetters);
        removeLetter(freeLetters, b);              /* so is b */

        plug[a - 'A'] = b - 'A';                   /* store the pair both ways */
        plug[b - 'A'] = a - 'A';
    }

    printf("\nPlugboard:");
    if (pairs == 0) printf(" no pairs");
    for (k = 0; k < 26; k++) {
        if (plug[k] > k) printf(" %c%c", 'A' + k, 'A' + plug[k]);
    }
    printf("\n");
}