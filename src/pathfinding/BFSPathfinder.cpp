#include "pathfinding/BFSPathfinder.h"
#include <queue>

namespace ml {

PathResult BFSPathfinder::FindPath(const Maze& maze, CellPos start, CellPos goal) {
    PathResult res;
    const int w = maze.Width();
    const int h = maze.Height();

    auto idx = [&](CellPos p) { return ToIndex(p.x, p.y, w); };

    std::vector<int> prev(static_cast<size_t>(w*h), -1);
    std::vector<uint8_t> vis(static_cast<size_t>(w*h), 0u);

    if (!maze.InBounds(start) || !maze.InBounds(goal) || maze.IsWall(start) || maze.IsWall(goal)) {
        res.found = false;
        return res;
    }

    std::queue<CellPos> q;
    q.push(start);
    vis[static_cast<size_t>(idx(start))] = 1u;

    while (!q.empty()) {
        CellPos cur = q.front();
        q.pop();
        res.expandedNodes++;

        if (cur == goal) {
            res.found = true;
            // reconstruct
            std::vector<CellPos> path;
            int curi = idx(cur);
            while (curi != -1) {
                int x = curi % w;
                int y = curi / w;
                path.push_back({x,y});
                curi = prev[static_cast<size_t>(curi)];
            }
            std::reverse(path.begin(), path.end());
            res.path = std::move(path);
            return res;
        }

        for (Dir d : kDirs) {
            CellPos nxt = maze.Step(cur, d);
            if (!maze.InBounds(nxt) || maze.IsWall(nxt)) continue;
            int ni = idx(nxt);
            if (vis[static_cast<size_t>(ni)]) continue;
            vis[static_cast<size_t>(ni)] = 1u;
            prev[static_cast<size_t>(ni)] = idx(cur);
            q.push(nxt);
        }
    }

    res.found = false;
    return res;
}

} // namespace ml
