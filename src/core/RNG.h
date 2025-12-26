#pragma once
#include <cstdint>
#include <random>

namespace ml {

class RNG {
public:
    RNG();
    explicit RNG(uint32_t seed);

    void Seed(uint32_t seed);
    uint32_t NextU32();
    int NextInt(int lo, int hiInclusive);

    template <class It>
    void Shuffle(It begin, It end) {
        std::shuffle(begin, end, m_rng);
    }

private:
    std::mt19937 m_rng;
};

} // namespace ml
