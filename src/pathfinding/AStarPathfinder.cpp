#include "pathfinding/AStarPathfinder.h"
#include <queue>
#include <limits>
#include <cmath>

namespace ml {

static int Manhattan(CellPos a, CellPos b) {
    return std::abs(a.x - b.x) + std::abs(a.y - b.y);
}

struct PQNode {
    int f;
    int g;
    CellPos pos;
};

struct PQCmp {
    bool operator()(const PQNode& a, const PQNode& b) const noexcept {
        // min-heap via reverse
        if (a.f != b.f) return a.f > b.f;
        return a.g > b.g;
    }
};

PathResult AStarPathfinder::FindPath(const Maze& maze, CellPos start, CellPos goal) {
    PathResult res;
    const int w = maze.Width();
    const int h = maze.Height();
    auto idx = [&](CellPos p) { return ToIndex(p.x, p.y, w); };

    if (!maze.InBounds(start) || !maze.InBounds(goal) || maze.IsWall(start) || maze.IsWall(goal)) {
        res.found = false;
        return res;
    }

    const int N = w*h;
    const int INF = std::numeric_limits<int>::max() / 4;

    std::vector<int> prev(static_cast<size_t>(N), -1);
    std::vector<int> gScore(static_cast<size_t>(N), INF);
    std::vector<uint8_t> closed(static_cast<size_t>(N), 0u);

    std::priority_queue<PQNode, std::vector<PQNode>, PQCmp> open;
    gScore[static_cast<size_t>(idx(start))] = 0;
    open.push({Manhattan(start, goal), 0, start});

    while (!open.empty()) {
        PQNode curN = open.top();
        open.pop();
        CellPos cur = curN.pos;
        int ci = idx(cur);
        if (closed[static_cast<size_t>(ci)]) continue;
        closed[static_cast<size_t>(ci)] = 1u;
        res.expandedNodes++;

        if (cur == goal) {
            res.found = true;
            std::vector<CellPos> path;
            int curi = ci;
            while (curi != -1) {
                path.push_back({curi % w, curi / w});
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
            if (closed[static_cast<size_t>(ni)]) continue;
            int tentativeG = gScore[static_cast<size_t>(ci)] + 1;
            if (tentativeG < gScore[static_cast<size_t>(ni)]) {
                gScore[static_cast<size_t>(ni)] = tentativeG;
                prev[static_cast<size_t>(ni)] = ci;
                int f = tentativeG + Manhattan(nxt, goal);
                open.push({f, tentativeG, nxt});
            }
        }
    }

    res.found = false;
    return res;
}

} // namespace ml
