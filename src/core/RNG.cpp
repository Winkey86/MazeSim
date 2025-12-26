#include "core/RNG.h"

namespace ml {

RNG::RNG() : m_rng(std::random_device{}()) {}
RNG::RNG(uint32_t seed) : m_rng(seed) {}

void RNG::Seed(uint32_t seed) { m_rng.seed(seed); }

uint32_t RNG::NextU32() { return static_cast<uint32_t>(m_rng()); }

int RNG::NextInt(int lo, int hiInclusive) {
    std::uniform_int_distribution<int> dist(lo, hiInclusive);
    return dist(m_rng);
}

} // namespace ml
