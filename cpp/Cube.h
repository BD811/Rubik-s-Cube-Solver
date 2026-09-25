#pragma once

#include <array>
#include <string>
#include <vector>

namespace rubik {

class Cube {
public:
    static constexpr int StickerCount = 54;
    Cube();
    explicit Cube(const std::string& state);

    bool isSolved() const;
    bool isValid(std::string& error) const;
    std::string getState() const;
    void applyMove(const std::string& move);
    void applyMoves(const std::vector<std::string>& moves);

private:
    std::array<char, StickerCount> stickers_{};
    void rotateLayer(char face, int quarterTurns);
};

std::vector<std::string> allMoves();
std::string inverseMove(const std::string& move);

} // namespace rubik
