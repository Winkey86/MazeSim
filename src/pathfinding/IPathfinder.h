#pragma once
#include <string>
#include "core/Maze.h"
#include "pathfinding/PathTypes.h"

namespace ml {

class IPathfinder {
public:
    virtual ~IPathfinder() = default;
    virtual std::string Name() const = 0;
    virtual PathResult FindPath(const Maze& maze, CellPos start, CellPos goal) = 0;
};

} // namespace ml
