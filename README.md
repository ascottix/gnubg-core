# GnuBG Core

A highly stripped-down version of the [GNU Backgammon](http://www.gnu.org/software/gnubg/) backgammon engine, designed to be compiled to **WebAssembly** and used directly in the browser.

- ✅ Only the core logic needed to play and analyze backgammon
- 🎉 No dependency on GLib
- ⚡ Super-fast, even in the browser
- 🛠️ Compilable with [Emscripten](https://emscripten.org/)
- 💡 Minimal JS interface — `hint()` is all you need
- 🎯 Includes bearoff databases

## ✨ Why this project?

The goal is to bring the power of GnuBG to the **web**, in a lightweight, modular form — ideal for:
- building custom UIs
- developing educational tools or analysis apps
- supporting interactive projects, even on mobile devices

Of course, you can also use it as a C library in your own applications.

## 📦 What's Included

This version contains:
- the GNU Backgammon evaluation engine
- code to represent and evaluate positions
- a simplified, high level interface to just compute the best action (`hint`)

## 🚫 What's Missing?

The following features didn't make the cut:
- multithreading support (not a big deal unless you need deep rollouts)
- pluggable MET's (has [Kazaross-XG2](https://bkgm.com/articles/Keith/KazarossXG2MET/index.html) built-in)
- pluggable RNG's (has ISAAC and MD5 builtin)

## 🚀 Usage

You can use this module from JavaScript as follows:

```js
import { initGnubgCore } from './gnubg-core.js';

const gnuBgCore = await initGnubgCore();
const evalDepth = 1; // Number of plies to search, starts from 0, usually you don't need more than 2 or 3

function test(xgid, expect) {
    const res = gnuBgCore.hint(xgid, evalDepth); // See below for the response description
    console.log(`Eval depth=${evalDepth}, expect: ${expect}`);
    console.log(res); // Check res.action against the expectation
}

// Examples
test("XGID=aa--BBBB----dE---d-e----B-:0:0:1:00:0:0:0:0:10", "double, drop");
test("XGID=aBaB--C-A---dE--ac-e----B-:0:0:1:00:0:0:0:0:10", "roll");
test("XGID=aBaB--C-A---dE--ac-e----B-:0:0:1:42:0:0:0:0:10", "play 8/4 6/4");
test("XGID=aa--BBBB----dE---d-e----B-:0:0:1:D:0:0:0:0:10", "drop");
```

### Example

Check `web/gnubg-core-demo.html` for a more comprehensive example.

## API

The module exposes the following functions:
- hint
- shutdown

### 📋 hint()

The `hint()` function returns a plain JavaScript object with the following structure:

```json
{
  "action": "...",     // The suggested action: "play", "double", "take", "drop", or "roll"
  "data": { ... }      // Evaluation data that supports the decision (structure depends on the action)
}
```

### 🎲 Actions and data structure

There are two kinds of actions:

1. *Cube-related actions*

These include:

- "double": suggest offering the cube
- "take": suggest accepting a double
- "drop": suggest declining a double
- "roll": suggest rolling the dice (i.e. don't double)

In these cases, data has this structure:

```json
{
  "cd": <int>,          // Cube decision enum (see below)
  "equity": [           // Equity values for different cube outcomes
    <no double>,        // 0: equity if not doubling
    <double, take>,     // 1: equity if doubling and taken
    <double, drop>,     // 2: equity if doubling and dropped
    <best>              // 3: optimal equity (equal to one of the above)
  ]
}
```

For example:

```json
{
  "xgid": "XGID=aa--BBBB----dE---d-e----B-:0:0:1:00:0:0:0:0:10",
  "action": "double",
  "data": {
    "cd": 1,
    "equity": [
      0.9,
      1.0728,
      1.0,
      1.0
    ]
  }
}
```

The cd value is an integer code for the cube decision. Possible values are:

- 0 = double, take
- 1 = double, pass
- 2 = no double, take
- 3 = too good, take
- 4 = too good, pass
- 5 = double, beaver
- 6 = no double, beaver
- 7 = redouble, take
- 8 = redouble, pass
- 9 = no redouble, take
- 10 = too good to redouble, take
- 11 = too good to redouble, pass
- 12 = no redouble, beaver
- 13 = no double, dead cube (match play only)
- 14 = no redouble, dead cube (match play only)
- 15 = not available
- 16 = optional double, take
- 17 = optional redouble, take
- 18 = optional double beaver
- 19 = optional double, pass
- 20 = optional redouble, pass

2. *Checker play actions*

When the action is `"play"`, the `data` field contains an **array of moves**, ordered from best to worst (the list is limited to the top 40 moves).

Each element in the array has the following structure:

```json
{
  "move": "8/4 6/4",           // Move in standard notation
  "equity": [                  // Equity values
    0.5899,                    // 0: cubeful equity (main)
    0.3905                     // 1: cubeless equity (secondary)
  ],
  "eval": [                    // Winning probability
    0.6049,                    // 0: win
    0.2732,                    // 1: gammon win
    0.0112,                    // 2: backgammon win
    0.0989,                    // 3: gammon loss
    0.0047                     // 4: backgammon loss
  ]
}
```

### 📋 shutdown()

Releases all resources used by the module and terminates it.

After calling `shutdown()`, the module can no longer be used.

## 🛠️ Build Instructions

This library can be compiled both with gcc and Emscripten.

### GCC

Run:

```bash
make
```

### Emscripten

Install [Emscripten](https://emscripten.org/) and [activate the environment](https://emscripten.org/docs/getting_started/downloads.html#installation-instructions-using-the-emsdk-recommended). Then run:

```bash
make -f Makefile.emcc
```

It will create three files:

- the WebAssembly (.wasm)
- an archive with the required support files (.data)
- the JavaScript module loader (.js)

**Note**: the Emscripten module exports a very low-level interface, check the API described above for a much more user-friendly interface.

## 💬 Credits

Thanks to all the original contributors to GnuBG. I hope this project helps make their brilliant work available to more developers and users.

## 🔒 License

This project is derived from GnuBG, which is licensed under GPL-3.0. All modifications in this version remain under the same license.

Copyright © 2025 Alessandro Scotti — Stripped it down, cleaned it up, and made it run in your browser.
