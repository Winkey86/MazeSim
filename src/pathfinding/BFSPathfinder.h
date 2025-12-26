#pragma once
#include "pathfinding/IPathfinder.h"

namespace ml {

class BFSPathfinder final : public IPathfinder {
public:
    std::string Name() const override { return "BFS"; }
    PathResult FindPath(const Maze& maze, CellPos start, CellPos goal) override;
};

} // namespace ml
