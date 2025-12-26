#pragma once
#include "agents/AgentBase.h"
#include "agents/Environment.h"
#include "agents/KnowledgeMap.h"
#include <deque>

namespace ml {

class FrontierExplorerAgent final : public AgentBase {
public:
    explicit FrontierExplorerAgent(const IPartialEnvironment* env) : m_env(env) {}
    std::string Name() const override { return "Frontier Explorer (Partial)"; }

    void OnMazeChanged(int w, int h) override;
    void Reset(CellPos start, CellPos exit) override;
    void Start() override;
    void Tick() override;

private:
    const IPartialEnvironment* m_env{nullptr};
    KnowledgeMap m_kmap;
    std::deque<CellPos> m_plan;
    CellPos m_target{-1,-1};

    void UpdateKnowledgeAt(CellPos at);
    bool IsFrontier(CellPos p) const;
    bool FindNearestFrontier(CellPos from, CellPos& outTarget);
    bool PlanPath(CellPos from, CellPos to);
};

} // namespace ml
