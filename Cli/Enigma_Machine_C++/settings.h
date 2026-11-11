#pragma once

// settings.h : the settings screen of the machine.
// The arrays are the hardware (real Enigma I wirings). Never change them:
// the machine works on copies, and these are the originals it resets from.

// Every letter is a number from 0 to 25 (A = 0, Z = 25).
const int ROTOR[5][26] = {
    { 4,10,12, 5,11, 6, 3,16,21,25,13,19,14,22,24, 7,23,20,18,15, 0, 8, 1,17, 2, 9},  // I
    { 0, 9, 3,10,18, 8,17,20,23, 1,11, 7,22,19,12, 2,16, 6,25,13,15,24, 5,21,14, 4},  // II
    { 1, 3, 5, 7, 9,11, 2,15,17,19,23,21,25,13,24, 4, 8,22, 6, 0,10,12,20,18,16,14},  // III
    { 4,18,14,21,15,25, 9, 0,24,16,20, 8,17, 7,23,11,13, 5,19, 6,10, 3, 2,12,22, 1},  // IV
    {21,25, 1,17, 6, 8,19,24,20,15,18, 3,13, 7,11,23, 0,22,12, 9,16,14, 5, 4, 2,10}   // V
};
const int REFLECTOR_B[26] =
    {24,17,20, 7,16,18,11, 3,15,23,13, 6,14,10,12, 8, 4, 1, 5,25, 2,22,21, 9, 0,19};
const int NOTCH[5] = {16, 4, 21, 9, 25};   // Q, E, V, J, Z: where rotors I-V turn their neighbor

const int  MAX_PAIRS = 10;     // plugboard cables that came with the machine
const bool STRONGER  = true;   // true = real Enigma (rotate, then minus_1). false = simple version

void menu();           // asks which 3 rotors to use and the start value for each
void setup();          // builds the machine from those answers
void setPlugboard();   // asks for the plugboard pairs