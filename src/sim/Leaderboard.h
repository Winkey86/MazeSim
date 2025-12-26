#pragma once
#include <string>
#include "agents/AgentMetrics.h"

namespace ml {

struct RunConfigSnapshot {
    int width{31};
    int height{31};
    std::string generator;
    std::string visibilityMode;
    std::string agentName;
    uint32_t seed{1};
    bool randomSeed{false};
};

struct LeaderboardEntry {
    std::string timestampIso;
    RunConfigSnapshot cfg;
    AgentMetrics metrics;
};

class Leaderboard {
public:
    explicit Leaderboard(std::string csvPath);

    void Append(const LeaderboardEntry& e);

    const std::string& Path() const noexcept { return m_path; }

private:
    std::string m_path;
    bool m_headerWritten{false};
    void EnsureHeader();
};

} // namespace ml
