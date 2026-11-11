# machine.py : the machine itself. No input or output happens here.

import settings

# Working state. Other files reach it as machine.r1, machine.p1 and so on.
r1 = [0] * 26     # copies of the 3 chosen rotors; these rotate
r2 = [0] * 26
r3 = [0] * 26
p1 = p2 = p3 = 0          # positions 0-25, shown in the window
notch1 = notch2 = 0       # notch positions of the rotors in slots 1 and 2
plug = list(range(26))    # plugboard: plug[i] is the letter that i swaps with


def rotate(r):
    """Shifts the list left one place, so r[0] moves to the end."""
    for i in range(25):
        temp = r[i]
        r[i] = r[i + 1]
        r[i + 1] = temp


def minus_1(r):
    """Subtracts 1 from every value. On a real rotor the exit ends
    of the wires turn too, and this is what moves them."""
    for i in range(26):
        r[i] = r[i] - 1
        if r[i] < 0:
            r[i] = 25


def turn(r):
    """One click of one rotor."""
    rotate(r)
    if settings.STRONGER:
        minus_1(r)


def step():
    """Runs before every letter. Rotor 1 always turns. A rotor sitting
    at its notch also turns the next rotor."""
    global p1, p2, p3

    # Check both notches before turning anything
    rotor1_at_notch = (p1 == notch1)
    rotor2_at_notch = (p2 == notch2)

    if rotor2_at_notch:
        # The double step: rotor 2 turns again and takes rotor 3 with it
        turn(r2)
        p2 = (p2 + 1) % 26
        turn(r3)
        p3 = (p3 + 1) % 26
    elif rotor1_at_notch:
        turn(r2)
        p2 = (p2 + 1) % 26

    turn(r1)
    p1 = (p1 + 1) % 26


def find(r, x):
    """Returns the position where x sits in r. The way back uses it."""
    for k in range(26):
        if r[k] == x:
            return k
    return -1


def encrypt(c):
    """One keypress: the rotors move, then the letter makes the full trip.
    The same function encrypts and decrypts. c must be a letter."""
    step()

    i = ord(c.upper()) - ord("A")   # letter to 0-25

    i = plug[i]                     # plugboard, way in
    i = r1[i]                       # no + p here: the lists already moved
    i = r2[i]
    i = r3[i]
    i = settings.REFLECTOR_B[i]
    i = find(r3, i)                 # way back: search the current lists
    i = find(r2, i)
    i = find(r1, i)
    i = plug[i]                     # plugboard, way out

    return chr(ord("A") + i)        # 0-25 back to a letter