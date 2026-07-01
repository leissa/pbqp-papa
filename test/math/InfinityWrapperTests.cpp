#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <cstdint>

#include <doctest/doctest.h>

#include "math/InfinityWrapper.hpp"

namespace pbqppapa {

TEST_CASE("infinityLimitTest") {

	InfinityWrapper<uint32_t> inful = InfinityWrapper<uint32_t>::getInfinite();
	InfinityWrapper<uint32_t> inful2(4294967295);
	CHECK(inful == inful2);
	CHECK(inful.isInfinite());
	CHECK(inful2.isInfinite());
	MESSAGE(std::to_string(inful.getValue()));
	MESSAGE(std::to_string(inful2.getValue()));
}

} // namespace pbqppapa
