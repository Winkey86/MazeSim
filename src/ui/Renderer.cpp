#include "ui/Renderer.h"
#include <algorithm>
#include <cstdint>
#include <cmath>

namespace ml::ui {

Renderer::Renderer() = default;

ml::CellPos Renderer::WorldToCell(sf::Vector2f world, const Simulation& sim) const {
    int x = (int)std::floor(world.x / m_tile);
    int y = (int)std::floor(world.y / m_tile);
    x = std::clamp(x, 0, sim.GetMaze().Width() - 1);
    y = std::clamp(y, 0, sim.GetMaze().Height() - 1);
    return {x,y};
}

void Renderer::Draw(sf::RenderTarget& rt, const Simulation& sim, const sf::View& view, bool partialShading) const {
    (void)view;

    const auto& maze = sim.GetMaze();
    const int w = maze.Width();
    const int h = maze.Height();

    const ml::IAgent* agent = sim.ActiveAgent();
    const auto& visited = agent ? agent->VisitedMask() : std::vector<uint8_t>();
    const auto& frontier = agent ? agent->FrontierMask() : std::vector<uint8_t>();

    // Use vertex array for tiles (fast)
    sf::VertexArray va(sf::PrimitiveType::Triangles);
    va.resize((size_t)w * (size_t)h * 6);

    auto tileColor = [&](int x, int y) -> sf::Color {
        ml::CellPos p{x,y};
        bool wall = maze.IsWall(p);
        sf::Color c = wall ? sf::Color(40,40,40) : sf::Color(95,95,95);

        int idx = ml::ToIndex(x,y,w);
        if (!wall && agent) {
            if (!frontier.empty() && frontier[(size_t)idx]) c = sf::Color(120,120,255);
            if (!visited.empty() && visited[(size_t)idx]) c = sf::Color(180,180,180);
        }

        if (partialShading && !wall) {
            // unknown shading: if not visited and not frontier -> darker
            if (agent && !visited.empty() && !visited[(size_t)idx] && (!frontier.empty() && !frontier[(size_t)idx])) {
                c.r = static_cast<std::uint8_t>(static_cast<float>(c.r) * 0.55f);
                c.g = static_cast<std::uint8_t>(static_cast<float>(c.g) * 0.55f);
                c.b = static_cast<std::uint8_t>(static_cast<float>(c.b) * 0.55f);
            }
        }
        return c;
    };

    size_t v = 0;
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            float x0 = x * m_tile;
            float y0 = y * m_tile;
            float x1 = x0 + m_tile;
            float y1 = y0 + m_tile;

            sf::Color c = tileColor(x,y);

            // two triangles
            va[v + 0] = sf::Vertex({x0,y0}, c);
            va[v + 1] = sf::Vertex({x1,y0}, c);
            va[v + 2] = sf::Vertex({x1,y1}, c);

            va[v + 3] = sf::Vertex({x0,y0}, c);
            va[v + 4] = sf::Vertex({x1,y1}, c);
            va[v + 5] = sf::Vertex({x0,y1}, c);
            v += 6;
        }
    }
    rt.draw(va);

    // Shortest path overlay (shown after success)
    if (sim.ShouldDrawShortestPath()) {
        const auto& mask = sim.ShortestPathMask();
        if (!mask.empty()) {
            sf::VertexArray pathVa(sf::PrimitiveType::Triangles);
            sf::Color pc = sf::Color::Red;
            pc.a = 170;

            for (int y = 0; y < h; ++y) {
                for (int x = 0; x < w; ++x) {
                    int idx = ml::ToIndex(x, y, w);
                    if (!mask[(size_t)idx]) continue;
                    if (maze.IsWall({x,y})) continue;

                    float x0 = x * m_tile;
                    float y0 = y * m_tile;
                    float x1 = x0 + m_tile;
                    float y1 = y0 + m_tile;

                    pathVa.append(sf::Vertex({x0,y0}, pc));
                    pathVa.append(sf::Vertex({x1,y0}, pc));
                    pathVa.append(sf::Vertex({x1,y1}, pc));

                    pathVa.append(sf::Vertex({x0,y0}, pc));
                    pathVa.append(sf::Vertex({x1,y1}, pc));
                    pathVa.append(sf::Vertex({x0,y1}, pc));
                }
            }
            rt.draw(pathVa);
        }
    }

    // grid overlay
    sf::VertexArray lines(sf::PrimitiveType::Lines);
    const sf::Color gridC(255,255,255,28);
    // vertical
    for (int x = 0; x <= w; ++x) {
        float X = x * m_tile;
        lines.append(sf::Vertex({X, 0.f}, gridC));
        lines.append(sf::Vertex({X, h * m_tile}, gridC));
    }
    for (int y = 0; y <= h; ++y) {
        float Y = y * m_tile;
        lines.append(sf::Vertex({0.f, Y}, gridC));
        lines.append(sf::Vertex({w * m_tile, Y}, gridC));
    }
    rt.draw(lines);

    // draw start/exit
    sf::RectangleShape mark({m_tile, m_tile});
    mark.setFillColor(sf::Color(80,200,120,200));
    mark.setPosition({1.f * m_tile, 1.f * m_tile});
    rt.draw(mark);

    sf::RectangleShape exit({m_tile, m_tile});
    exit.setFillColor(sf::Color(220,120,120,220));
    exit.setPosition({(float)(w-2) * m_tile, (float)(h-2) * m_tile});
    rt.draw(exit);

    // draw agent
    if (agent) {
        sf::CircleShape dot(m_tile * 0.35f);
        dot.setFillColor(sf::Color::Cyan);
        dot.setPosition({agent->Position().x * m_tile + m_tile*0.15f, agent->Position().y * m_tile + m_tile*0.15f});
        rt.draw(dot);
    }
}

} // namespace ml::ui
