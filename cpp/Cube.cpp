#include "Cube.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <map>
#include <set>
#include <stdexcept>

namespace rubik {
namespace {
struct Vec { int x; int y; int z; };
struct StickerLocation { Vec position; Vec normal; };

const std::array<char, 6> kColors = {'W', 'Y', 'G', 'B', 'O', 'R'};
const std::array<char, 6> kFaces = {'U', 'D', 'F', 'B', 'L', 'R'};

Vec normalForFace(char face) {
    switch (face) {
        case 'U': return {0, 1, 0}; case 'D': return {0, -1, 0};
        case 'F': return {0, 0, 1}; case 'B': return {0, 0, -1};
        case 'L': return {-1, 0, 0}; default: return {1, 0, 0};
    }
}

StickerLocation locationFor(int index) {
    int face = index / 9, local = index % 9;
    int row = local / 3, col = local % 3;
    switch (kFaces[face]) {
        case 'U': return {{col - 1, 1, row - 1}, {0, 1, 0}};
        case 'D': return {{col - 1, -1, 1 - row}, {0, -1, 0}};
        case 'F': return {{col - 1, 1 - row, 1}, {0, 0, 1}};
        case 'B': return {{1 - col, 1 - row, -1}, {0, 0, -1}};
        case 'L': return {{-1, 1 - row, col - 1}, {-1, 0, 0}};
        default: return {{1, 1 - row, 1 - col}, {1, 0, 0}};
    }
}

Vec rotate(Vec value, char axis, int direction) {
    // direction is +1 or -1 for a right-handed quarter turn.
    for (int i = 0; i < (direction < 0 ? 3 : 1); ++i) {
        if (axis == 'x') value = {value.x, -value.z, value.y};
        if (axis == 'y') value = {value.z, value.y, -value.x};
        if (axis == 'z') value = {-value.y, value.x, value.z};
    }
    return value;
}

int indexFor(const StickerLocation& target) {
    for (int i = 0; i < Cube::StickerCount; ++i) {
        auto location = locationFor(i);
        if (location.position.x == target.position.x && location.position.y == target.position.y &&
            location.position.z == target.position.z && location.normal.x == target.normal.x &&
            location.normal.y == target.normal.y && location.normal.z == target.normal.z) return i;
    }
    throw std::logic_error("Sticker coordinate lookup failed");
}

const std::array<std::array<int, 3>, 8> kCorners = {{{8, 20, 45}, {2, 47, 27}, {0, 29, 36}, {6, 38, 18},
    {11, 26, 51}, {17, 53, 33}, {15, 35, 42}, {9, 44, 24}}};
const std::array<std::array<int, 2>, 12> kEdges = {{{7, 19}, {5, 46}, {1, 28}, {3, 37}, {10, 25}, {14, 52},
    {16, 34}, {12, 43}, {23, 48}, {30, 50}, {32, 39}, {21, 41}}};
const std::array<std::array<char, 3>, 8> kCornerColors = {{{'W','G','R'}, {'W','R','B'}, {'W','B','O'}, {'W','O','G'},
    {'Y','G','R'}, {'Y','R','B'}, {'Y','B','O'}, {'Y','O','G'}}};
const std::array<std::array<char, 2>, 12> kEdgeColors = {{{'W','G'}, {'W','R'}, {'W','B'}, {'W','O'}, {'Y','G'}, {'Y','R'},
    {'Y','B'}, {'Y','O'}, {'G','R'}, {'B','R'}, {'B','O'}, {'G','O'}}};

bool sameSet(const std::array<char, 3>& a, const std::array<char, 3>& b) {
    auto left = a, right = b; std::sort(left.begin(), left.end()); std::sort(right.begin(), right.end()); return left == right;
}
bool sameSet(const std::array<char, 2>& a, const std::array<char, 2>& b) {
    return (a[0] == b[0] && a[1] == b[1]) || (a[0] == b[1] && a[1] == b[0]);
}
} // namespace

Cube::Cube() {
    for (int face = 0; face < 6; ++face) for (int i = 0; i < 9; ++i) stickers_[face * 9 + i] = kColors[face];
}

Cube::Cube(const std::string& state) {
    if (state.size() != StickerCount) throw std::invalid_argument("Cube state must contain 54 stickers");
    std::copy(state.begin(), state.end(), stickers_.begin());
}

bool Cube::isSolved() const {
    for (int face = 0; face < 6; ++face) for (int i = 0; i < 9; ++i) if (stickers_[face * 9 + i] != kColors[face]) return false;
    return true;
}

std::string Cube::getState() const { return std::string(stickers_.begin(), stickers_.end()); }

void Cube::rotateLayer(char face, int quarterTurns) {
    char axis = (face == 'U' || face == 'D') ? 'y' : (face == 'F' || face == 'B') ? 'z' : 'x';
    Vec normal = normalForFace(face);
    int layer = axis == 'x' ? normal.x : axis == 'y' ? normal.y : normal.z;
    for (int turn = 0; turn < quarterTurns; ++turn) {
        auto old = stickers_;
        for (int i = 0; i < StickerCount; ++i) {
            auto source = locationFor(i);
            int coordinate = axis == 'x' ? source.position.x : axis == 'y' ? source.position.y : source.position.z;
            if (coordinate != layer) continue;
            StickerLocation destination{rotate(source.position, axis, -1), rotate(source.normal, axis, -1)};
            stickers_[indexFor(destination)] = old[i];
        }
    }
}

void Cube::applyMove(const std::string& move) {
    if (move.empty()) throw std::invalid_argument("Empty move");
    int turns = move.size() > 1 && move[1] == '2' ? 2 : move.size() > 1 && move[1] == '\'' ? 3 : 1;
    rotateLayer(move[0], turns);
}
void Cube::applyMoves(const std::vector<std::string>& moves) { for (const auto& move : moves) applyMove(move); }

bool Cube::isValid(std::string& error) const {
    std::map<char, int> counts; for (char color : stickers_) ++counts[color];
    for (char color : kColors) if (counts[color] != 9) { error = "Invalid number of colors: each color must appear exactly 9 times."; return false; }
    if (counts.size() != 6) { error = "Invalid number of colors."; return false; }
    for (int face = 0; face < 6; ++face) if (stickers_[face * 9 + 4] != kColors[face]) { error = "Invalid center-color configuration."; return false; }

    std::array<int, 8> cornerPermutation{}; int cornerOrientation = 0;
    for (int i = 0; i < 8; ++i) {
        std::array<char, 3> actual{}; for (int j = 0; j < 3; ++j) actual[j] = stickers_[kCorners[i][j]];
        int match = -1; for (int candidate = 0; candidate < 8; ++candidate) if (sameSet(actual, kCornerColors[candidate])) { match = candidate; break; }
        if (match < 0) { error = "Invalid corner configuration."; return false; }
        cornerPermutation[i] = match;
        int orientation = 0;
        for (int j = 0; j < 3; ++j) {
            if (actual[j] != 'W' && actual[j] != 'Y') continue;
            orientation = i < 4 ? j : (3 - j) % 3;
        }
        cornerOrientation += orientation;
    }
    if (cornerOrientation % 3 != 0) { error = "Invalid corner orientation."; return false; }
    std::array<int, 12> edgePermutation{}; int edgeOrientation = 0;
    for (int i = 0; i < 12; ++i) {
        std::array<char, 2> actual = {stickers_[kEdges[i][0]], stickers_[kEdges[i][1]]}; int match = -1;
        for (int candidate = 0; candidate < 12; ++candidate) if (sameSet(actual, kEdgeColors[candidate])) { match = candidate; break; }
        if (match < 0) { error = "Invalid edge configuration."; return false; }
        edgePermutation[i] = match;
        bool hasVerticalColor = actual[0] == 'W' || actual[0] == 'Y' || actual[1] == 'W' || actual[1] == 'Y';
        if (hasVerticalColor) {
            int verticalPosition = (actual[0] == 'W' || actual[0] == 'Y') ? 0 : 1;
            edgeOrientation += verticalPosition;
        } else {
            int greenBluePosition = (actual[0] == 'G' || actual[0] == 'B') ? 0 : 1;
            edgeOrientation += greenBluePosition;
        }
    }
    if (edgeOrientation % 2 != 0) { error = "Invalid edge orientation."; return false; }
    int cornerParity = 0, edgeParity = 0;
    for (int i = 0; i < 8; ++i) for (int j = i + 1; j < 8; ++j) cornerParity += cornerPermutation[i] > cornerPermutation[j];
    for (int i = 0; i < 12; ++i) for (int j = i + 1; j < 12; ++j) edgeParity += edgePermutation[i] > edgePermutation[j];
    if ((cornerParity + edgeParity) % 2 != 0) { error = "Invalid permutation parity."; return false; }
    error = "Cube configuration is valid and solvable."; return true;
}

std::vector<std::string> allMoves() {
    std::vector<std::string> moves; for (char face : kFaces) { moves.push_back(std::string(1, face)); moves.push_back(std::string(1, face) + "'"); moves.push_back(std::string(1, face) + "2"); } return moves;
}
std::string inverseMove(const std::string& move) { if (move.size() > 1 && move[1] == '2') return move; if (move.size() > 1 && move[1] == '\'') return std::string(1, move[0]); return std::string(1, move[0]) + "'"; }
} // namespace rubik
