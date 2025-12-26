#include "generators/PrimGenerator.h"
#include <vector>

namespace ml {

static CellPos MakeOddInBounds(const Maze& m, CellPos p) {
    if (p.x < 1) p.x = 1;
    if (p.y < 1) p.y = 1;
    if (p.x > m.Width() - 2) p.x = m.Width() - 2;
    if (p.y > m.Height() - 2) p.y = m.Height() - 2;
    if ((p.x % 2) == 0) p.x += (p.x == m.Width() - 2 ? -1 : 1);
    if ((p.y % 2) == 0) p.y += (p.y == m.Height() - 2 ? -1 : 1);
    return p;
}

struct Edge {
    CellPos from;
    CellPos to;
};

void PrimGenerator::Generate(Maze& maze, const MazeGenConfig& cfg) {
    maze.Resize(cfg.width, cfg.height);
    maze.FillWalls();

    RNG rng(cfg.randomSeed ? RNG().NextU32() : cfg.seed);

    CellPos startOdd = MakeOddInBounds(maze, cfg.start);
    CellPos exitOdd  = MakeOddInBounds(maze, cfg.exit);

    const int w = maze.Width();
    const int h = maze.Height();
    std::vector<uint8_t> inTree(static_cast<size_t>(w*h), 0u);

    auto mark = [&](CellPos p) { inTree[static_cast<size_t>(ToIndex(p.x,p.y,w))] = 1u; };
    auto isMarked = [&](CellPos p)->bool { return inTree[static_cast<size_t>(ToIndex(p.x,p.y,w))] != 0u; };

    std::vector<Edge> frontier;

    auto addFrontier = [&](CellPos p) {
        for (auto n : maze.OddNeighbors2(p)) {
            if (!isMarked(n)) frontier.push_back({p, n});
        }
    };

    maze.SetFree(startOdd, true);
    mark(startOdd);
    addFrontier(startOdd);

    while (!frontier.empty()) {
        int idx = rng.NextInt(0, static_cast<int>(frontier.size()) - 1);
        Edge e = frontier[static_cast<size_t>(idx)];
        frontier[static_cast<size_t>(idx)] = frontier.back();
        frontier.pop_back();

        if (isMarked(e.to)) continue;

        maze.CarvePassage(e.from, e.to);
        mark(e.to);
        addFrontier(e.to);
    }

    maze.SetFree(startOdd, true);
    maze.SetFree(exitOdd, true);
}

} // namespace ml
