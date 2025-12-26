#include "agents/RightHandAgent.h"

namespace ml {

void RightHandAgent::Reset(CellPos start, CellPos exit) {
    AgentBase::Reset(start, exit);
    m_facing = Dir::E;
}

void RightHandAgent::Start() {
    AgentBase::Start();
}

static bool IsBlocked(const Sense4& s, Dir d) {
    switch (d) {
    case Dir::N: return s.n;
    case Dir::E: return s.e;
    case Dir::S: return s.s;
    case Dir::W: return s.w;
    default: return true;
    }
}

void RightHandAgent::Tick() {
    if (!IsRunning()) return;
    if (!m_env) { RequestStopFail(); return; }

    if (m_pos == m_env->GetExit()) {
        m_metrics.status = AgentStatus::Success;
        m_metrics.path_length = m_metrics.steps;
        return;
    }

    Sense4 s = m_env->SenseWalls4(m_pos);

    // Right-hand rule: try right, forward, left, back.
    Dir cand[4] = { TurnRight(m_facing), m_facing, TurnLeft(m_facing), TurnBack(m_facing) };
    Dir chosen = cand[0];
    int chosenIndex = -1;

    for (int i = 0; i < 4; ++i) {
        if (!IsBlocked(s, cand[i])) {
            chosen = cand[i];
            chosenIndex = i;
            break;
        }
    }

    if (chosenIndex > 0) {
        // had to deviate from "turn right first" => replan-ish
        m_metrics.replans++;
    }

    CellPos newPos{};
    if (m_env->TryMove(m_pos, chosen, newPos)) {
        m_facing = chosen;
        MoveTo(newPos);
    } else {
        // unexpected: sensed free but couldn't move
        m_metrics.replans++;
    }

    if (m_pos == m_env->GetExit()) {
        m_metrics.status = AgentStatus::Success;
        m_metrics.path_length = m_metrics.steps;
    }
}

} // namespace ml
