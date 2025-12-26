#pragma once
#include "generators/IMazeGenerator.h"

namespace ml {

// Cave-like maze generator based on Cellular Automata smoothing.
// Note: this is not a "perfect maze" like DFS/Prim, but it is always
// guaranteed to have a path from start to exit.
class CellularAutomataGenerator final : public IMazeGenerator {
public:
    std::string Name() const override { return "Cellular Automata"; }
    void Generate(Maze& maze, const MazeGenConfig& cfg) override;

private:
    static int CountWallNeighbors8(const Maze& maze, int x, int y);
    static void CarveManhattanCorridor(Maze& maze, CellPos start, CellPos goal);
};

} // namespace ml
