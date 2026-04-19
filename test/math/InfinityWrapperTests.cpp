#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "math/InfinityWrapper.hpp"

namespace pbqppapa {

TEST_CASE("infinityLimitTest") {

	InfinityWrapper<unsigned int> inful = InfinityWrapper<unsigned int>::getInfinite();
	InfinityWrapper<unsigned int> inful2(4294967295);
	CHECK(inful == inful2);
	CHECK(inful.isInfinite());
	CHECK(inful2.isInfinite());
	MESSAGE(std::to_string(inful.getValue()));
	MESSAGE(std::to_string(inful2.getValue()));
}

} // namespace pbqppapa
