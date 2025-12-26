#pragma once
#include <vector>
#include "core/Types.h"

namespace ml {

struct PathResult {
    bool found{false};
    std::vector<CellPos> path; // includes start and end
    int expandedNodes{0};
};

} // namespace ml
