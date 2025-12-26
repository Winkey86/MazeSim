#include "agents/BFSAgent.h"
#include <algorithm>

namespace ml {

void BFSAgent::Reset(CellPos start, CellPos exit) {
    AgentBase::Reset(start, exit);
    m_phase = Phase::Explore;
    m_q = {};
    m_prev.assign(static_cast<size_t>(m_w*m_h), -1);
    m_seen.assign(static_cast<size_t>(m_w*m_h), 0u);
    m_path.clear();
    ClearFrontier();

    if (!m_env) {
        m_metrics.status = AgentStatus::Fail;
        return;
    }
}

void BFSAgent::Start() {
    AgentBase::Start();
    if (!m_env) { m_metrics.status = AgentStatus::Fail; return; }
    if (m_env->IsWall(m_start) || m_env->IsWall(m_exit)) { m_metrics.status = AgentStatus::Fail; return; }

    auto idx = [&](CellPos p) { return ToIndex(p.x, p.y, m_w); };
    m_q = {};
    while (!m_q.empty()) m_q.pop();
    m_q.push(m_start);
    m_seen[static_cast<size_t>(idx(m_start))] = 1u;
    m_prev[static_cast<size_t>(idx(m_start))] = -1;
    SetFrontier(m_start);
}

void BFSAgent::Tick() {
    if (!IsRunning()) return;
    if (!m_env) { RequestStopFail(); return; }

    auto idx = [&](CellPos p) { return ToIndex(p.x, p.y, m_w); };

    if (m_phase == Phase::Follow) {
        if (m_path.empty()) {
            // reached
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

    // Explore: expand one node per tick
    if (m_q.empty()) {
        m_metrics.status = AgentStatus::Fail;
        return;
    }

    CellPos cur = m_q.front();
    m_q.pop();
    ClearFrontier(); // simple: recompute sparse frontier by marking queue entries could be expensive; keep minimal
    m_metrics.expanded_nodes++;

    if (cur == m_exit) {
        // reconstruct path start->exit
        std::vector<CellPos> rev;
        int ci = idx(cur);
        while (ci != -1) {
            rev.push_back({ci % m_w, ci / m_w});
            ci = m_prev[static_cast<size_t>(ci)];
        }
        std::reverse(rev.begin(), rev.end());

        // build follow path excluding current position (start)
        m_path.clear();
        for (size_t i = 1; i < rev.size(); ++i) m_path.push_back(rev[i]);

        m_phase = Phase::Follow;
        return;
    }

    // closed set visualization == visited mask; we mark on dequeue
    MarkVisited(cur);

    for (Dir d : kDirs) {
        CellPos nxt{cur.x + Delta(d).dx, cur.y + Delta(d).dy};
        if (!m_env->IsFree(nxt)) continue;
        int ni = idx(nxt);
        if (m_seen[static_cast<size_t>(ni)]) continue;
        m_seen[static_cast<size_t>(ni)] = 1u;
        m_prev[static_cast<size_t>(ni)] = idx(cur);
        m_q.push(nxt);
        SetFrontier(nxt);
    }
}

} // namespace ml
