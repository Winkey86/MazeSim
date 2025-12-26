#include "agents/AStarAgent.h"
#include <algorithm>
#include <limits>
#include <cmath>

namespace ml {

static int manh(CellPos a, CellPos b) {
    return std::abs(a.x - b.x) + std::abs(a.y - b.y);
}

void AStarAgent::Reset(CellPos start, CellPos exit) {
    AgentBase::Reset(start, exit);
    m_phase = Phase::Explore;
    m_open = {};
    m_prev.assign(static_cast<size_t>(m_w*m_h), -1);
    m_g.assign(static_cast<size_t>(m_w*m_h), std::numeric_limits<int>::max()/4);
    m_closed.assign(static_cast<size_t>(m_w*m_h), 0u);
    m_inOpen.assign(static_cast<size_t>(m_w*m_h), 0u);
    m_path.clear();
    ClearFrontier();
}

void AStarAgent::Start() {
    AgentBase::Start();
    if (!m_env) { m_metrics.status = AgentStatus::Fail; return; }
    if (m_env->IsWall(m_start) || m_env->IsWall(m_exit)) { m_metrics.status = AgentStatus::Fail; return; }

    auto idx = [&](CellPos p) { return ToIndex(p.x, p.y, m_w); };

    while (!m_open.empty()) m_open.pop();
    std::fill(m_prev.begin(), m_prev.end(), -1);
    std::fill(m_g.begin(), m_g.end(), std::numeric_limits<int>::max()/4);
    std::fill(m_closed.begin(), m_closed.end(), 0u);
    std::fill(m_inOpen.begin(), m_inOpen.end(), 0u);

    m_g[static_cast<size_t>(idx(m_start))] = 0;
    m_open.push({manh(m_start, m_exit), 0, m_start});
    m_inOpen[static_cast<size_t>(idx(m_start))] = 1u;
    ClearFrontier();
    SetFrontier(m_start);
}

void AStarAgent::Tick() {
    if (!IsRunning()) return;
    if (!m_env) { RequestStopFail(); return; }
    auto idx = [&](CellPos p) { return ToIndex(p.x, p.y, m_w); };

    if (m_phase == Phase::Follow) {
        if (m_path.empty()) {
            if (m_pos == m_exit) {
                m_metrics.status = AgentStatus::Success;
                m_metrics.path_length = m_metrics.steps;
            } else {
                m_metrics.status = AgentStatus::Fail;
            }
            return;
        }
        CellPos next = m_path.front();
        m_path.pop_front();
        MoveTo(next);
        if (m_pos == m_exit) {
            m_metrics.status = AgentStatus::Success;
            m_metrics.path_length = m_metrics.steps;
        }
        return;
    }

    // Explore one best node per tick
    while (!m_open.empty() && m_closed[static_cast<size_t>(idx(m_open.top().p))]) m_open.pop();
    if (m_open.empty()) { m_metrics.status = AgentStatus::Fail; return; }

    Node curN = m_open.top();
    m_open.pop();
    CellPos cur = curN.p;
    int ci = idx(cur);
    if (m_closed[static_cast<size_t>(ci)]) return;
    m_closed[static_cast<size_t>(ci)] = 1u;
    m_metrics.expanded_nodes++;

    // visualization: visited mask as closed set
    MarkVisited(cur);

    if (cur == m_exit) {
        // reconstruct
        std::vector<CellPos> rev;
        int at = ci;
        while (at != -1) {
            rev.push_back({at % m_w, at / m_w});
            at = m_prev[static_cast<size_t>(at)];
        }
        std::reverse(rev.begin(), rev.end());
        m_path.clear();
        for (size_t i = 1; i < rev.size(); ++i) m_path.push_back(rev[i]);
        m_phase = Phase::Follow;
        return;
    }

    ClearFrontier();
    // Mark some open nodes as frontier - cheap: mark neighbors we push
    for (Dir d : kDirs) {
        CellPos nxt{cur.x + Delta(d).dx, cur.y + Delta(d).dy};
        if (!m_env->IsFree(nxt)) continue;
        int ni = idx(nxt);
        if (m_closed[static_cast<size_t>(ni)]) continue;
        int tentativeG = m_g[static_cast<size_t>(ci)] + 1;
        if (tentativeG < m_g[static_cast<size_t>(ni)]) {
            m_g[static_cast<size_t>(ni)] = tentativeG;
            m_prev[static_cast<size_t>(ni)] = ci;
            int f = tentativeG + manh(nxt, m_exit);
            m_open.push({f, tentativeG, nxt});
            m_inOpen[static_cast<size_t>(ni)] = 1u;
            SetFrontier(nxt);
        }
    }
}

} // namespace ml
