#include "sim/Leaderboard.h"
#include <fstream>
#include <filesystem>

namespace ml {

Leaderboard::Leaderboard(std::string csvPath) : m_path(std::move(csvPath)) {}

void Leaderboard::EnsureHeader() {
    if (m_headerWritten) return;

    const bool exists = std::filesystem::exists(m_path);
    if (exists) {
        m_headerWritten = true;
        return;
    }

    std::ofstream out(m_path, std::ios::binary);
    out << "timestamp,width,height,generator,visibility,agent,seed,random_seed,"
           "status,steps,path_length,visited_unique,expanded_nodes,replans,duration_ms\n";
    m_headerWritten = true;
}

void Leaderboard::Append(const LeaderboardEntry& e) {
    EnsureHeader();
    std::ofstream out(m_path, std::ios::binary | std::ios::app);
    out
        << e.timestampIso << ","
        << e.cfg.width << ","
        << e.cfg.height << ","
        << '"' << e.cfg.generator << '"' << ","
        << e.cfg.visibilityMode << ","
        << '"' << e.cfg.agentName << '"' << ","
        << e.cfg.seed << ","
        << (e.cfg.randomSeed ? 1 : 0) << ","
        << ToString(e.metrics.status) << ","
        << e.metrics.steps << ","
        << e.metrics.path_length << ","
        << e.metrics.visited_unique << ","
        << e.metrics.expanded_nodes << ","
        << e.metrics.replans << ","
        << e.metrics.duration_ms
        << "\n";
}

} // namespace ml
