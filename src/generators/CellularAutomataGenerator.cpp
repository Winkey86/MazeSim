#include "generators/CellularAutomataGenerator.h"

#include <algorithm>
#include <queue>

namespace ml {

static bool HasPathBFS(const Maze& maze, CellPos start, CellPos goal) {
    const int w = maze.Width();
    const int h = maze.Height();
    if (!maze.InBounds(start) || !maze.InBounds(goal)) return false;
    if (maze.IsWall(start) || maze.IsWall(goal)) return false;

    std::vector<uint8_t> vis(static_cast<size_t>(w * h), 0u);
    auto idx = [&](CellPos p) { return static_cast<size_t>(ToIndex(p.x, p.y, w)); };

    std::queue<CellPos> q;
    q.push(start);
    vis[idx(start)] = 1u;

    while (!q.empty()) {
        CellPos cur = q.front();
        q.pop();
        if (cur == goal) return true;

        for (Dir d : kDirs) {
            CellPos nxt = maze.Step(cur, d);
            if (!maze.InBounds(nxt) || maze.IsWall(nxt)) continue;
            auto i = idx(nxt);
            if (vis[i]) continue;
            vis[i] = 1u;
            q.push(nxt);
        }
    }
    return false;
}

int CellularAutomataGenerator::CountWallNeighbors8(const Maze& maze, int x, int y) {
    int count = 0;
    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            if (dx == 0 && dy == 0) continue;
            const int nx = x + dx;
            const int ny = y + dy;
            if (nx < 0 || ny < 0 || nx >= maze.Width() || ny >= maze.Height()) {
                // out of bounds treated as wall
                count++;
                continue;
            }
            if (maze.IsWall({nx, ny})) count++;
        }
    }
    return count;
}

void CellularAutomataGenerator::CarveManhattanCorridor(Maze& maze, CellPos start, CellPos goal) {
    // Carve an L-shaped corridor (x then y). Always stays in bounds for interior points.
    CellPos cur = start;
    maze.SetFree(cur, true);

    while (cur.x != goal.x) {
        cur.x += (goal.x > cur.x) ? 1 : -1;
        maze.SetFree(cur, true);
    }
    while (cur.y != goal.y) {
        cur.y += (goal.y > cur.y) ? 1 : -1;
        maze.SetFree(cur, true);
    }

    // Slightly widen around start/goal to avoid immediate dead-ends
    for (int oy = -1; oy <= 1; ++oy) {
        for (int ox = -1; ox <= 1; ++ox) {
            CellPos a{start.x + ox, start.y + oy};
            CellPos b{goal.x + ox, goal.y + oy};
            if (maze.InBounds(a)) maze.SetFree(a, true);
            if (maze.InBounds(b)) maze.SetFree(b, true);
        }
    }
}

void CellularAutomataGenerator::Generate(Maze& maze, const MazeGenConfig& cfg) {
    RNG rng(cfg.randomSeed ? RNG() : RNG(cfg.seed));
    if (cfg.randomSeed) {
        // RNG() already seeds itself; nothing to do
    }

    maze.Resize(cfg.width, cfg.height);

    // Initial random fill.
    // Border is wall. Interior is wall with probability p.
    const int pWall = 45; // %
    for (int y = 0; y < cfg.height; ++y) {
        for (int x = 0; x < cfg.width; ++x) {
            const bool border = (x == 0 || y == 0 || x == cfg.width - 1 || y == cfg.height - 1);
            bool wall = border;
            if (!border) {
                wall = (rng.NextInt(0, 99) < pWall);
            }
            maze.SetFree({x, y}, !wall);
        }
    }

    // Ensure start/exit are open before smoothing.
    maze.SetFree(cfg.start, true);
    maze.SetFree(cfg.exit, true);

    // Cellular automata smoothing
    const int iters = 5;
    std::vector<uint8_t> next(static_cast<size_t>(cfg.width * cfg.height), 1u);
    auto setNext = [&](int x, int y, bool free) {
        next[static_cast<size_t>(ToIndex(x, y, cfg.width))] = free ? 0u : 1u;
    };

    for (int it = 0; it < iters; ++it) {
        for (int y = 0; y < cfg.height; ++y) {
            for (int x = 0; x < cfg.width; ++x) {
                const bool border = (x == 0 || y == 0 || x == cfg.width - 1 || y == cfg.height - 1);
                if (border) {
                    setNext(x, y, false);
                    continue;
                }

                int walls8 = CountWallNeighbors8(maze, x, y);
                // Typical cave rule: >=5 walls -> wall, else free
                bool willBeWall = (walls8 >= 5);
                setNext(x, y, !willBeWall);
            }
        }

        // Commit
        for (int y = 0; y < cfg.height; ++y) {
            for (int x = 0; x < cfg.width; ++x) {
                bool free = next[static_cast<size_t>(ToIndex(x, y, cfg.width))] == 0u;
                maze.SetFree({x, y}, free);
            }
        }

        maze.SetFree(cfg.start, true);
        maze.SetFree(cfg.exit, true);
    }

    // Guarantee connectivity.
    if (!HasPathBFS(maze, cfg.start, cfg.exit)) {
        CarveManhattanCorridor(maze, cfg.start, cfg.exit);
    }
    // Final guarantee (rare): if still no path, fully open interior corridor area.
    if (!HasPathBFS(maze, cfg.start, cfg.exit)) {
        for (int y = 1; y < cfg.height - 1; ++y) {
            for (int x = 1; x < cfg.width - 1; ++x) {
                maze.SetFree({x, y}, true);
            }
        }
        CarveManhattanCorridor(maze, cfg.start, cfg.exit);
    }
}

} // namespace ml
