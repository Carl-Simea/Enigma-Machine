# settings.py : the settings screen of the machine.
# The lists are the hardware (real Enigma I wirings). Never change them:
# the machine works on copies, and these are the originals it resets from.

import sys
import machine

# Every letter is a number from 0 to 25 (A = 0, Z = 25).
ROTOR = [
    [ 4,10,12, 5,11, 6, 3,16,21,25,13,19,14,22,24, 7,23,20,18,15, 0, 8, 1,17, 2, 9],  # I
    [ 0, 9, 3,10,18, 8,17,20,23, 1,11, 7,22,19,12, 2,16, 6,25,13,15,24, 5,21,14, 4],  # II
    [ 1, 3, 5, 7, 9,11, 2,15,17,19,23,21,25,13,24, 4, 8,22, 6, 0,10,12,20,18,16,14],  # III
    [ 4,18,14,21,15,25, 9, 0,24,16,20, 8,17, 7,23,11,13, 5,19, 6,10, 3, 2,12,22, 1],  # IV
    [21,25, 1,17, 6, 8,19,24,20,15,18, 3,13, 7,11,23, 0,22,12, 9,16,14, 5, 4, 2,10],  # V
]
REFLECTOR_B = [24,17,20, 7,16,18,11, 3,15,23,13, 6,14,10,12, 8, 4, 1, 5,25, 2,22,21, 9, 0,19]
NOTCH = [16, 4, 21, 9, 25]   # Q, E, V, J, Z: where rotors I-V turn their neighbor

MAX_PAIRS = 10     # plugboard cables that came with the machine
STRONGER = True    # True = real Enigma (rotate, then minus_1). False = simple version

ROTOR_NAME = ["I", "II", "III", "IV", "V"]

# What the user picked in menu(). setup() reads these.
rotor1 = rotor2 = rotor3 = 0   # which rotor (0-4 means I-V) sits in slots 1, 2, 3
start1 = start2 = start3 = 0   # window start value (0-25) for slots 1, 2, 3


def ask(question):
    """input(), but quits cleanly if the input is closed."""
    try:
        return input(question)
    except EOFError:
        sys.exit(0)


def ask_number(question, low, high):
    """Asks the question until the answer is a number from low to high."""
    while True:
        answer = ask(question).split()
        try:
            n = int(answer[0])
            if low <= n <= high:
                return n
        except (ValueError, IndexError):
            pass
        print(f"Please type a number from {low} to {high}.")


def ask_free_letter(question, free_letters):
    """Asks the question until the answer is a letter that is still free."""
    while True:
        answer = ask(question).strip().upper()
        if answer != "" and answer[0] in free_letters:
            return answer[0]
        print("Pick one of the free letters:", free_letters)


def menu():
    """Asks which 3 rotors to use, in what order, and the start value for each."""
    global rotor1, rotor2, rotor3, start1, start2, start3

    print("=== ENIGMA MACHINE ===")
    print("Rotors: 1 = I, 2 = II, 3 = III, 4 = IV, 5 = V")
    print("Slot 1 is the right-hand rotor, the one that turns on every key.\n")

    rotor1 = ask_number("Rotor for slot 1 (1-5): ", 1, 5) - 1

    rotor2 = ask_number("Rotor for slot 2 (1-5): ", 1, 5) - 1
    while rotor2 == rotor1:
        print("That rotor is already in the machine. Pick a different one.")
        rotor2 = ask_number("Rotor for slot 2 (1-5): ", 1, 5) - 1

    rotor3 = ask_number("Rotor for slot 3 (1-5): ", 1, 5) - 1
    while rotor3 == rotor1 or rotor3 == rotor2:
        print("That rotor is already in the machine. Pick a different one.")
        rotor3 = ask_number("Rotor for slot 3 (1-5): ", 1, 5) - 1

    start1 = ask_number("Start value for slot 1 (0-25): ", 0, 25)
    start2 = ask_number("Start value for slot 2 (0-25): ", 0, 25)
    start3 = ask_number("Start value for slot 3 (0-25): ", 0, 25)


def setup():
    """Builds the machine from the answers. Run it again with the same
    settings and the machine is back at the start, ready to decrypt."""
    # Copy the chosen rotors, so the originals in ROTOR stay clean
    for k in range(26):
        machine.r1[k] = ROTOR[rotor1][k]
        machine.r2[k] = ROTOR[rotor2][k]
        machine.r3[k] = ROTOR[rotor3][k]
        machine.plug[k] = k              # no cables yet: every letter is itself

    # Turn each rotor to its start value
    for k in range(start1):
        machine.turn(machine.r1)
    for k in range(start2):
        machine.turn(machine.r2)
    for k in range(start3):
        machine.turn(machine.r3)
    machine.p1 = start1                  # machine.p1, so the machine's own copy changes
    machine.p2 = start2
    machine.p3 = start3

    machine.notch1 = NOTCH[rotor1]
    machine.notch2 = NOTCH[rotor2]

    print("\nRotors, left to right:", ROTOR_NAME[rotor3], ROTOR_NAME[rotor2], ROTOR_NAME[rotor1])
    print("Window, left to right:", chr(65 + machine.p3), chr(65 + machine.p2), chr(65 + machine.p1))


def set_plugboard():
    """Asks how many pairs (0-10), then each pair."""
    free_letters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"

    pairs = ask_number("\nHow many plugboard pairs (0-10)? ", 0, MAX_PAIRS)

    for n in range(1, pairs + 1):
        print(f"\nPair {n}. Free letters: {free_letters}")

        a = ask_free_letter("  Letter: ", free_letters)
        free_letters = free_letters.replace(a, "")     # a is used now

        b = ask_free_letter("  Swap it with: ", free_letters)
        free_letters = free_letters.replace(b, "")     # so is b

        machine.plug[ord(a) - 65] = ord(b) - 65        # store the pair both ways
        machine.plug[ord(b) - 65] = ord(a) - 65

    text = "\nPlugboard:"
    if pairs == 0:
        text += " no pairs"
    for k in range(26):
        if machine.plug[k] > k:
            text += " " + chr(65 + k) + chr(65 + machine.plug[k])
    print(text)