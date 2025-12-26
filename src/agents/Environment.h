#pragma once
#include "core/Types.h"
#include "core/Directions.h"

namespace ml {

struct Sense4 {
    // true means wall/blocked in that direction
    bool n{true}, e{true}, s{true}, w{true};
};

class IPartialEnvironment {
public:
    virtual ~IPartialEnvironment() = default;

    virtual int Width() const = 0;
    virtual int Height() const = 0;
    virtual CellPos GetExit() const = 0;

    virtual Sense4 SenseWalls4(CellPos at) const = 0;

    // Attempt move by one tile. Returns true if moved; false if wall.
    virtual bool TryMove(CellPos from, Dir dir, CellPos& outNewPos) const = 0;
};

class IFullEnvironment : public IPartialEnvironment {
public:
    virtual ~IFullEnvironment() = default;
    virtual bool IsWall(CellPos p) const = 0;
    virtual bool IsFree(CellPos p) const = 0;
};

} // namespace ml
