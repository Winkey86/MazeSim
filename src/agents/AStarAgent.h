#pragma once
#include "agents/AgentBase.h"
#include "agents/Environment.h"
#include <queue>
#include <vector>
#include <deque>

namespace ml {

class AStarAgent final : public AgentBase {
public:
    explicit AStarAgent(const IFullEnvironment* env) : m_env(env) {}
    std::string Name() const override { return "A* (Full)"; }

    void Reset(CellPos start, CellPos exit) override;
    void Start() override;
    void Tick() override;

private:
    enum class Phase { Explore, Follow };

    struct Node { int f; int g; CellPos p; };
    struct Cmp {
        bool operator()(const Node& a, const Node& b) const noexcept {
            if (a.f != b.f) return a.f > b.f;
            return a.g > b.g;
        }
    };

    const IFullEnvironment* m_env{nullptr};
    Phase m_phase{Phase::Explore};

    std::priority_queue<Node, std::vector<Node>, Cmp> m_open;
    std::vector<int> m_prev;
    std::vector<int> m_g;
    std::vector<uint8_t> m_closed;
    std::vector<uint8_t> m_inOpen;
    std::deque<CellPos> m_path;
};

} // namespace ml
