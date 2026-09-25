#include "Solver.h"
#include <chrono>
#include <deque>
#include <unordered_map>

namespace rubik {
namespace {
struct Node { Cube cube; std::vector<std::string> path; };
bool allowed(const std::string& previous, const std::string& next) { return previous.empty() || previous[0] != next[0]; }
}
SolveResult solveBidirectional(const Cube& start, int maxDepth, size_t stateLimit) {
    SolveResult result; auto began = std::chrono::steady_clock::now();
    if (start.isSolved()) { result.solved = true; return result; }
    std::string error; if (!start.isValid(error)) { result.error = error; return result; }
    std::deque<Node> left{{start, {}}}; std::deque<Node> right{{Cube(), {}}};
    std::unordered_map<std::string, std::vector<std::string>> leftSeen{{start.getState(), {}}};
    std::unordered_map<std::string, std::vector<std::string>> rightSeen{{Cube().getState(), {}}};
    auto finish = [&]() { result.milliseconds = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - began).count(); };
    while (!left.empty() && !right.empty() && result.statesExplored < stateLimit) {
        bool expandLeft = left.size() <= right.size(); auto& queue = expandLeft ? left : right; auto& own = expandLeft ? leftSeen : rightSeen; auto& other = expandLeft ? rightSeen : leftSeen;
        size_t level = queue.size();
        while (level-- && result.statesExplored < stateLimit) {
            Node current = std::move(queue.front()); queue.pop_front(); ++result.statesExplored;
            if (static_cast<int>(current.path.size()) >= maxDepth / 2) continue;
            for (const auto& move : allMoves()) {
                if (!allowed(current.path.empty() ? "" : current.path.back(), move)) continue;
                Cube next = current.cube; next.applyMove(move); std::string state = next.getState();
                if (own.find(state) != own.end()) continue;
                auto path = current.path; path.push_back(move); own[state] = path;
                auto meeting = other.find(state);
                if (meeting != other.end()) {
                    result.moves = expandLeft ? path : meeting->second;
                    if (expandLeft) for (auto it = meeting->second.rbegin(); it != meeting->second.rend(); ++it) result.moves.push_back(inverseMove(*it));
                    else { result.moves = meeting->second; for (auto it = path.rbegin(); it != path.rend(); ++it) result.moves.push_back(inverseMove(*it)); }
                    Cube check = start; check.applyMoves(result.moves);
                    if (check.isSolved()) result.solved = true; else result.error = "Internal error: reconstructed path did not solve the cube.";
                    finish(); result.maxQueueSize = std::max(left.size(), right.size()); return result;
                }
                queue.push_back({next, std::move(path)}); result.maxQueueSize = std::max(result.maxQueueSize, left.size() + right.size());
            }
        }
    }
    result.error = result.statesExplored >= stateLimit ? "Search limit reached. Try a shorter scramble." : "No solution found within the configured search depth."; finish(); return result;
}
} // namespace rubik
