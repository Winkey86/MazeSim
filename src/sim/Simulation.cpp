#include "sim/Simulation.h"

#include "generators/RecursiveBacktrackerGenerator.h"
#include "generators/PrimGenerator.h"
#include "generators/CellularAutomataGenerator.h"

#include "pathfinding/BFSPathfinder.h"

#include "agents/BFSAgent.h"
#include "agents/AStarAgent.h"
#include "agents/RightHandAgent.h"
#include "agents/FrontierExplorerAgent.h"
#include "agents/ManualAgent.h"

#include <algorithm>
#include <sstream>
#include <iomanip>
#include <ctime>

namespace ml {

static int MakeOddSize(int v) {
    v = std::clamp(v, 10, 200);
    if (v % 2 == 0) {
        if (v < 200) v += 1;
        else v -= 1;
    }
    return std::clamp(v, 11, 199); // keep inside and odd
}

int SimEnvironmentFull::Width() const { return m_maze ? m_maze->Width() : 0; }
int SimEnvironmentFull::Height() const { return m_maze ? m_maze->Height() : 0; }

Sense4 SimEnvironmentFull::SenseWalls4(CellPos at) const {
    Sense4 s{};
    if (!m_maze) return s;
    s.n = m_maze->IsWall({at.x, at.y-1});
    s.e = m_maze->IsWall({at.x+1, at.y});
    s.s = m_maze->IsWall({at.x, at.y+1});
    s.w = m_maze->IsWall({at.x-1, at.y});
    return s;
}

bool SimEnvironmentFull::TryMove(CellPos from, Dir dir, CellPos& outNewPos) const {
    if (!m_maze) return false;
    CellPos to = m_maze->Step(from, dir);
    if (!m_maze->InBounds(to) || m_maze->IsWall(to)) return false;
    outNewPos = to;
    return true;
}

bool SimEnvironmentFull::IsWall(CellPos p) const { return m_maze ? m_maze->IsWall(p) : true; }
bool SimEnvironmentFull::IsFree(CellPos p) const { return m_maze ? m_maze->IsFree(p) : false; }

int SimEnvironmentPartial::Width() const { return m_maze ? m_maze->Width() : 0; }
int SimEnvironmentPartial::Height() const { return m_maze ? m_maze->Height() : 0; }

Sense4 SimEnvironmentPartial::SenseWalls4(CellPos at) const {
    Sense4 s{};
    if (!m_maze) return s;
    s.n = m_maze->IsWall({at.x, at.y-1});
    s.e = m_maze->IsWall({at.x+1, at.y});
    s.s = m_maze->IsWall({at.x, at.y+1});
    s.w = m_maze->IsWall({at.x-1, at.y});
    return s;
}

bool SimEnvironmentPartial::TryMove(CellPos from, Dir dir, CellPos& outNewPos) const {
    if (!m_maze) return false;
    CellPos to = m_maze->Step(from, dir);
    if (!m_maze->InBounds(to) || m_maze->IsWall(to)) return false;
    outNewPos = to;
    return true;
}

Simulation::Simulation()
    : m_genDFS(std::make_unique<RecursiveBacktrackerGenerator>())
    , m_genPrim(std::make_unique<PrimGenerator>())
    , m_genCellular(std::make_unique<CellularAutomataGenerator>())
    , m_envFull(&m_maze, m_exit)
    , m_envPartial(&m_maze, m_exit)
{
    m_cfg.width = 51;
    m_cfg.height = 51;
    m_cfg.seed = 1;
    m_cfg.randomSeed = true;
    m_cfg.generatorIndex = 0;
    m_cfg.visibility = VisibilityMode::Full;
    m_cfg.agentIndex = 0;
    GenerateMaze();
    ResetAgent();
}

void Simulation::SetTicksPerFrame(int t) {
    m_ticksPerFrame = std::clamp(t, 1, 50);
}

void Simulation::GenerateMaze() {
    m_cfg.width = MakeOddSize(m_cfg.width);
    m_cfg.height = MakeOddSize(m_cfg.height);

    m_start = {1,1};
    m_exit = {m_cfg.width - 2, m_cfg.height - 2};

    MazeGenConfig gc;
    gc.width = m_cfg.width;
    gc.height = m_cfg.height;
    gc.seed = m_cfg.seed;
    gc.randomSeed = m_cfg.randomSeed;
    gc.start = m_start;
    gc.exit = m_exit;

    if (m_cfg.generatorIndex == 0) {
        m_genDFS->Generate(m_maze, gc);
    } else if (m_cfg.generatorIndex == 1) {
        m_genPrim->Generate(m_maze, gc);
    } else {
        m_genCellular->Generate(m_maze, gc);
    }

    // reset shortest-path overlay on new maze
    m_shortestPathMask.clear();

    // ensure start/exit are free
    m_maze.SetFree(m_start, true);
    m_maze.SetFree(m_exit, true);

    m_envFull.BindMaze(&m_maze);
    m_envPartial.BindMaze(&m_maze);
    m_envFull.SetExit(m_exit);
    m_envPartial.SetExit(m_exit);

    m_stepLimit = m_maze.Width() * m_maze.Height() * 20;
}

void Simulation::BuildAgent() {
    // One agent at a time. Visibility restricts allowed agents to prevent "cheating".
    // Full    -> BFS / A* / Manual
    // Partial -> Right-hand / Frontier / Manual
    if (m_cfg.visibility == VisibilityMode::Full) {
        if (m_cfg.agentIndex == 2 || m_cfg.agentIndex == 3) m_cfg.agentIndex = 0;
    } else {
        if (m_cfg.agentIndex == 0 || m_cfg.agentIndex == 1) m_cfg.agentIndex = 2;
    }

    switch (m_cfg.agentIndex) {
    case 0: // BFS
        m_agent = std::make_unique<BFSAgent>(&m_envFull);
        break;
    case 1: // A*
        m_agent = std::make_unique<AStarAgent>(&m_envFull);
        break;
    case 2: // Right-hand
        m_agent = std::make_unique<RightHandAgent>(&m_envPartial);
        break;
    case 3: // Frontier
        m_agent = std::make_unique<FrontierExplorerAgent>(&m_envPartial);
        break;
    case 4: // Manual
    default:
        if (m_cfg.visibility == VisibilityMode::Partial) {
            m_agent = std::make_unique<ManualAgent>(&m_envPartial);
        } else {
            m_agent = std::make_unique<ManualAgent>(&m_envFull);
        }
        break;
    }

    if (m_agent) {
        m_agent->OnMazeChanged(m_maze.Width(), m_maze.Height());
        m_agent->Reset(m_start, m_exit);
    }
}

void Simulation::ResetAgent() {
    m_running = false;
    m_paused = false;
    m_totalTicks = 0;
    m_shortestPathMask.clear();
    BuildAgent();
}

void Simulation::Start() {
    if (!m_agent) BuildAgent();
    if (!m_agent) return;
    m_running = true;
    m_paused = false;
    m_totalTicks = 0;

    m_agent->Reset(m_start, m_exit);
    m_agent->Start();
    m_runStart = std::chrono::steady_clock::now();
}

void Simulation::PauseToggle() {
    if (!m_running) return;
    m_paused = !m_paused;
}

void Simulation::StepOnce() {
    if (!m_running) {
        // allow single-step by auto-starting
        Start();
        m_paused = true;
    }
    TickMany(1);
}

void Simulation::StopReset() {
    ResetAgent();
}

void Simulation::TickMany(int ticks) {
    if (!m_running || m_paused || !m_agent) return;

    for (int i = 0; i < ticks; ++i) {
        if (!m_running || !m_agent) break;

        auto t0 = std::chrono::steady_clock::now();
        m_agent->Tick();
        auto t1 = std::chrono::steady_clock::now();

        auto& m = const_cast<AgentMetrics&>(m_agent->Metrics());
        m.duration_ms += std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

        m_totalTicks++;

        if (m_totalTicks > m_stepLimit) {
            m_agent->RequestStopFail();
        }

        FinishIfNeeded();
    }
}

void Simulation::FinishIfNeeded() {
    if (!m_agent) return;
    if (m_agent->Status() == AgentStatus::Running) return;

    m_running = false;
    m_paused = false;

    // compute shortest path overlay when the agent succeeds
    if (m_agent->Status() == AgentStatus::Success) {
        BFSPathfinder bfs;
        auto r = bfs.FindPath(m_maze, m_start, m_exit);
        if (r.found) {
            const int w = m_maze.Width();
            const int h = m_maze.Height();
            m_shortestPathMask.assign(static_cast<size_t>(w * h), 0u);
            for (const auto& p : r.path) {
                if (m_maze.InBounds(p)) {
                    m_shortestPathMask[static_cast<size_t>(ToIndex(p.x, p.y, w))] = 1u;
                }
            }
        } else {
            m_shortestPathMask.clear();
        }
    } else {
        m_shortestPathMask.clear();
    }

    // append leaderboard
    if (m_leaderboard) m_leaderboard->Append(MakeLeaderboardEntry());
}

bool Simulation::ShouldDrawShortestPath() const noexcept {
    if (!m_agent) return false;
    return (m_agent->Status() == AgentStatus::Success) && !m_shortestPathMask.empty();
}

LeaderboardEntry Simulation::MakeLeaderboardEntry() const {
    // timestamp ISO
    auto now = std::chrono::system_clock::now();
    std::time_t tt = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &tt);
#else
    tm = *std::localtime(&tt);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S");

    RunConfigSnapshot snap;
    snap.width = m_cfg.width;
    snap.height = m_cfg.height;
    if (m_cfg.generatorIndex == 0) snap.generator = "DFS";
    else if (m_cfg.generatorIndex == 1) snap.generator = "Prim";
    else snap.generator = "Cellular";
    snap.visibilityMode = (m_cfg.visibility == VisibilityMode::Full) ? "Full" : "Partial";
    snap.agentName = m_agent ? m_agent->Name() : std::string{};
    snap.seed = m_cfg.seed;
    snap.randomSeed = m_cfg.randomSeed;

    LeaderboardEntry e;
    e.timestampIso = oss.str();
    e.cfg = snap;
    e.metrics = m_agent ? m_agent->Metrics() : AgentMetrics{};
    return e;
}

bool Simulation::SaveSnapshotToLeaderboard() {
    if (!m_leaderboard || !m_agent) return false;
    m_leaderboard->Append(MakeLeaderboardEntry());
    return true;
}

void Simulation::ManualRightClick(CellPos target) {
    if (!m_agent) return;
    // Only manual supports it; safe cast
    if (auto* man = dynamic_cast<ManualAgent*>(m_agent.get())) {
        man->OnRightClick(target);
    }
}

std::string Simulation::ActiveAgentName() const {
    return m_agent ? m_agent->Name() : std::string{};
}

} // namespace ml
