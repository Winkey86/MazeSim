#pragma once
#include "agents/IAgent.h"
#include <chrono>

namespace ml {

class AgentBase : public IAgent {
public:
    void OnMazeChanged(int w, int h) override {
        m_w = w; m_h = h;
        m_visited.assign(static_cast<size_t>(w*h), 0u);
        m_frontier.assign(static_cast<size_t>(w*h), 0u);
        m_uniqueVisited = 0;
    }

    void Reset(CellPos start, CellPos exit) override {
        m_start = start;
        m_exit = exit;
        m_pos = start;
        std::fill(m_visited.begin(), m_visited.end(), 0u);
        std::fill(m_frontier.begin(), m_frontier.end(), 0u);
        m_uniqueVisited = 0;
        m_metrics = AgentMetrics{};
        m_metrics.status = AgentStatus::Running;
        MarkVisited(m_pos);
        m_running = false;
    }

    void Start() override { m_running = true; }

    void RequestStopFail() override {
        if (m_metrics.status == AgentStatus::Running) m_metrics.status = AgentStatus::Fail;
        m_running = false;
    }

    AgentStatus Status() const override { return m_metrics.status; }
    const AgentMetrics& Metrics() const override { return m_metrics; }
    CellPos Position() const override { return m_pos; }
    const std::vector<uint8_t>& VisitedMask() const override { return m_visited; }
    const std::vector<uint8_t>& FrontierMask() const override { return m_frontier; }

protected:
    bool IsRunning() const noexcept { return m_running && m_metrics.status == AgentStatus::Running; }

    void MarkVisited(CellPos p) {
        if (m_w <= 0 || m_h <= 0) return;
        int i = ToIndex(p.x, p.y, m_w);
        if (i < 0 || i >= m_w*m_h) return;
        if (m_visited[static_cast<size_t>(i)] == 0u) {
            m_visited[static_cast<size_t>(i)] = 1u;
            m_uniqueVisited++;
            m_metrics.visited_unique = m_uniqueVisited;
        }
    }

    void ClearFrontier() {
        std::fill(m_frontier.begin(), m_frontier.end(), 0u);
    }
    void SetFrontier(CellPos p) {
        if (m_w <= 0 || m_h <= 0) return;
        int i = ToIndex(p.x, p.y, m_w);
        if (i < 0 || i >= m_w*m_h) return;
        m_frontier[static_cast<size_t>(i)] = 1u;
    }

    void MoveTo(CellPos newPos) {
        if (newPos != m_pos) {
            m_pos = newPos;
            m_metrics.steps++;
            MarkVisited(m_pos);
        }
    }

protected:
    int m_w{0}, m_h{0};
    CellPos m_start{1,1};
    CellPos m_exit{1,1};
    CellPos m_pos{1,1};
    AgentMetrics m_metrics{};
    bool m_running{false};

    std::vector<uint8_t> m_visited;
    std::vector<uint8_t> m_frontier;

    int m_uniqueVisited{0};
};

} // namespace ml
