#pragma once
#include <memory>
#include <string>
#include <chrono>
#include <vector>

#include "core/Maze.h"
#include "core/RNG.h"
#include "generators/IMazeGenerator.h"
#include "agents/Environment.h"
#include "agents/IAgent.h"
#include "sim/Leaderboard.h"

namespace ml {

enum class VisibilityMode : uint8_t { Full, Partial };

struct SimConfig {
    int width{31};
    int height{31};
    uint32_t seed{1};
    bool randomSeed{false};
    int generatorIndex{0}; // 0=DFS, 1=Prim, 2=CellularAutomata
    VisibilityMode visibility{VisibilityMode::Full};
    int agentIndex{0}; // 0=BFS,1=A*,2=Right,3=Frontier,4=Manual
};

class SimEnvironmentFull final : public IFullEnvironment {
public:
    explicit SimEnvironmentFull(const Maze* maze, CellPos exit) : m_maze(maze), m_exit(exit) {}

    int Width() const override;
    int Height() const override;
    CellPos GetExit() const override { return m_exit; }

    Sense4 SenseWalls4(CellPos at) const override;
    bool TryMove(CellPos from, Dir dir, CellPos& outNewPos) const override;

    bool IsWall(CellPos p) const override;
    bool IsFree(CellPos p) const override;

    void SetExit(CellPos e) { m_exit = e; }
    void BindMaze(const Maze* m) { m_maze = m; }

private:
    const Maze* m_maze{nullptr};
    CellPos m_exit{1,1};
};

class SimEnvironmentPartial final : public IPartialEnvironment {
public:
    explicit SimEnvironmentPartial(const Maze* maze, CellPos exit) : m_maze(maze), m_exit(exit) {}

    int Width() const override;
    int Height() const override;
    CellPos GetExit() const override { return m_exit; }

    Sense4 SenseWalls4(CellPos at) const override;
    bool TryMove(CellPos from, Dir dir, CellPos& outNewPos) const override;

    void SetExit(CellPos e) { m_exit = e; }
    void BindMaze(const Maze* m) { m_maze = m; }

private:
    const Maze* m_maze{nullptr};
    CellPos m_exit{1,1};
};

class Simulation {
public:
    Simulation();

    const Maze& GetMaze() const noexcept { return m_maze; }
    Maze& GetMaze() noexcept { return m_maze; }

    const SimConfig& GetConfig() const noexcept { return m_cfg; }
    SimConfig& GetConfig() noexcept { return m_cfg; }

    bool IsRunning() const noexcept { return m_running; }
    bool IsPaused() const noexcept { return m_paused; }

    int TicksPerFrame() const noexcept { return m_ticksPerFrame; }
    void SetTicksPerFrame(int t);

    void GenerateMaze();
    void ResetAgent();
    void Start();
    void PauseToggle();
    void StepOnce();
    void StopReset();

    void TickMany(int ticks);

    // UI: shortest path highlight (computed when agent finishes SUCCESS)
    const std::vector<uint8_t>& ShortestPathMask() const noexcept { return m_shortestPathMask; }
    bool ShouldDrawShortestPath() const noexcept;

    // Save current simulation state to leaderboard.csv (even if still running).
    // Useful for a "Save to CSV" UI button.
    bool SaveSnapshotToLeaderboard();

    // For UI: manual click
    void ManualRightClick(CellPos target);

    const IAgent* ActiveAgent() const noexcept { return m_agent.get(); }
    IAgent* ActiveAgent() noexcept { return m_agent.get(); }

    std::string ActiveAgentName() const;

    // Called when run ends to append leaderboard
    void AttachLeaderboard(std::shared_ptr<Leaderboard> lb) { m_leaderboard = std::move(lb); }

private:
    LeaderboardEntry MakeLeaderboardEntry() const;
    void BuildAgent();
    void FinishIfNeeded();

private:
    Maze m_maze;
    SimConfig m_cfg;

    std::unique_ptr<IMazeGenerator> m_genDFS;
    std::unique_ptr<IMazeGenerator> m_genPrim;
    std::unique_ptr<IMazeGenerator> m_genCellular;

    CellPos m_start{1,1};
    CellPos m_exit{1,1};

    SimEnvironmentFull m_envFull;
    SimEnvironmentPartial m_envPartial;

    std::unique_ptr<IAgent> m_agent;

    bool m_running{false};
    bool m_paused{false};

    int m_ticksPerFrame{1};
    int m_stepLimit{0};
    int m_totalTicks{0};

    std::shared_ptr<Leaderboard> m_leaderboard;

    // shortest path overlay (mask of tiles)
    std::vector<uint8_t> m_shortestPathMask;

    // duration accounting (logic-only)
    std::chrono::steady_clock::time_point m_runStart;
};

} // namespace ml
