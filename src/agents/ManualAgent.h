#pragma once
#include "agents/AgentBase.h"
#include "agents/Environment.h"
#include <deque>
#include <optional>

namespace ml {

class ManualAgent final : public AgentBase {
public:
    explicit ManualAgent(const IPartialEnvironment* env) : m_env(env) {}
    std::string Name() const override { return "Manual (Click-to-move)"; }

    void Reset(CellPos start, CellPos exit) override;
    void Start() override;
    void Tick() override;

    // UI calls this on right-click in grid coordinates
    void OnRightClick(CellPos target);

private:
    const IPartialEnvironment* m_env{nullptr};
    std::deque<CellPos> m_path;
};

} // namespace ml
