#include "generators/RecursiveBacktrackerGenerator.h"
#include <vector>

namespace ml {

static CellPos MakeOddInBounds(const Maze& m, CellPos p) {
    // clamp to inside boundary, then force odd
    if (p.x < 1) p.x = 1;
    if (p.y < 1) p.y = 1;
    if (p.x > m.Width() - 2) p.x = m.Width() - 2;
    if (p.y > m.Height() - 2) p.y = m.Height() - 2;
    if ((p.x % 2) == 0) p.x += (p.x == m.Width() - 2 ? -1 : 1);
    if ((p.y % 2) == 0) p.y += (p.y == m.Height() - 2 ? -1 : 1);
    return p;
}

void RecursiveBacktrackerGenerator::Generate(Maze& maze, const MazeGenConfig& cfg) {
    maze.Resize(cfg.width, cfg.height);
    maze.FillWalls();

    RNG rng(cfg.randomSeed ? RNG().NextU32() : cfg.seed);

    CellPos startOdd = MakeOddInBounds(maze, cfg.start);
    CellPos exitOdd  = MakeOddInBounds(maze, cfg.exit);

    const int w = maze.Width();
    const int h = maze.Height();
    std::vector<uint8_t> visited(static_cast<size_t>(w*h), 0u);

    auto markVisited = [&](CellPos p) { visited[static_cast<size_t>(ToIndex(p.x,p.y,w))] = 1u; };
    auto isVisited = [&](CellPos p) -> bool { return visited[static_cast<size_t>(ToIndex(p.x,p.y,w))] != 0u; };

    std::vector<CellPos> stack;
    stack.push_back(startOdd);
    markVisited(startOdd);
    maze.SetFree(startOdd, true);

    while (!stack.empty()) {
        CellPos cur = stack.back();
        auto neigh = maze.OddNeighbors2(cur);

        // collect unvisited
        std::vector<CellPos> cand;
        cand.reserve(neigh.size());
        for (auto n : neigh) if (!isVisited(n)) cand.push_back(n);

        if (cand.empty()) {
            stack.pop_back();
            continue;
        }

        int idx = rng.NextInt(0, static_cast<int>(cand.size()) - 1);
        CellPos nxt = cand[static_cast<size_t>(idx)];
        maze.CarvePassage(cur, nxt);
        markVisited(nxt);
        stack.push_back(nxt);
    }

    // Ensure start/exit are open and connected via carved graph (they are odd nodes, so path exists).
    maze.SetFree(startOdd, true);
    maze.SetFree(exitOdd, true);
}

} // namespace ml
