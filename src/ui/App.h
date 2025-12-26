#pragma once
#include <SFML/Graphics.hpp>
#include "sim/Simulation.h"
#include "ui/Widgets.h"
#include "ui/Renderer.h"

namespace ml::ui {

class App {
public:
    App();
    void Run();

private:
    void HandleEvent(const sf::Event& e);
    void HandlePanelEvent(const sf::Event& e);
    void LayoutUI();
    void LayoutUIForTarget(const sf::Vector2u& targetSize, bool detached);
    void DrawPanel(sf::RenderTarget& rt, const sf::Vector2u& targetSize, bool detached);
    sf::FloatRect PanelRect(const sf::Vector2u& targetSize, bool detached) const;
    sf::FloatRect PanelHeaderRect(const sf::Vector2u& targetSize, bool detached) const;
    bool IsMouseOverPanel(const sf::Vector2f& mouse, const sf::Vector2u& targetSize) const;
    void SetPanelDetached(bool detached);
    void SyncUIToSim();
    void SyncSimToUI();
    void RefreshAgentSelector();

    sf::RenderWindow m_window;
    sf::View m_view;

    FontCache m_fonts;
    Style m_style;
    Renderer m_renderer;

    Simulation m_sim;

    // UI elements
    StepperInt m_wStep;
    StepperInt m_hStep;
    StepperInt m_seedStep;
    Toggle m_randomSeed;

    CycleSelector m_genSel;
    CycleSelector m_visSel;
    CycleSelector m_agentSel;

    // maps m_agentSel.selected -> SimConfig.agentIndex
    std::vector<int> m_agentIds;

    Button m_btnGenerate;
    Button m_btnStart;
    Button m_btnPause;
    Button m_btnStep;
    Button m_btnReset;

    SliderInt m_speed;

    Button m_btnSaveCsv;
    Button m_btnHidePanel;
    Button m_btnDetachPanel;
    Button m_btnShowPanel;

    // camera
    bool m_panning{false};
    sf::Vector2i m_panStartMouse{};
    sf::Vector2f m_panStartCenter{};

    // panel (left by default)
    bool m_panelVisible{true};
    bool m_panelDetached{false};
    sf::RenderWindow m_panelWindow;

    sf::Vector2f m_panelPos{10.f, 60.f};
    float m_panelW{360.f};
    float m_panelH{520.f};
    float m_panelHeaderH{28.f};

    bool m_panelDragging{false};
    sf::Vector2f m_panelDragOffset{};

    // small toast ("Saved")
    std::string m_toast;
    float m_toastTimer{0.f};
    sf::Clock m_frameClock;

    // time-based ticking (keeps speed independent from FPS)
    float m_tickBudget{0.f};
};

} // namespace ml::ui
