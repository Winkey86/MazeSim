#pragma once
#include <cstdint>
#include <string>

namespace ml {

struct CellPos {
    int x{0};
    int y{0};

    bool operator==(const CellPos& o) const noexcept { return x == o.x && y == o.y; }
    bool operator!=(const CellPos& o) const noexcept { return !(*this == o); }
};

inline int ToIndex(int x, int y, int w) { return y * w + x; }

enum class AgentStatus : uint8_t { Running, Success, Fail };

inline const char* ToString(AgentStatus s) {
    switch (s) {
    case AgentStatus::Running: return "RUNNING";
    case AgentStatus::Success: return "SUCCESS";
    case AgentStatus::Fail: return "FAIL";
    default: return "UNKNOWN";
    }
}

} // namespace ml
