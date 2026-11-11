

# Design notes

How this Enigma came together, from watching a video on how the machine works to a working version in four languages. I came up with the structure. Claude (Anthropic's AI assistant) reviewed each idea, and where it was wrong, explained why and what works instead.

**Try it in your browser:** [Enigma visualizer](https://carl-simea.github.io/Enigma-Machine/visualizer/)

## My starting idea

Three rotors, each set to a number from 1 to 26, a reflector, and the letter going through the rotors, into the reflector and back out. A menu to choose the settings, a plugboard to swap letters, and output that shows up as you type and stops when you press Enter.

## Decisions

| Idea | Whose | What happened |
| --- | --- | --- |
| Rotors as Caesar shifts (+2, +3), reflector as +9 | Mine | Corrected. Shifts cancel out on the way back, and +9 can't be reversed, so the machine collapses into one fixed cipher. Changed to scrambled arrays using the real wirings, with the reflector swapping letters in pairs. |
| Count how many times a key is pressed to decide how far the letter goes | Mine | Corrected. Every keypress takes the full trip. What changes between presses is where the rotors sit. |
| Each rotor function calls the next one | Mine | Corrected. The reflector would call rotor 3, which would call the reflector, forever. Changed to one `encrypt()` that calls each step in order. |
| Use numbers 0 to 25 inside the machine, convert letters only at the edges | Mine | Kept. |
| Turn a rotor by moving the array itself one place, instead of adding to the value | Mine | Kept. This became `rotate()`. |
| Rotate by swapping neighbors (from bubble sort) | Mine | Kept, without bubble sort's comparison. |
| Subtract 1 from every value when a rotor turns | Claude's correction | Added as `minus_1()`. On a real rotor the exit contacts turn with it. |
| Find the way back with a search instead of a second array | Claude's correction | Added as `find()`. |
| Turn rotor 2 with a counter (`% 26`) | Mine | Refined into notch letters with the double step. Claude explained how the real machine does it; I chose to match it. |
| Menu to set the rotors and plugboard | Mine | Kept. |
| Plugboard limited to 10 cables, like the original | Mine | Kept. |
| Show the free letters while choosing plugboard pairs | Mine | Kept. Claude added rejecting a letter that's already used, which would break decryption. |
| No separate decrypt function | Mine | Kept. The reflector makes the machine its own reverse. |
| Split into main, machine and settings files, like a phone and its settings screen | Mine | Kept. |
| Visualizer that looks like the real machine, with keys you can click | Mine | Built. Rotor windows show numbers 0 to 25, and the output appears under what you typed. |

## Things I worked through along the way

**Why a simple cipher breaks the machine.** My first plan used Caesar shifts for the rotors. A shift looks the same from every position, so turning the rotor changes nothing, and the way back undoes the way in. The whole machine shrinks to a single shift. The reflector taught me the same lesson from the other side: it has to swap letters in pairs (if A becomes Y, Y must become A), or decryption fails. That pairing is also why the same settings both encrypt and decrypt, and why no letter can ever encrypt to itself.

**Letters as numbers.** Every `char` is secretly a number: 'A' is 65 and 'Z' is 90. `c - 'A'` turns a letter into 0 to 25, and `'A' + i` turns it back. Lowercase letters have their own codes ('a' is 97), so `toupper()` comes first. Keeping everything as 0 to 25 inside the machine means array indexes and `% 26` just work.

**Turning a rotor.** I turn a rotor by moving the array itself one place, by swapping neighbors. That needs a temporary variable, because `a = b; b = a;` only copies. The loop stops at `i < 25` because it also touches `i + 1`, while `minus_1()` runs to `i < 26`. `minus_1()` exists because on a real rotor the exit contacts turn too. I also learned that `%` in C++ can return a negative number, so wrapping back sometimes needs `+ 26`.

**Program structure.** Having functions call each other in a chain would have looped forever, so one function calls every step in order. Splitting the code into files taught me what a header is for. A `.h` declares what exists and a `.cpp` holds the code. `#pragma once` stops a header from being pasted in twice, and `extern` lets several files share one variable without each getting its own copy. You compile the `.cpp` files and never include one. C needs `static` on constant arrays in a header, and Python has no headers at all: `import` runs a module once and reuses it.

**Reading keys live.** The terminal normally holds every key until Enter. To show each letter as it's typed, Windows uses `_getch()`, and macOS and Linux use `termios` with line buffering and echo turned off. The terminal settings have to be restored afterward (`stty sane` fixes it if they aren't). Windows sends `'\r'` for Enter, arrow keys send two codes, output needs flushing to show up right away, and Backspace can't undo a letter, because the rotors have already turned.

**Choosing the right tool.** A plain C++ array can't be resized or copied with `=`, so the plugboard's list of free letters is a `std::string` with `find()` and `erase()`. `encrypt()` takes a `char`, because the machine works one letter at a time. For the visuals, JavaScript in the browser was simpler than graphics in C++, and a database was overkill for 130 numbers.

## Testing

Each version passed the same checklist: AAAAA at AAA gives BDZGO, the double step, the carry, the round trip, and no letter encrypting to itself. Each also matched py-enigma on hundreds of random messages with random rotors, start positions and plugboards.