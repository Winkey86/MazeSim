#pragma once
#include <string>
#include <cstdint>
#include "core/Maze.h"
#include "core/RNG.h"
#include "core/Types.h"

namespace ml {

struct MazeGenConfig {
    int width{31};
    int height{31};
    uint32_t seed{1};
    bool randomSeed{false};
    CellPos start{1,1};
    CellPos exit{29,29};
};

class IMazeGenerator {
public:
    virtual ~IMazeGenerator() = default;
    virtual std::string Name() const = 0;
    virtual void Generate(Maze& maze, const MazeGenConfig& cfg) = 0;
};

} // namespace ml
