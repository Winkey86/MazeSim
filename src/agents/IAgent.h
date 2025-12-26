#pragma once
#include <string>
#include <vector>
#include "core/Types.h"
#include "agents/AgentMetrics.h"

namespace ml {

class IAgent {
public:
    virtual ~IAgent() = default;
    virtual std::string Name() const = 0;

    virtual void OnMazeChanged(int w, int h) = 0;
    virtual void Reset(CellPos start, CellPos exit) = 0;
    virtual void Start() = 0;

    virtual void Tick() = 0; // a single logical tick

    virtual void RequestStopFail() = 0;

    virtual AgentStatus Status() const = 0;
    virtual const AgentMetrics& Metrics() const = 0;
    virtual CellPos Position() const = 0;

    // For rendering overlays (same-size grids, 0/1):
    virtual const std::vector<uint8_t>& VisitedMask() const = 0;
    virtual const std::vector<uint8_t>& FrontierMask() const = 0; // optional (may be empty or zeros)
};

} // namespace ml
