#pragma once
#include "pathfinding/IPathfinder.h"

namespace ml {

class AStarPathfinder final : public IPathfinder {
public:
    std::string Name() const override { return "A*"; }
    PathResult FindPath(const Maze& maze, CellPos start, CellPos goal) override;
};

} // namespace ml
