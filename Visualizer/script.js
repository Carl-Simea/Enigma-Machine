// script.js : the same machine as the C++ version, plus the code that draws it.

// =====================================================================
// SETTINGS (the hardware and what you set before typing)
// =====================================================================

// Every letter is a number from 0 to 25 (A = 0, Z = 25).
const ROTOR = [
    [ 4,10,12, 5,11, 6, 3,16,21,25,13,19,14,22,24, 7,23,20,18,15, 0, 8, 1,17, 2, 9],  // I
    [ 0, 9, 3,10,18, 8,17,20,23, 1,11, 7,22,19,12, 2,16, 6,25,13,15,24, 5,21,14, 4],  // II
    [ 1, 3, 5, 7, 9,11, 2,15,17,19,23,21,25,13,24, 4, 8,22, 6, 0,10,12,20,18,16,14],  // III
    [ 4,18,14,21,15,25, 9, 0,24,16,20, 8,17, 7,23,11,13, 5,19, 6,10, 3, 2,12,22, 1],  // IV
    [21,25, 1,17, 6, 8,19,24,20,15,18, 3,13, 7,11,23, 0,22,12, 9,16,14, 5, 4, 2,10]   // V
];
const REFLECTOR_B = [24,17,20, 7,16,18,11, 3,15,23,13, 6,14,10,12, 8, 4, 1, 5,25, 2,22,21, 9, 0,19];
const NOTCH = [16, 4, 21, 9, 25];   // Q, E, V, J, Z: where rotors I-V turn their neighbor

const MAX_PAIRS = 10;     // plugboard cables that came with the machine
const STRONGER = true;    // true = real Enigma (rotate, then minus_1). false = simple version

const ROTOR_NAME = ["I", "II", "III", "IV", "V"];

// What is set on the machine right now. setup() reads these.
let rotor1 = 2, rotor2 = 1, rotor3 = 0;   // which rotor (0-4 means I-V) sits in slots 1, 2, 3
let start1 = 0, start2 = 0, start3 = 0;   // window start value (0-25) for slots 1, 2, 3
let pairs = [];                           // plugboard cables, for example [[0, 1], [16, 22]]

// =====================================================================
// MACHINE (no drawing happens here)
// =====================================================================

// Working state
let r1 = [], r2 = [], r3 = [];   // copies of the 3 chosen rotors; these rotate
let p1 = 0, p2 = 0, p3 = 0;      // positions 0-25, shown in the windows
let notch1 = 0, notch2 = 0;      // notch positions of the rotors in slots 1 and 2
let plug = [];                   // plugboard: plug[i] is the letter that i swaps with

// Shifts the array left one place, so r[0] moves to the end.
function rotate(r) {
    for (let i = 0; i < 25; i++) {
        const temp = r[i];
        r[i] = r[i + 1];
        r[i + 1] = temp;
    }
}

// Subtracts 1 from every value. On a real rotor the exit ends
// of the wires turn too, and this is what moves them.
function minus_1(r) {
    for (let i = 0; i < 26; i++) {
        r[i] = r[i] - 1;
        if (r[i] < 0) r[i] = 25;
    }
}

// One click of one rotor.
function turn(r) {
    rotate(r);
    if (STRONGER) minus_1(r);
}

// Runs before every letter. Rotor 1 always turns. A rotor sitting
// at its notch also turns the next rotor.
function step() {
    // Check both notches before turning anything
    const rotor1AtNotch = (p1 === notch1);
    const rotor2AtNotch = (p2 === notch2);

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
function find(r, x) {
    for (let k = 0; k < 26; k++) {
        if (r[k] === x) return k;
    }
    return -1;
}

// One keypress: the rotors move, then the letter makes the full trip.
// The same function encrypts and decrypts. c must be a letter.
function encrypt(c) {
    step();

    let i = c.toUpperCase().charCodeAt(0) - 65;   // letter to 0-25

    i = plug[i];            // plugboard, way in
    i = r1[i];              // no + p here: the arrays already moved
    i = r2[i];
    i = r3[i];
    i = REFLECTOR_B[i];
    i = find(r3, i);        // way back: search the current arrays
    i = find(r2, i);
    i = find(r1, i);
    i = plug[i];            // plugboard, way out

    return String.fromCharCode(65 + i);           // 0-25 back to a letter
}

// Builds the machine from the settings. Run it again with the same
// settings and the machine is back at the start, ready to decrypt.
function setup() {
    // Copy the chosen rotors, so the originals in ROTOR stay clean
    for (let k = 0; k < 26; k++) {
        r1[k] = ROTOR[rotor1][k];
        r2[k] = ROTOR[rotor2][k];
        r3[k] = ROTOR[rotor3][k];
        plug[k] = k;                    // every letter is itself until a cable says otherwise
    }

    // Turn each rotor to its start value
    for (let k = 0; k < start1; k++) turn(r1);
    for (let k = 0; k < start2; k++) turn(r2);
    for (let k = 0; k < start3; k++) turn(r3);
    p1 = start1;
    p2 = start2;
    p3 = start3;

    notch1 = NOTCH[rotor1];
    notch2 = NOTCH[rotor2];

    // Plug in the cables: store each pair both ways
    for (let n = 0; n < pairs.length; n++) {
        plug[pairs[n][0]] = pairs[n][1];
        plug[pairs[n][1]] = pairs[n][0];
    }
}

// =====================================================================
// THE PAGE (drawing, clicks and keys)
// =====================================================================

// The real machine's layout, used for the lamps, the keys and the plugboard
const ROWS = ["QWERTZUIO", "ASDFGHJK", "PYXCVBNML"];
const CABLE_COLORS = ["#c8553d", "#d9a441", "#5b8e7d", "#4f7cac", "#b07aa1",
                      "#e07b39", "#7fb069", "#c9ada7", "#8d99ae", "#e6c229"];
const MIN_LAMP_TIME = 250;   // a lamp stays on at least this long (milliseconds)

const lamps = {};      // lamps["A"] is the lamp element for A
const keys = {};       // keys["A"] is the key element for A
const sockets = [];    // sockets[0] is the plugboard socket for A

let typed = "";        // the paper strip
let lit = "";
let litLamp = null;    // the lamp that is on right now
let downKey = null;    // the key that is down right now
let heldLetter = "";   // the keyboard letter being held
let litSince = 0;
let offTimer = null;
let picked = -1;       // first socket of a cable that is half plugged in

const el = (id) => document.getElementById(id);

// ---- Building the machine on the page ----

// Fills a board with three rows of letters. make(letter) returns one element.
function buildRows(board, make) {
    for (const row of ROWS) {
        const rowElement = document.createElement("div");
        rowElement.className = "row";
        for (const letter of row) rowElement.appendChild(make(letter));
        board.appendChild(rowElement);
    }
}

function buildLamp(letter) {
    const lamp = document.createElement("span");
    lamp.className = "lamp";
    lamp.textContent = letter;
    lamps[letter] = lamp;
    return lamp;
}

function buildKey(letter) {
    const key = document.createElement("button");
    key.type = "button";
    key.className = "key";
    key.textContent = letter;
    key.tabIndex = -1;               // typing does the same job, so keys are not tab stops
    key.addEventListener("pointerdown", (event) => {
        event.preventDefault();
        pressKey(letter);
    });
    keys[letter] = key;
    return key;
}

function buildSocket(letter) {
    const index = letter.charCodeAt(0) - 65;
    const socket = document.createElement("button");
    socket.type = "button";
    socket.className = "socket";
    socket.innerHTML = '<span class="socket-letter">' + letter + "</span>" +
                       '<span class="socket-holes"></span>' +
                       '<span class="socket-partner"></span>';
    socket.addEventListener("click", () => socketClicked(index));
    sockets[index] = socket;
    return socket;
}

// One rotor slot: a rotor picker, the window, and two buttons to turn it.
function buildRotor(slot) {
    const rotor = document.createElement("div");
    rotor.className = "rotor";

    const picker = document.createElement("select");
    picker.id = "picker" + slot;
    picker.setAttribute("aria-label", "Rotor in slot " + slot);
    for (let n = 0; n < 5; n++) picker.add(new Option(ROTOR_NAME[n], n));
    picker.addEventListener("change", () => rotorPicked(slot, Number(picker.value)));

    const up = document.createElement("button");
    up.type = "button";
    up.className = "wheel";
    up.textContent = "▲";
    up.setAttribute("aria-label", "Turn slot " + slot + " forward");
    up.addEventListener("click", () => wheelTurned(slot, 1));

    const windowBox = document.createElement("div");
    windowBox.className = "window";
    windowBox.innerHTML = '<span class="window-number" id="number' + slot + '"></span>';

    const down = document.createElement("button");
    down.type = "button";
    down.className = "wheel";
    down.textContent = "▼";
    down.setAttribute("aria-label", "Turn slot " + slot + " back");
    down.addEventListener("click", () => wheelTurned(slot, -1));

    const label = document.createElement("span");
    label.className = "rotor-label";
    label.textContent = "Slot " + slot;

    rotor.append(picker, up, windowBox, down, label);
    return rotor;
}

// ---- Showing the state of the machine ----

function showWindows() {
    const positions = [0, p1, p2, p3];
    const rotors = [0, rotor1, rotor2, rotor3];
    for (let slot = 1; slot <= 3; slot++) {
        const number = el("number" + slot);
        const text = String(positions[slot]);     // 0-25, as on the rotor's number ring
        if (number.textContent !== text) {
            number.textContent = text;
            number.classList.remove("tick");
            void number.offsetWidth;          // restarts the little turning animation
            number.classList.add("tick");
        }
        el("picker" + slot).value = rotors[slot];
    }
}

function showTape() {
    el("typedText").textContent = typed;
    el("lampText").textContent = lit;
}

function showPlugboard() {
    for (let i = 0; i < 26; i++) {
        sockets[i].classList.remove("plugged", "picked");
        sockets[i].style.removeProperty("--cable");
        sockets[i].querySelector(".socket-partner").textContent = "";
        sockets[i].setAttribute("aria-label", "Socket " + String.fromCharCode(65 + i) + ", free");
    }
    for (let n = 0; n < pairs.length; n++) {
        for (let side = 0; side < 2; side++) {
            const mine = pairs[n][side];
            const other = String.fromCharCode(65 + pairs[n][1 - side]);
            sockets[mine].classList.add("plugged");
            sockets[mine].style.setProperty("--cable", CABLE_COLORS[n]);
            sockets[mine].querySelector(".socket-partner").textContent = other;
            sockets[mine].setAttribute("aria-label",
                "Socket " + String.fromCharCode(65 + mine) + ", cabled to " + other);
        }
    }
    if (picked !== -1) sockets[picked].classList.add("picked");
    el("cableCount").textContent = pairs.length + " of " + MAX_PAIRS + " cables";
    drawCables();
}

// Draws each cable as a curve that sags between its two sockets.
function drawCables() {
    const svg = el("cables");
    const board = el("plugboard").getBoundingClientRect();
    let paths = "";
    for (let n = 0; n < pairs.length; n++) {
        const a = sockets[pairs[n][0]].querySelector(".socket-holes").getBoundingClientRect();
        const b = sockets[pairs[n][1]].querySelector(".socket-holes").getBoundingClientRect();
        const x1 = a.left + a.width / 2 - board.left, y1 = a.top + a.height / 2 - board.top;
        const x2 = b.left + b.width / 2 - board.left, y2 = b.top + b.height / 2 - board.top;
        const sag = Math.max(y1, y2) + 18 + Math.abs(x2 - x1) * 0.12;
        paths += '<path d="M' + x1 + " " + y1 + " Q" + (x1 + x2) / 2 + " " + sag + " " + x2 + " " + y2 +
                 '" stroke="' + CABLE_COLORS[n] + '"/>';
    }
    svg.innerHTML = paths;
}

function say(message) {
    el("status").textContent = message;
}

// Rebuilds the machine from the settings and clears the paper strip.
function restart(message) {
    setup();
    releaseNow();
    typed = "";
    lit = "";
    showWindows();
    showTape();
    showPlugboard();
    say(message);
}

// ---- Pressing and releasing a key ----

function pressKey(letter) {
    releaseNow();                         // only one lamp can be on
    const out = encrypt(letter);

    downKey = keys[letter];
    downKey.classList.add("down");
    litLamp = lamps[out];
    litLamp.classList.add("lit");
    litSince = Date.now();

    typed += letter;
    lit += out;
    showWindows();
    showTape();
}

// The key comes back up. The lamp goes off with it, but never
// so fast that a quick tap can't be seen.
function releaseKey() {
    if (downKey) downKey.classList.remove("down");
    downKey = null;
    heldLetter = "";
    const wait = MIN_LAMP_TIME - (Date.now() - litSince);
    clearTimeout(offTimer);
    if (wait > 0) offTimer = setTimeout(lampOff, wait);
    else lampOff();
}

function lampOff() {
    if (litLamp) litLamp.classList.remove("lit");
    litLamp = null;
}

function releaseNow() {
    clearTimeout(offTimer);
    if (downKey) downKey.classList.remove("down");
    downKey = null;
    lampOff();
}

// ---- Changing the settings ----

// A wheel was turned: the numbers in the windows become the new start values.
function wheelTurned(slot, direction) {
    start1 = p1;
    start2 = p2;
    start3 = p3;
    if (slot === 1) start1 = (start1 + direction + 26) % 26;
    if (slot === 2) start2 = (start2 + direction + 26) % 26;
    if (slot === 3) start3 = (start3 + direction + 26) % 26;
    restart("Start position set. Type your message.");
}

// A rotor was picked for a slot. If that rotor is already in
// another slot, the two rotors trade places.
function rotorPicked(slot, rotor) {
    const before = [0, rotor1, rotor2, rotor3];
    const now = before.slice();
    for (let other = 1; other <= 3; other++) {
        if (other !== slot && before[other] === rotor) now[other] = before[slot];
    }
    now[slot] = rotor;
    rotor1 = now[1];
    rotor2 = now[2];
    rotor3 = now[3];
    restart("Rotors, left to right: " + ROTOR_NAME[rotor3] + " " + ROTOR_NAME[rotor2] + " " + ROTOR_NAME[rotor1] + ".");
}

// Returns which cable uses this socket, or -1 if the socket is free.
function cableAt(socket) {
    for (let n = 0; n < pairs.length; n++) {
        if (pairs[n][0] === socket || pairs[n][1] === socket) return n;
    }
    return -1;
}

function socketClicked(socket) {
    const cable = cableAt(socket);

    if (cable !== -1) {                           // a plugged socket: pull that cable out
        pairs.splice(cable, 1);
        picked = -1;
        restart("Cable removed. The rotors are back at their start position.");
    } else if (picked === -1) {                   // first end of a new cable
        if (pairs.length >= MAX_PAIRS) {
            say("All " + MAX_PAIRS + " cables are in use. Click a plugged letter to pull its cable out.");
            return;
        }
        picked = socket;
        showPlugboard();
        say("Now click the letter to swap " + String.fromCharCode(65 + socket) + " with.");
    } else if (picked === socket) {               // same socket again: put the plug down
        picked = -1;
        showPlugboard();
        say("To decrypt, press Reset and type the output letters.");
    } else {                                      // second end: the cable is in
        pairs.push([picked, socket]);
        const names = String.fromCharCode(65 + picked) + " and " + String.fromCharCode(65 + socket);
        picked = -1;
        restart(names + " are swapped. The rotors are back at their start position.");
    }
}

// ---- Start ----

const rotorsBox = el("rotors");
for (let slot = 3; slot >= 1; slot--) rotorsBox.appendChild(buildRotor(slot));   // slot 3 is on the left
buildRows(el("lampboard"), buildLamp);
buildRows(el("keyboard"), buildKey);
buildRows(el("plugboard"), buildSocket);

window.addEventListener("pointerup", releaseKey);
window.addEventListener("pointercancel", releaseKey);
window.addEventListener("resize", drawCables);

// The computer keyboard works like the keys on the machine
document.addEventListener("keydown", (event) => {
    if (event.ctrlKey || event.metaKey || event.altKey) return;
    if (event.target.tagName === "SELECT") return;        // let the rotor picker use its keys

    if (/^[a-zA-Z]$/.test(event.key)) {
        event.preventDefault();
        if (event.repeat) return;                         // holding a key types it once
        heldLetter = event.key.toUpperCase();
        pressKey(heldLetter);
    } else if (event.key === " " && event.target.tagName !== "BUTTON") {
        event.preventDefault();                           // a space goes on the strip; the rotors don't move
        typed += " ";
        lit += " ";
        showTape();
    }
});

// After a mouse click on a button, let go of it. Otherwise the next
// space typed would press that button again. (detail is 0 for keyboard clicks.)
document.addEventListener("click", (event) => {
    if (event.detail > 0 && document.activeElement.tagName === "BUTTON") document.activeElement.blur();
});

document.addEventListener("keyup", (event) => {
    if (event.key.toUpperCase() === heldLetter) releaseKey();
});

el("resetButton").addEventListener("click", () => {
    restart("Rotors are back at their start position. Type the output letters to decrypt.");
});

restart("To decrypt, press Reset and type the output letters.");