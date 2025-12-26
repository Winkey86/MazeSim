#include "ui/App.h"

#include <algorithm>
#include <memory>
#include <cmath>

namespace ml::ui {

static sf::Vector2f ToF(sf::Vector2i p) { return sf::Vector2f((float)p.x, (float)p.y); }

App::App() {
    // Main window
    auto desktop = sf::VideoMode::getDesktopMode();
    m_window.create(desktop, "MazeLab", sf::Style::Default);
    m_window.setVerticalSyncEnabled(true);

    m_view = m_window.getDefaultView();
    m_view.setCenter({0.f, 0.f});

    m_fonts.LoadDefault();
    m_renderer.SetTileSize(12.f);

    // Attach leaderboard
    m_sim.AttachLeaderboard(std::make_shared<ml::Leaderboard>("leaderboard.csv"));

    // Defaults
    m_wStep.label = "W"; m_wStep.min = 10; m_wStep.max = 200; m_wStep.value = m_sim.GetConfig().width;
    m_hStep.label = "H"; m_hStep.min = 10; m_hStep.max = 200; m_hStep.value = m_sim.GetConfig().height;
    m_seedStep.label = "Seed"; m_seedStep.min = 0; m_seedStep.max = 999999; m_seedStep.value = (int)m_sim.GetConfig().seed;
    m_randomSeed.label = "Random seed"; m_randomSeed.value = m_sim.GetConfig().randomSeed;

    m_genSel.label = "Generator";
    m_genSel.SetItems({"DFS", "Prim", "Cellular"});
    m_genSel.selected = m_sim.GetConfig().generatorIndex;

    m_visSel.label = "Visibility";
    m_visSel.SetItems({"Full", "Partial"});
    m_visSel.selected = (m_sim.GetConfig().visibility == ml::VisibilityMode::Full) ? 0 : 1;

    m_agentSel.label = "Agent";
    RefreshAgentSelector();

    m_btnGenerate.label = "Generate";
    m_btnStart.label = "Start";
    m_btnPause.label = "Pause/Resume";
    m_btnStep.label = "Step";
    m_btnReset.label = "Reset";
    m_btnSaveCsv.label = "Save to CSV";

    m_btnHidePanel.label = "Hide";
    m_btnDetachPanel.label = "Detach";
    m_btnShowPanel.label = "Show Panel";

    m_speed.label = "Speed";
    m_speed.min = 1;
    m_speed.max = 25;
    m_speed.value = m_sim.TicksPerFrame();

    // Wiring
    m_btnGenerate.onClick = [&] {
        SyncSimToUI();
        m_sim.GenerateMaze();
        m_sim.ResetAgent();
    };
    m_btnStart.onClick = [&] {
        SyncSimToUI();
        m_sim.ResetAgent();
        m_sim.Start();
    };
    m_btnPause.onClick = [&] { m_sim.PauseToggle(); };
    m_btnStep.onClick = [&] { m_sim.StepOnce(); };
    m_btnReset.onClick = [&] { m_sim.StopReset(); };

    m_btnSaveCsv.onClick = [&] {
        if (m_sim.SaveSnapshotToLeaderboard()) {
            m_toast = "Saved to leaderboard.csv";
        } else {
            m_toast = "Nothing to save";
        }
        m_toastTimer = 2.0f;
    };

    m_btnHidePanel.onClick = [&] {
        if (m_panelDetached) {
            if (m_panelWindow.isOpen()) m_panelWindow.close();
            m_panelDetached = false;
        }
        m_panelVisible = false;
    };

    m_btnDetachPanel.onClick = [&] {
        SetPanelDetached(!m_panelDetached);
    };

    m_btnShowPanel.onClick = [&] {
        m_panelVisible = true;
    };

    // Default: left side
    m_panelPos = {10.f, 60.f};

    LayoutUI();

    // Center camera to maze
    m_view = m_window.getDefaultView();
    m_view.setCenter({(m_sim.GetMaze().Width() * m_renderer.TileSize()) * 0.5f,
                      (m_sim.GetMaze().Height() * m_renderer.TileSize()) * 0.5f});

    m_frameClock.restart();
}

sf::FloatRect App::PanelRect(const sf::Vector2u& targetSize, bool detached) const {
    if (detached) {
        return sf::FloatRect({0.f, 0.f}, {(float)targetSize.x, (float)targetSize.y});
    }
    return sf::FloatRect(m_panelPos, {m_panelW, m_panelH});
}

sf::FloatRect App::PanelHeaderRect(const sf::Vector2u& targetSize, bool detached) const {
    auto pr = PanelRect(targetSize, detached);
    return sf::FloatRect(pr.position, {pr.size.x, m_panelHeaderH});
}

bool App::IsMouseOverPanel(const sf::Vector2f& mouse, const sf::Vector2u& targetSize) const {
    if (!m_panelVisible) return false;
    if (m_panelDetached) return false;
    return PanelRect(targetSize, false).contains(mouse);
}

void App::SetPanelDetached(bool detached) {
    if (detached == m_panelDetached) return;

    if (detached) {
        // create separate window with the panel
        if (!m_panelWindow.isOpen()) {
            const unsigned w = (unsigned)std::max(320.f, m_panelW);
            const unsigned h = (unsigned)std::max(520.f, m_panelH);
            sf::VideoMode vm(sf::Vector2u(w, h));
            m_panelWindow.create(vm, "MazeLab Panel", sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize);
            m_panelWindow.setVerticalSyncEnabled(true);
        }
        m_panelDetached = true;
        m_panelVisible = true;
        m_btnDetachPanel.label = "Attach";
    } else {
        if (m_panelWindow.isOpen()) m_panelWindow.close();
        m_panelDetached = false;
        m_btnDetachPanel.label = "Detach";
        // keep visible (user can hide explicitly)
        m_panelVisible = true;
    }

    LayoutUI();
}

void App::LayoutUI() {
    // Keep panel height within the main window
    auto mainSz = m_window.getSize();
    m_panelH = std::min(m_panelH, std::max(300.f, (float)mainSz.y - 20.f));
    m_panelW = std::min(m_panelW, std::max(260.f, (float)mainSz.x - 20.f));

    LayoutUIForTarget(mainSz, false);
    if (m_panelDetached && m_panelWindow.isOpen()) {
        LayoutUIForTarget(m_panelWindow.getSize(), true);
    }
}

void App::LayoutUIForTarget(const sf::Vector2u& targetSize, bool detached) {
    auto pr = PanelRect(targetSize, detached);
    const float baseX = pr.position.x;
    const float baseY = pr.position.y;
    const float W = pr.size.x;

    // Header buttons
    m_btnHidePanel.rect = sf::FloatRect({baseX + W - 74.f, baseY + 4.f}, {70.f, m_panelHeaderH - 8.f});
    m_btnDetachPanel.rect = sf::FloatRect({baseX + W - 150.f, baseY + 4.f}, {72.f, m_panelHeaderH - 8.f});

    // Show button when hidden (only in main)
    m_btnShowPanel.rect = sf::FloatRect({8.f, 8.f}, {110.f, 28.f});

    // Content
    float x = baseX + 10.f;
    float y = baseY + m_panelHeaderH + 10.f;
    const float gap = 8.f;
    const float rowH = 34.f;
    const float full = W - 20.f;

    auto row = [&](float w) {
        sf::FloatRect r{{x, y}, {w, rowH}};
        x += w + gap;
        return r;
    };
    auto nextLine = [&] {
        x = baseX + 10.f;
        y += rowH + gap;
    };

    // W/H
    float half = (full - gap) * 0.5f;
    m_wStep.rect = row(half);
    m_hStep.rect = row(half);
    nextLine();

    // Seed
    m_seedStep.rect = row(full);
    nextLine();

    // Random seed toggle
    m_randomSeed.rect = row(full);
    nextLine();

    // Generator/Visibility/Agent
    m_genSel.rect = row(full);
    nextLine();
    m_visSel.rect = row(full);
    nextLine();
    m_agentSel.rect = row(full);
    nextLine();

    // Buttons
    m_btnGenerate.rect = row(half);
    m_btnStart.rect = row(half);
    nextLine();

    m_btnPause.rect = row(half);
    m_btnStep.rect = row(half);
    nextLine();

    m_btnReset.rect = row(half);
    m_btnSaveCsv.rect = row(half);
    nextLine();

    // Speed
    m_speed.rect = row(full);
}

void App::SyncSimToUI() {
    auto& cfg = m_sim.GetConfig();
    cfg.width = m_wStep.value;
    cfg.height = m_hStep.value;
    cfg.seed = (uint32_t)std::max(0, m_seedStep.value);
    cfg.randomSeed = m_randomSeed.value;
    cfg.generatorIndex = m_genSel.selected;
    cfg.visibility = (m_visSel.selected == 0) ? ml::VisibilityMode::Full : ml::VisibilityMode::Partial;
    if (!m_agentIds.empty() && m_agentSel.selected >= 0 && m_agentSel.selected < (int)m_agentIds.size()) {
        cfg.agentIndex = m_agentIds[(size_t)m_agentSel.selected];
    }
    m_sim.SetTicksPerFrame(m_speed.value);
}

void App::SyncUIToSim() {
    const auto& cfg = m_sim.GetConfig();
    m_wStep.value = cfg.width;
    m_hStep.value = cfg.height;
    m_seedStep.value = (int)cfg.seed;
    m_randomSeed.value = cfg.randomSeed;
    m_genSel.selected = cfg.generatorIndex;
    m_visSel.selected = (cfg.visibility == ml::VisibilityMode::Full) ? 0 : 1;
    RefreshAgentSelector();
    // map cfg.agentIndex to selector index
    int sel = 0;
    for (size_t i = 0; i < m_agentIds.size(); ++i) {
        if (m_agentIds[i] == cfg.agentIndex) { sel = (int)i; break; }
    }
    m_agentSel.selected = sel;
    m_speed.value = m_sim.TicksPerFrame();
}

void App::RefreshAgentSelector() {
    // Keep current agent choice if possible, otherwise fall back to first.
    int prevAgentIdx = 0;
    if (!m_agentIds.empty() && m_agentSel.selected >= 0 && m_agentSel.selected < (int)m_agentIds.size()) {
        prevAgentIdx = m_agentIds[(size_t)m_agentSel.selected];
    } else {
        prevAgentIdx = m_sim.GetConfig().agentIndex;
    }

    if (m_visSel.selected == 0) {
        m_agentIds = {0, 1, 4};
        m_agentSel.SetItems({"BFS", "A*", "Manual"});
    } else {
        m_agentIds = {2, 3, 4};
        m_agentSel.SetItems({"Right-hand", "Frontier", "Manual"});
    }

    int sel = 0;
    for (size_t i = 0; i < m_agentIds.size(); ++i) {
        if (m_agentIds[i] == prevAgentIdx) { sel = (int)i; break; }
    }
    m_agentSel.selected = sel;
}

void App::HandlePanelEvent(const sf::Event& e) {
    if (!m_panelWindow.isOpen()) return;

    if (e.is<sf::Event::Closed>()) {
        // treat as hide
        m_panelWindow.close();
        m_panelDetached = false;
        m_panelVisible = false;
        m_btnDetachPanel.label = "Detach";
        return;
    }
    if (e.is<sf::Event::Resized>()) {
        LayoutUI();
        return;
    }

    sf::Vector2f mouse = ToF(sf::Mouse::getPosition(m_panelWindow));

    // UI only
    m_wStep.Handle(e, mouse);
    m_hStep.Handle(e, mouse);
    m_seedStep.Handle(e, mouse);
    m_randomSeed.Handle(e, mouse);

    int prevVis = m_visSel.selected;
    m_genSel.Handle(e, mouse);
    m_visSel.Handle(e, mouse);
    if (m_visSel.selected != prevVis) {
        RefreshAgentSelector();
    }
    m_agentSel.Handle(e, mouse);

    m_btnGenerate.Handle(e, mouse);
    m_btnStart.Handle(e, mouse);
    m_btnPause.Handle(e, mouse);
    m_btnStep.Handle(e, mouse);
    m_btnReset.Handle(e, mouse);
    m_btnSaveCsv.Handle(e, mouse);

    m_speed.Handle(e, mouse);

    // Header buttons
    m_btnHidePanel.Handle(e, mouse);
    m_btnDetachPanel.Handle(e, mouse);
}

void App::HandleEvent(const sf::Event& e) {
    if (e.is<sf::Event::Closed>()) {
        m_window.close();
        return;
    }

    if (e.is<sf::Event::Resized>()) {
        LayoutUI();
        // keep view stable but re-center to maze
        m_view = sf::View(sf::FloatRect({0.f, 0.f}, {(float)m_window.getSize().x, (float)m_window.getSize().y}));
        m_view.setCenter({(m_sim.GetMaze().Width() * m_renderer.TileSize()) * 0.5f,
                          (m_sim.GetMaze().Height() * m_renderer.TileSize()) * 0.5f});
        return;
    }

    // Hotkeys
    if (e.is<sf::Event::KeyPressed>()) {
        auto kp = e.getIf<sf::Event::KeyPressed>();
        if (kp) {
            if (kp->code == sf::Keyboard::Key::P) {
                m_panelVisible = !m_panelVisible;
            }
            if (kp->code == sf::Keyboard::Key::F2) {
                SetPanelDetached(!m_panelDetached);
            }
            if (kp->code == sf::Keyboard::Key::S && (kp->control)) {
                if (m_btnSaveCsv.onClick) m_btnSaveCsv.onClick();
            }
        }
    }

    sf::Vector2f mouse = ToF(sf::Mouse::getPosition(m_window));
    bool overPanel = IsMouseOverPanel(mouse, m_window.getSize());

    // If panel hidden: show small button
    if (!m_panelVisible && !m_panelDetached) {
        m_btnShowPanel.Handle(e, mouse);
    }

    // Panel drag (attached)
    if (m_panelVisible && !m_panelDetached) {
        auto hdr = PanelHeaderRect(m_window.getSize(), false);
        if (e.is<sf::Event::MouseButtonPressed>()) {
            auto mb = e.getIf<sf::Event::MouseButtonPressed>();
            if (mb && mb->button == sf::Mouse::Button::Left) {
                if (hdr.contains(mouse) && !m_btnHidePanel.rect.contains(mouse) && !m_btnDetachPanel.rect.contains(mouse)) {
                    m_panelDragging = true;
                    m_panelDragOffset = mouse - m_panelPos;
                }
            }
        }
        if (e.is<sf::Event::MouseButtonReleased>()) {
            auto mb = e.getIf<sf::Event::MouseButtonReleased>();
            if (mb && mb->button == sf::Mouse::Button::Left) {
                m_panelDragging = false;
            }
        }
        if (e.is<sf::Event::MouseMoved>() && m_panelDragging) {
            auto mm = e.getIf<sf::Event::MouseMoved>();
            if (mm) {
                m_panelPos = mouse - m_panelDragOffset;
                // clamp to window
                auto ws = m_window.getSize();
                m_panelPos.x = std::clamp(m_panelPos.x, 0.f, std::max(0.f, (float)ws.x - m_panelW));
                m_panelPos.y = std::clamp(m_panelPos.y, 0.f, std::max(0.f, (float)ws.y - m_panelH));
                LayoutUI();
            }
        }
    }

    // mouse wheel zoom (only when not over panel)
    if (!overPanel && e.is<sf::Event::MouseWheelScrolled>()) {
        auto mw = e.getIf<sf::Event::MouseWheelScrolled>();
        if (mw) {
            float factor = (mw->delta > 0) ? 0.9f : 1.111111f;
            m_view.zoom(factor);
        }
    }

    // panning with LMB drag (only when not over panel)
    if (!overPanel && e.is<sf::Event::MouseButtonPressed>()) {
        auto mb = e.getIf<sf::Event::MouseButtonPressed>();
        if (mb && mb->button == sf::Mouse::Button::Left) {
            m_panning = true;
            m_panStartMouse = {mb->position.x, mb->position.y};
            m_panStartCenter = m_view.getCenter();
        }
    }
    if (e.is<sf::Event::MouseButtonReleased>()) {
        auto mb = e.getIf<sf::Event::MouseButtonReleased>();
        if (mb && mb->button == sf::Mouse::Button::Left) {
            m_panning = false;
        }
    }
    if (!overPanel && e.is<sf::Event::MouseMoved>() && m_panning) {
        auto mm = e.getIf<sf::Event::MouseMoved>();
        if (mm) {
            sf::Vector2i cur{mm->position.x, mm->position.y};
            sf::Vector2f delta = ToF(m_panStartMouse - cur);
            auto vs = m_view.getSize();
            auto ws = m_window.getSize();
            sf::Vector2f worldDelta{delta.x * (vs.x / (float)ws.x), delta.y * (vs.y / (float)ws.y)};
            m_view.setCenter(m_panStartCenter + worldDelta);
        }
    }

    // right click in maze => Manual target (only when not over panel)
    if (!overPanel && e.is<sf::Event::MouseButtonPressed>()) {
        auto mb = e.getIf<sf::Event::MouseButtonPressed>();
        if (mb && mb->button == sf::Mouse::Button::Right) {
            sf::Vector2f world = m_window.mapPixelToCoords({mb->position.x, mb->position.y}, m_view);
            ml::CellPos cell = m_renderer.WorldToCell(world, m_sim);
            m_sim.ManualRightClick(cell);
        }
    }

    // UI input handling
    if (m_panelVisible && !m_panelDetached) {
        bool stepEditing = m_wStep.IsEditing() || m_hStep.IsEditing() || m_seedStep.IsEditing();
        if (overPanel || stepEditing) {
            m_wStep.Handle(e, mouse);
            m_hStep.Handle(e, mouse);
            m_seedStep.Handle(e, mouse);
        }
        if (overPanel) {
            m_randomSeed.Handle(e, mouse);

            int prevVis = m_visSel.selected;
            m_genSel.Handle(e, mouse);
            m_visSel.Handle(e, mouse);
            if (m_visSel.selected != prevVis) {
                RefreshAgentSelector();
            }
            m_agentSel.Handle(e, mouse);

            m_btnGenerate.Handle(e, mouse);
            m_btnStart.Handle(e, mouse);
            m_btnPause.Handle(e, mouse);
            m_btnStep.Handle(e, mouse);
            m_btnReset.Handle(e, mouse);
            m_btnSaveCsv.Handle(e, mouse);

            m_speed.Handle(e, mouse);

            // Header buttons last
            m_btnHidePanel.Handle(e, mouse);
            m_btnDetachPanel.Handle(e, mouse);
        }
    }
}

void App::DrawPanel(sf::RenderTarget& rt, const sf::Vector2u& targetSize, bool detached) {
    if (!m_panelVisible && !detached) {
        // Show a small button to reopen
        if (m_fonts.IsReady()) {
            m_btnShowPanel.Draw(rt, m_fonts.Get(), m_style);
        }
        return;
    }
    if (!m_panelVisible && detached) return;

    auto pr = PanelRect(targetSize, detached);

    // background
    sf::RectangleShape panel;
    panel.setPosition(pr.position);
    panel.setSize(pr.size);
    panel.setFillColor(m_style.panel);
    panel.setOutlineColor(m_style.outline);
    panel.setOutlineThickness(1.f);
    rt.draw(panel);

    // header bar
    sf::RectangleShape header;
    header.setPosition(pr.position);
    header.setSize({pr.size.x, m_panelHeaderH});
    header.setFillColor(sf::Color(15, 15, 15, 240));
    header.setOutlineColor(m_style.outline);
    header.setOutlineThickness(1.f);
    rt.draw(header);

    if (!m_fonts.IsReady()) return;
    const sf::Font& font = m_fonts.Get();

    // header title
    sf::Text title(font, detached ? "MazeLab Panel (Detached)" : "MazeLab Panel", 14);
    title.setFillColor(m_style.text);
    title.setPosition({pr.position.x + 10.f, pr.position.y + 6.f});
    rt.draw(title);

    // header buttons
    m_btnHidePanel.Draw(rt, font, m_style);
    m_btnDetachPanel.Draw(rt, font, m_style);

    // widgets
    m_wStep.Draw(rt, font, m_style);
    m_hStep.Draw(rt, font, m_style);
    m_seedStep.Draw(rt, font, m_style);
    m_randomSeed.Draw(rt, font, m_style);

    m_genSel.Draw(rt, font, m_style);
    m_visSel.Draw(rt, font, m_style);
    m_agentSel.Draw(rt, font, m_style);

    m_btnGenerate.Draw(rt, font, m_style);
    m_btnStart.Draw(rt, font, m_style);
    m_btnPause.Draw(rt, font, m_style);
    m_btnStep.Draw(rt, font, m_style);
    m_btnReset.Draw(rt, font, m_style);
    m_btnSaveCsv.Draw(rt, font, m_style);

    m_speed.Draw(rt, font, m_style);

    // status + toast
    auto* ag = m_sim.ActiveAgent();
    std::string status = ag ? std::string(ToString(ag->Status())) : "N/A";
    std::string line = "Agent: " + m_sim.ActiveAgentName() + " | Status: " + status +
                       "\nsteps: " + std::to_string(ag ? ag->Metrics().steps : 0) +
                       " | visited: " + std::to_string(ag ? ag->Metrics().visited_unique : 0) +
                       " | expanded: " + std::to_string(ag ? ag->Metrics().expanded_nodes : 0) +
                       "\nreplans: " + std::to_string(ag ? ag->Metrics().replans : 0) +
                       " | duration_ms: " + std::to_string(ag ? ag->Metrics().duration_ms : 0);

    float textY = m_speed.rect.position.y + m_speed.rect.size.y + 10.f;
    sf::Text t(font, line, 13);
    t.setFillColor(sf::Color(220, 220, 220));
    t.setPosition({pr.position.x + 10.f, textY});
    rt.draw(t);

    if (m_toastTimer > 0.f && !m_toast.empty()) {
        sf::Text toast(font, m_toast, 13);
        toast.setFillColor(m_style.accent);
        toast.setPosition({pr.position.x + 10.f, textY + 48.f});
        rt.draw(toast);
    }
}

void App::Run() {
    while (m_window.isOpen()) {
        float dt = m_frameClock.restart().asSeconds();
        if (m_toastTimer > 0.f) {
            m_toastTimer = std::max(0.f, m_toastTimer - dt);
        }

        // MAIN events (SFML 3: pollEvent() -> optional)
        while (auto ev = m_window.pollEvent()) {
            HandleEvent(*ev);
        }

        // PANEL events
        if (m_panelDetached && m_panelWindow.isOpen()) {
            while (auto ev = m_panelWindow.pollEvent()) {
                HandlePanelEvent(*ev);
            }
        }

        // sync speed
        m_sim.SetTicksPerFrame(m_speed.value);

        // run simulation ticks (time-based, independent from FPS)
        if (m_sim.IsRunning() && !m_sim.IsPaused()) {
            // Previously x1 roughly matched "1 tick per frame" with vsync (~60 tps).
            // Now we make it 5x slower: base 12 ticks/sec at x1.
            constexpr float baseTicksPerSecond = 12.0f;
            m_tickBudget += dt * baseTicksPerSecond * (float)m_sim.TicksPerFrame();
            int ticks = (int)std::floor(m_tickBudget);
            if (ticks > 0) {
                m_tickBudget -= (float)ticks;
                m_sim.TickMany(ticks);
            }
        } else {
            // when paused/stopped - don't accumulate
            m_tickBudget = 0.f;
        }

        // draw main
        m_window.clear(sf::Color(10, 10, 10));
        m_window.setView(m_view);
        bool partialShade = (m_visSel.selected == 1);
        m_renderer.Draw(m_window, m_sim, m_view, partialShade);

        m_window.setView(m_window.getDefaultView());
        if (!m_panelDetached) {
            DrawPanel(m_window, m_window.getSize(), false);
        } else {
            // even when detached, keep a small show button if the panel is hidden
            if (!m_panelVisible && m_fonts.IsReady()) {
                m_btnShowPanel.Draw(m_window, m_fonts.Get(), m_style);
            }
        }
        m_window.display();

        // draw detached panel window
        if (m_panelDetached && m_panelWindow.isOpen()) {
            m_panelWindow.clear(sf::Color(10, 10, 10));
            m_panelWindow.setView(m_panelWindow.getDefaultView());
            DrawPanel(m_panelWindow, m_panelWindow.getSize(), true);
            m_panelWindow.display();
        }
    }

    if (m_panelWindow.isOpen()) m_panelWindow.close();
}

} // namespace ml::ui
