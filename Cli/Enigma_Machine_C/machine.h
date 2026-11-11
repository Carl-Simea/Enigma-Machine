#pragma once

/* machine.h : what exists in machine.c. */

/* Working state. It is defined once, in machine.c.
   "extern" lets settings.c and main.c use the same copy. */
extern int r1[26], r2[26], r3[26];   /* copies of the 3 chosen rotors; these rotate */
extern int p1, p2, p3;               /* positions 0-25, shown in the window */
extern int notch1, notch2;           /* notch positions of the rotors in slots 1 and 2 */
extern int plug[26];                 /* plugboard: plug[i] is the letter that i swaps with */

void rotate(int r[26]);       /* shifts the array left one place */
void minus_1(int r[26]);      /* subtracts 1 from every value (0 wraps to 25) */
void turn(int r[26]);         /* one click of one rotor */
void step(void);              /* moves the rotors before each letter */
int  find(int r[26], int x);  /* position where x sits in r */
char encrypt(char c);         /* one letter in, one letter out */