#pragma once
#include <vector>
#include <cstdint>
#include "core/Types.h"
#include "core/Directions.h"

namespace ml {

class Maze {
public:
    Maze() = default;
    Maze(int w, int h);

    void Resize(int w, int h);
    int Width() const noexcept { return m_w; }
    int Height() const noexcept { return m_h; }

    bool InBounds(CellPos p) const noexcept;
    bool IsWall(CellPos p) const noexcept;
    bool IsFree(CellPos p) const noexcept;

    void FillWalls();
    void SetFree(CellPos p, bool free);

    // Move on the tile grid: adjacent step to a free tile.
    bool CanMove(CellPos from, Dir dir) const noexcept;
    CellPos Step(CellPos from, Dir dir) const noexcept;

    // Helpers for classic maze carving with odd coordinates.
    bool IsOddCell(CellPos p) const noexcept;
    std::vector<CellPos> OddNeighbors2(CellPos p) const; // neighbors at distance 2 (odd->odd)
    void CarvePassage(CellPos aOdd, CellPos bOdd); // opens a, b and the middle tile

private:
    int m_w{0};
    int m_h{0};
    std::vector<uint8_t> m_wall; // 1 = wall, 0 = free
};

} // namespace ml
