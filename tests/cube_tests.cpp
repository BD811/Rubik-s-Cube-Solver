#include "../cpp/Cube.h"
#include "../cpp/Solver.h"
#include <cassert>
#include <iostream>

using namespace rubik;
int main() {
    Cube solved; assert(solved.isSolved()); std::string message; if (!solved.isValid(message)) { std::cerr << message << "\n"; return 1; }
    for (const auto& face : {"U", "D", "L", "R", "F", "B"}) { Cube cube; for (int i = 0; i < 4; ++i) cube.applyMove(face); assert(cube.getState() == solved.getState()); }
    for (const auto& move : allMoves()) { Cube cube; cube.applyMove(move); cube.applyMove(inverseMove(move)); assert(cube.getState() == solved.getState()); }
    for (const auto& move : {"U", "D", "L", "R", "F", "B"}) { Cube cube; if (!cube.isValid(message)) { std::cerr << move << ": " << message << "\n"; return 1; } cube.applyMove(move); if (!cube.isValid(message)) { std::cerr << move << ": " << message << "\n"; return 1; } }
    std::string invalidState = solved.getState(); invalidState[0] = 'Y'; Cube invalid(invalidState); assert(!invalid.isValid(message));
    Cube scrambled; scrambled.applyMoves({"R", "U", "R'", "U'"}); assert(!scrambled.isSolved()); if (!scrambled.isValid(message)) { std::cerr << message << "\n"; return 1; }
    const std::vector<std::string> browserScramble = {"D2", "B'", "F'", "U2", "L'", "U", "R", "B'"};
    Cube longer; for (const auto& move : browserScramble) { longer.applyMove(move); if (!longer.isValid(message)) { std::cerr << move << ": " << message << "\n"; return 1; } }
    auto result = solveBidirectional(scrambled, 10, 500000); if (!result.solved) { std::cerr << result.error << "\n"; return 1; } scrambled.applyMoves(result.moves); assert(scrambled.isSolved());
    std::cout << "All cube tests passed\n";
}
