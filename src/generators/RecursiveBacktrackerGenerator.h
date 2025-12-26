#pragma once
#include "generators/IMazeGenerator.h"

namespace ml {

class RecursiveBacktrackerGenerator final : public IMazeGenerator {
public:
    std::string Name() const override { return "DFS (Recursive Backtracker)"; }
    void Generate(Maze& maze, const MazeGenConfig& cfg) override;
};

} // namespace ml
