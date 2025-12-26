#pragma once
#include <array>

namespace ml {

enum class Dir : uint8_t { N = 0, E = 1, S = 2, W = 3 };

inline Dir TurnRight(Dir d) { return static_cast<Dir>((static_cast<int>(d) + 1) & 3); }
inline Dir TurnLeft(Dir d) { return static_cast<Dir>((static_cast<int>(d) + 3) & 3); }
inline Dir TurnBack(Dir d) { return static_cast<Dir>((static_cast<int>(d) + 2) & 3); }

struct DirDelta { int dx; int dy; };

inline DirDelta Delta(Dir d) {
    switch (d) {
    case Dir::N: return {0, -1};
    case Dir::E: return {1, 0};
    case Dir::S: return {0, 1};
    case Dir::W: return {-1, 0};
    default: return {0, 0};
    }
}

inline constexpr std::array<Dir, 4> kDirs{Dir::N, Dir::E, Dir::S, Dir::W};

} // namespace ml
