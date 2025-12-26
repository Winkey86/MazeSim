// File: src/agents/ManualAgent.cpp
#include "agents/ManualAgent.h"
#include <algorithm>
#include <cmath>

namespace ml {

    void ManualAgent::Reset(CellPos start, CellPos exit) {
        AgentBase::Reset(start, exit);
        m_path.clear();
    }

    void ManualAgent::Start() {
        AgentBase::Start();
    }

    void ManualAgent::OnRightClick(CellPos target) {
        if (!m_env) return;
        if (m_metrics.status != AgentStatus::Running) return;
        if (target.x < 0 || target.y < 0 || target.x >= m_env->Width() || target.y >= m_env->Height()) return;

        // Must be reachable strictly by straight line (same row or col) with no walls on the segment.
        if (target.x != m_pos.x && target.y != m_pos.y) return;

        std::deque<CellPos> path;
        int dx = (target.x > m_pos.x) ? 1 : (target.x < m_pos.x ? -1 : 0);
        int dy = (target.y > m_pos.y) ? 1 : (target.y < m_pos.y ? -1 : 0);

        CellPos cur = m_pos;
        while (cur != target) {
            Dir dir = Dir::N;
            if (dx == 1) dir = Dir::E;
            else if (dx == -1) dir = Dir::W;
            else if (dy == 1) dir = Dir::S;
            else if (dy == -1) dir = Dir::N;

            CellPos nxt{};
            if (!m_env->TryMove(cur, dir, nxt)) {
                return; // blocked
            }

            path.push_back(nxt);
            cur = nxt;
        }

        m_path = std::move(path);
    }

    void ManualAgent::Tick() {
        if (!m_env) return;
        if (m_metrics.status != AgentStatus::Running) return;

        // Mark visited
        MarkVisited(m_pos);

        if (!m_path.empty()) {
            CellPos next = m_path.front();
            m_path.pop_front();

            // Validate the move against the environment.
            Dir dir = Dir::N;
            if (next.x == m_pos.x + 1 && next.y == m_pos.y) dir = Dir::E;
            else if (next.x == m_pos.x - 1 && next.y == m_pos.y) dir = Dir::W;
            else if (next.x == m_pos.x && next.y == m_pos.y + 1) dir = Dir::S;
            else if (next.x == m_pos.x && next.y == m_pos.y - 1) dir = Dir::N;
            else {
                m_path.clear();
                return;
            }

            CellPos moved{};
            if (m_env->TryMove(m_pos, dir, moved)) {
                MoveTo(moved);
            }
            else {
                m_path.clear();
            }
        }

        if (m_pos == m_exit) {
            m_metrics.status = AgentStatus::Success;
            m_metrics.path_length = m_metrics.steps;
        }
    }

} // namespace ml
