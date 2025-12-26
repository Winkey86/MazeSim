#pragma once
#include "agents/AgentBase.h"
#include "agents/Environment.h"
#include "core/Directions.h"

namespace ml {

class RightHandAgent final : public AgentBase {
public:
    explicit RightHandAgent(const IPartialEnvironment* env) : m_env(env) {}
    std::string Name() const override { return "Right-hand (Partial)"; }

    void Reset(CellPos start, CellPos exit) override;
    void Start() override;
    void Tick() override;

private:
    const IPartialEnvironment* m_env{nullptr};
    Dir m_facing{Dir::E};
};

} // namespace ml
