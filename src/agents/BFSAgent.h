#pragma once
#include "agents/AgentBase.h"
#include "agents/Environment.h"
#include <queue>
#include <vector>
#include <deque>

namespace ml {

class BFSAgent final : public AgentBase {
public:
    explicit BFSAgent(const IFullEnvironment* env) : m_env(env) {}
    std::string Name() const override { return "BFS (Full)"; }

    void Reset(CellPos start, CellPos exit) override;
    void Start() override;
    void Tick() override;

private:
    enum class Phase { Explore, Follow };

    const IFullEnvironment* m_env{nullptr};
    Phase m_phase{Phase::Explore};

    std::queue<CellPos> m_q;
    std::vector<int> m_prev;
    std::vector<uint8_t> m_seen;     // discovered
    std::deque<CellPos> m_path;      // includes current->exit (excluding current)
};

} // namespace ml
