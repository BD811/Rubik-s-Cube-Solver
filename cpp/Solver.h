#pragma once
#include "Cube.h"
#include <string>
#include <vector>

namespace rubik {
struct SolveResult { bool solved = false; std::vector<std::string> moves; size_t statesExplored = 0; size_t maxQueueSize = 0; double milliseconds = 0; std::string error; };
SolveResult solveBidirectional(const Cube& start, int maxDepth = 8, size_t stateLimit = 500000);
}
