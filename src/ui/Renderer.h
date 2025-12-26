#pragma once
#include <SFML/Graphics.hpp>
#include "sim/Simulation.h"

namespace ml::ui {

class Renderer {
public:
    Renderer();

    void SetTileSize(float px) { m_tile = px; }
    float TileSize() const noexcept { return m_tile; }

    void Draw(sf::RenderTarget& rt, const Simulation& sim, const sf::View& view, bool partialShading) const;

    // Convert from world to cell
    ml::CellPos WorldToCell(sf::Vector2f world, const Simulation& sim) const;

private:
    float m_tile{12.f};
};

} // namespace ml::ui
