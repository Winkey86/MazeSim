#include "agents/FrontierExplorerAgent.h"
#include <queue>
#include <vector>
#include <algorithm>

namespace ml {

void FrontierExplorerAgent::OnMazeChanged(int w, int h) {
    AgentBase::OnMazeChanged(w, h);
    m_kmap.Resize(w, h);
}

void FrontierExplorerAgent::Reset(CellPos start, CellPos exit) {
    AgentBase::Reset(start, exit);
    m_plan.clear();
    m_target = {-1,-1};
    m_kmap.Resize(m_w, m_h);
    // at least know start is free
    m_kmap.Set(start, Know::Free);
}

void FrontierExplorerAgent::Start() {
    AgentBase::Start();
}

static void ApplySense(KnowledgeMap& km, CellPos at, const Sense4& s) {
    km.Set(at, Know::Free);
    // Mark neighbors as wall/free by sensing adjacency.
    auto markNeighbor = [&](int dx, int dy, bool blocked) {
        CellPos n{at.x + dx, at.y + dy};
        if (!km.InBounds(n)) return;
        km.Set(n, blocked ? Know::Wall : Know::Free);
    };
    markNeighbor(0,-1, s.n);
    markNeighbor(1,0,  s.e);
    markNeighbor(0,1,  s.s);
    markNeighbor(-1,0, s.w);
}

void FrontierExplorerAgent::UpdateKnowledgeAt(CellPos at) {
    if (!m_env) return;
    Sense4 s = m_env->SenseWalls4(at);
    ApplySense(m_kmap, at, s);
}

bool FrontierExplorerAgent::IsFrontier(CellPos p) const {
    if (m_kmap.Get(p) != Know::Free) return false;
    // frontier if has unknown neighbor
    CellPos n[4] = {{p.x, p.y-1}, {p.x+1,p.y}, {p.x,p.y+1}, {p.x-1,p.y}};
    for (auto q : n) {
        if (!m_kmap.InBounds(q)) continue;
        if (m_kmap.Get(q) == Know::Unknown) return true;
    }
    return false;
}

bool FrontierExplorerAgent::FindNearestFrontier(CellPos from, CellPos& outTarget) {
    const int w = m_w, h = m_h;
    std::vector<uint8_t> vis(static_cast<size_t>(w*h), 0u);
    std::queue<CellPos> q;
    q.push(from);
    vis[static_cast<size_t>(ToIndex(from.x,from.y,w))] = 1u;

    while (!q.empty()) {
        CellPos cur = q.front(); q.pop();
        if (IsFrontier(cur)) { outTarget = cur; return true; }

        for (Dir d : kDirs) {
            CellPos nxt{cur.x + Delta(d).dx, cur.y + Delta(d).dy};
            if (!m_kmap.InBounds(nxt)) continue;
            if (m_kmap.Get(nxt) != Know::Free) continue;
            int ni = ToIndex(nxt.x,nxt.y,w);
            if (vis[static_cast<size_t>(ni)]) continue;
            vis[static_cast<size_t>(ni)] = 1u;
            q.push(nxt);
        }
    }
    return false;
}

bool FrontierExplorerAgent::PlanPath(CellPos from, CellPos to) {
    const int w = m_w, h = m_h;
    std::vector<int> prev(static_cast<size_t>(w*h), -1);
    std::queue<CellPos> q;

    auto idx = [&](CellPos p) { return ToIndex(p.x,p.y,w); };

    q.push(from);
    prev[static_cast<size_t>(idx(from))] = idx(from); // mark as root

    while (!q.empty()) {
        CellPos cur = q.front(); q.pop();
        if (cur == to) break;

        for (Dir d : kDirs) {
            CellPos nxt{cur.x + Delta(d).dx, cur.y + Delta(d).dy};
            if (!m_kmap.InBounds(nxt)) continue;
            if (m_kmap.Get(nxt) != Know::Free) continue;
            int ni = idx(nxt);
            if (prev[static_cast<size_t>(ni)] != -1) continue;
            prev[static_cast<size_t>(ni)] = idx(cur);
            q.push(nxt);
        }
    }

    int ti = idx(to);
    if (prev[static_cast<size_t>(ti)] == -1) return false;

    // reconstruct
    std::vector<CellPos> rev;
    int curi = ti;
    while (curi != prev[static_cast<size_t>(curi)]) {
        rev.push_back({curi % w, curi / w});
        curi = prev[static_cast<size_t>(curi)];
    }
    rev.push_back(from);
    std::reverse(rev.begin(), rev.end());

    m_plan.clear();
    for (size_t i = 1; i < rev.size(); ++i) m_plan.push_back(rev[i]);
    return true;
}

void FrontierExplorerAgent::Tick() {
    if (!IsRunning()) return;
    if (!m_env) { RequestStopFail(); return; }

    if (m_pos == m_env->GetExit()) {
        m_metrics.status = AgentStatus::Success;
        m_metrics.path_length = m_metrics.steps;
        return;
    }

    // 1) update knowledge around current
    UpdateKnowledgeAt(m_pos);

    // 2) if no plan, pick nearest frontier and plan
    if (m_plan.empty()) {
        CellPos target{};
        if (FindNearestFrontier(m_pos, target)) {
            m_target = target;
            if (!PlanPath(m_pos, target)) {
                m_metrics.replans++;
            }
        } else {
            // no known frontier: try move randomly among sensed free, else fail
            Sense4 s = m_env->SenseWalls4(m_pos);
            Dir opts[4] = {Dir::N, Dir::E, Dir::S, Dir::W};
            bool moved = false;
            for (Dir d : opts) {
                CellPos newPos{};
                if (m_env->TryMove(m_pos, d, newPos)) {
                    MoveTo(newPos);
                    moved = true;
                    break;
                }
            }
            if (!moved) m_metrics.status = AgentStatus::Fail;
            return;
        }
    }

    // 3) execute one step of plan
    if (!m_plan.empty()) {
        CellPos next = m_plan.front();

        // Ensure next is adjacent; otherwise replan
        if (std::abs(next.x - m_pos.x) + std::abs(next.y - m_pos.y) != 1) {
            m_plan.clear();
            m_metrics.replans++;
            return;
        }

        // Attempt move in direction
        Dir dir = Dir::N;
        if (next.x > m_pos.x) dir = Dir::E;
        else if (next.x < m_pos.x) dir = Dir::W;
        else if (next.y > m_pos.y) dir = Dir::S;
        else dir = Dir::N;

        CellPos newPos{};
        if (m_env->TryMove(m_pos, dir, newPos)) {
            m_plan.pop_front();
            MoveTo(newPos);
        } else {
            // discovered wall unexpectedly -> update knowledge & replan
            m_kmap.Set(next, Know::Wall);
            m_plan.clear();
            m_metrics.replans++;
        }
    }

    if (m_pos == m_env->GetExit()) {
        m_metrics.status = AgentStatus::Success;
        m_metrics.path_length = m_metrics.steps;
    }
}

} // namespace ml
