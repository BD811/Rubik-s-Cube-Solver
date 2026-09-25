#include "Cube.h"
#include "Solver.h"
#include <iostream>
#include <regex>

using namespace rubik;

static std::string field(const std::string& input, const std::string& name) {
    std::regex pattern("\\\"" + name + "\\\"\\s*:\\s*\\\"([^\\\"]*)\\\""); std::smatch match;
    return std::regex_search(input, match, pattern) ? match[1].str() : "";
}
static std::string quote(const std::string& value) { std::string escaped; for (char c : value) escaped += c == '\"' ? "\\\"" : std::string(1, c); return "\"" + escaped + "\""; }
static std::string movesJson(const std::vector<std::string>& moves) { std::string output = "["; for (size_t i = 0; i < moves.size(); ++i) { if (i) output += ","; output += quote(moves[i]); } return output + "]"; }

int main() {
    std::string input;
    while (std::getline(std::cin, input)) {
        try {
            std::string cubeState = field(input, "cube"); Cube cube(cubeState); std::string error;
            bool valid = cube.isValid(error);
            if (input.find("validate") != std::string::npos) {
                std::cout << "{\"solvable\":" << (valid ? "true" : "false") << ",\"message\":" << quote(error) << "}\n";
                continue;
            }
            if (!valid) { std::cout << "{\"solvable\":false,\"error\":" << quote(error) << "}\n"; continue; }
            SolveResult result = solveBidirectional(cube, 12, 2000000);
            std::cout << "{\"solvable\":" << (result.solved ? "true" : "false") << ",\"moves\":" << movesJson(result.moves)
                      << ",\"moveCount\":" << result.moves.size() << ",\"statesExplored\":" << result.statesExplored
                      << ",\"maxQueueSize\":" << result.maxQueueSize << ",\"milliseconds\":" << result.milliseconds;
            if (!result.error.empty()) std::cout << ",\"error\":" << quote(result.error);
            std::cout << "}\n";
        } catch (const std::exception& exception) { std::cout << "{\"solvable\":false,\"error\":" << quote(exception.what()) << "}\n"; }
    }
}
