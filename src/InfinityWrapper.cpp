#include "math/InfinityWrapper.hpp"

namespace pbqppapa {

template <>
uint64_t InfinityWrapper<uint64_t>::magicInfiniteNumber = UINT64_MAX;
template <>
uint32_t InfinityWrapper<uint32_t>::magicInfiniteNumber = UINT32_MAX;
template <>
uint16_t InfinityWrapper<uint16_t>::magicInfiniteNumber = UINT16_MAX;

} // namespace pbqppapa
