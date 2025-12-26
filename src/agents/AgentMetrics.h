#pragma once
#include <cstdint>
#include <string>
#include "core/Types.h"

namespace ml {

struct AgentMetrics {
    int steps{0};
    int path_length{0};
    int visited_unique{0};
    int expanded_nodes{0};
    int replans{0};
    long long duration_ms{0};
    AgentStatus status{AgentStatus::Running};
};

} // namespace ml
