# Orbit Cube Solver

A portfolio-quality 3x3 Rubik's Cube workbench with a vanilla HTML/CSS/JavaScript interface, an Express REST bridge, and a C++ cube engine. The browser lets you paint a cube state, validate it, generate an eight-move scramble, request a solution, and play the returned moves one at a time.

## Architecture

```text
Frontend (HTML + CSS + JS) -> Express API -> C++ JSON-lines CLI
```

The Node process launches the C++ executable for each request. This keeps the algorithm independently testable and makes the boundary easy to explain in an interview.

## Algorithm

The C++ `Cube` class stores all 54 facelets in the fixed order `U, D, F, B, L, R`, nine stickers per face. Each sticker also has a 3D position and outward normal. A face turn rotates the affected layer and maps every sticker to its destination coordinate, so the six faces share one move implementation.

The validator checks:

- exactly nine of each `W Y G B O R` color;
- fixed centers in the documented orientation;
- each corner and edge is a known cubie with no duplicate;
- corner orientation sum modulo 3;
- edge orientation sum modulo 2;
- corner and edge permutation parity.

The solver performs bidirectional BFS. One frontier starts at the submitted state and one starts at the solved state. Separate visited maps retain paths, the smaller frontier is expanded, immediate same-face repetitions are pruned, and the first shared state reconstructs the path. The resulting moves are applied to a copy of the input before the solver reports success.

```text
scrambled cube        solved cube
      |                    |
   BFS queue            BFS queue
      \                  /
       \-- meeting -----/
             |
      reconstruct paths
```

This implementation intentionally uses a bounded search (`maxDepth = 8`, `stateLimit = 500,000`). It is designed to make the algorithm visible and reliable for short educational scrambles, not to pretend that unrestricted BFS makes the full 3x3 state space small.

## Run locally

Prerequisites: a C++17 compiler and Node.js 18+.

```powershell
# From the repository root
g++ -std=c++17 -O2 cpp\Cube.cpp cpp\Solver.cpp cpp\main.cpp -o build\solver.exe
g++ -std=c++17 -O2 cpp\Cube.cpp cpp\Solver.cpp tests\cube_tests.cpp -o build\cube_tests.exe
.\build\cube_tests.exe
npm install
npm start
```

### Windows with Device Guard

If Windows blocks unsigned `.exe` files, use the installed WSL2 Ubuntu environment. The backend automatically invokes the same C++ solver through WSL on Windows:

```powershell
npm run build:cpp:wsl
npm.cmd start
```

The WSL build creates `build/solver-wsl` and `build/cube-tests-wsl` on the shared project folder. To run the tests inside Ubuntu, use `wsl ./build/cube-tests-wsl` from the project root.

Open `http://localhost:3000`. On systems where the executable is elsewhere, set `SOLVER_PATH` before starting the server.

### Windows Device Guard / AppLocker

The C++ CLI is a locally compiled unsigned executable. If the API reports that Windows blocked the solver, Device Guard or AppLocker is preventing native binaries from running. This cannot be fixed by changing Node's `spawnSync` options. Ask the machine administrator to allow the generated `build\solver.exe`, or run the project on a development machine without that application-control rule, then restart `npm.cmd start`.

## API

`POST /api/validate` and `POST /api/solve` accept:

```json
{"cube":"WWWWWWWWWYYYYYYYYYGGGGGGGGGBBBBBBBBBOOOOOOOOORRRRRRRRR"}
```

The value must be exactly 54 stickers using `W Y G B O R`. A solve response includes `moves`, `moveCount`, `statesExplored`, `maxQueueSize`, and `milliseconds`. The frontend displays those measurements as development telemetry.

## Complexity and limits

A Rubik's Cube has roughly $4.3 \times 10^{19}$ reachable states. With branching factor 18, ordinary BFS grows exponentially. Bidirectional BFS reduces a depth-$d$ search to roughly two searches of depth $d/2$, but memory still grows exponentially and duplicate states consume substantial space. The current facelet representation uses more memory than a cubie-coordinate or packed representation. Practical next steps are IDA*, pattern databases, stronger canonical pruning, packed cubie coordinates, and parallel frontier expansion.

## Test coverage

The C++ test target covers solved detection, all six four-quarter-turn identities, every move/inverse pair, serialization through state comparisons, a short scramble, solver reconstruction, and final solved-state verification. More malformed-state fixtures should be added as the validator evolves, especially for each individual orientation and parity failure.
