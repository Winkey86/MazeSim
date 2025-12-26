#pragma once
#include <vector>
#include <cstdint>
#include "core/Types.h"

namespace ml {

enum class Know : uint8_t { Unknown = 0, Wall = 1, Free = 2 };

class KnowledgeMap {
public:
    void Resize(int w, int h) {
        m_w = w; m_h = h;
        m_cells.assign(static_cast<size_t>(w*h), Know::Unknown);
    }

    int Width() const noexcept { return m_w; }
    int Height() const noexcept { return m_h; }

    bool InBounds(CellPos p) const noexcept {
        return p.x >= 0 && p.y >= 0 && p.x < m_w && p.y < m_h;
    }

    Know Get(CellPos p) const noexcept {
        if (!InBounds(p)) return Know::Wall;
        return m_cells[static_cast<size_t>(ToIndex(p.x,p.y,m_w))];
    }

    void Set(CellPos p, Know k) {
        if (!InBounds(p)) return;
        m_cells[static_cast<size_t>(ToIndex(p.x,p.y,m_w))] = k;
    }

    const std::vector<Know>& Raw() const noexcept { return m_cells; }

private:
    int m_w{0}, m_h{0};
    std::vector<Know> m_cells;
};

} // namespace ml
