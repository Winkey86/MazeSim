#pragma once
#include "generators/IMazeGenerator.h"

namespace ml {

class PrimGenerator final : public IMazeGenerator {
public:
    std::string Name() const override { return "Randomized Prim"; }
    void Generate(Maze& maze, const MazeGenConfig& cfg) override;
};

} // namespace ml
