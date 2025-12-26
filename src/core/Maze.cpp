#include "core/Maze.h"
#include <algorithm>

namespace ml {

Maze::Maze(int w, int h) { Resize(w, h); }

void Maze::Resize(int w, int h) {
    m_w = std::max(3, w);
    m_h = std::max(3, h);
    m_wall.assign(static_cast<size_t>(m_w * m_h), 1u);
}

bool Maze::InBounds(CellPos p) const noexcept {
    return p.x >= 0 && p.y >= 0 && p.x < m_w && p.y < m_h;
}

bool Maze::IsWall(CellPos p) const noexcept {
    if (!InBounds(p)) return true;
    return m_wall[static_cast<size_t>(ToIndex(p.x, p.y, m_w))] != 0;
}

bool Maze::IsFree(CellPos p) const noexcept { return !IsWall(p); }

void Maze::FillWalls() {
    std::fill(m_wall.begin(), m_wall.end(), 1u);
}

void Maze::SetFree(CellPos p, bool free) {
    if (!InBounds(p)) return;
    m_wall[static_cast<size_t>(ToIndex(p.x, p.y, m_w))] = free ? 0u : 1u;
}

CellPos Maze::Step(CellPos from, Dir dir) const noexcept {
    const auto d = Delta(dir);
    return {from.x + d.dx, from.y + d.dy};
}

bool Maze::CanMove(CellPos from, Dir dir) const noexcept {
    CellPos to = Step(from, dir);
    return InBounds(to) && IsFree(to);
}

bool Maze::IsOddCell(CellPos p) const noexcept {
    return (p.x % 2 == 1) && (p.y % 2 == 1) && p.x > 0 && p.y > 0 && p.x < m_w - 1 && p.y < m_h - 1;
}

std::vector<CellPos> Maze::OddNeighbors2(CellPos p) const {
    std::vector<CellPos> out;
    // step by 2
    const CellPos cand[4] = { {p.x, p.y-2}, {p.x+2, p.y}, {p.x, p.y+2}, {p.x-2, p.y} };
    for (auto c : cand) {
        if (IsOddCell(c)) out.push_back(c);
    }
    return out;
}

void Maze::CarvePassage(CellPos aOdd, CellPos bOdd) {
    // open endpoints and middle
    SetFree(aOdd, true);
    SetFree(bOdd, true);
    CellPos mid{ (aOdd.x + bOdd.x) / 2, (aOdd.y + bOdd.y) / 2 };
    SetFree(mid, true);
}

} // namespace ml
