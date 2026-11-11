# main.py : calls each function in order, then runs the typing loop.
#
# Run:  python main.py      (in a terminal, not IDLE or a notebook)

import os
import sys
import machine
import settings

if os.name == "nt":
    import msvcrt             # Windows: getwch
else:
    import select             # Mac/Linux
    import termios

# ---- Keyboard helpers: one key at a time, without waiting for Enter ----

normal_terminal = None        # the terminal's own settings, to put back later
live = False                  # True while keys arrive one at a time


def keyboard_on():
    """Mac/Linux: tell the terminal to hand over each key right away and not
    to print it. Windows needs nothing here, because getwch() already does both."""
    global normal_terminal, live
    if not sys.stdin.isatty():
        return                                    # input is not a terminal
    live = True
    if os.name != "nt":
        normal_terminal = termios.tcgetattr(0)
        new = termios.tcgetattr(0)
        new[3] &= ~(termios.ICANON | termios.ECHO | termios.ISIG)   # no line buffering, no echo, Ctrl+C comes in as a key
        new[6][termios.VMIN] = 1
        new[6][termios.VTIME] = 0
        termios.tcsetattr(0, termios.TCSANOW, new)


def keyboard_off():
    """Puts the terminal back the way it was."""
    global normal_terminal, live
    if normal_terminal is not None:
        termios.tcsetattr(0, termios.TCSANOW, normal_terminal)
    normal_terminal = None
    live = False


def read_key():
    """Returns the next key pressed. Returns "" for a key to ignore (arrows,
    function keys) and "\\n" when Ctrl+C is pressed or the input is closed."""
    if not live:
        key = sys.stdin.read(1)                   # input from a file or a pipe
    elif os.name == "nt":
        key = msvcrt.getwch()
        if key == "\x00" or key == "\xe0":        # arrow and function keys send two codes
            msvcrt.getwch()                       # throw the second one away
            return ""
        if key == "�":                       # there is no console to read from
            key = ""
    else:
        key = os.read(0, 1).decode("latin-1")
        if key == "\x1b":                         # Esc: arrow keys send Esc [ A in one burst
            while select.select([0], [], [], 0)[0]:
                os.read(0, 1)                     # throw the rest of the burst away
            return ""
    if key == "" or key == "\x03":
        return "\n"
    return key


def main():
    settings.menu()             # 1. pick the rotors and start values
    settings.setup()            # 2. build the machine
    settings.set_plugboard()    # 3. plug in the cables

    print("\nType your message. Press Enter when you are done.")
    print("> ", end="", flush=True)

    keyboard_on()
    try:
        key = read_key()
        while key != "\r" and key != "\n":
            if key.isascii() and key.isalpha():
                print(machine.encrypt(key), end="", flush=True)   # a letter goes through the machine
            elif " " <= key <= "~":
                print(key, end="", flush=True)                    # spaces and punctuation print as they are
            # anything else (Backspace, arrows) is ignored: the rotors can't turn back
            key = read_key()
    except KeyboardInterrupt:
        pass                    # Ctrl+C on Windows: finish like Enter
    finally:
        keyboard_off()

    print("\n\nWindow now, left to right:", chr(65 + machine.p3), chr(65 + machine.p2), chr(65 + machine.p1))
    print("To decrypt, run the program again with the same settings and type the letters above.")


main()
