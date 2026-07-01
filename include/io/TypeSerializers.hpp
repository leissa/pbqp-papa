#pragma once

#include <string>

#include "math/InfinityWrapper.hpp"

namespace pbqppapa {

/**
 * Dark magic to allow serialization to properly handle both int and InfinityWrapper<int>.
 * I'm blaming C++ for this one
 */
template <typename T>
struct dummyType {};

// 1

template <typename T>
std::string serializeElement(T t) {
	return serializeElement2(dummyType<T>{}, t);
}

template <typename T>
T deserializeElement(std::string serialized) {
	return deserializeElement2(dummyType<T>{}, serialized);
}

template <typename T>
std::string getTypeName() {
	return getTypeName2(dummyType<T>{});
}

// 2
std::string serializeElement2(dummyType<uint64_t>, uint64_t element);

uint64_t deserializeElement2(dummyType<uint64_t>, std::string serialized);

std::string getTypeName2(dummyType<uint64_t>);

std::string serializeElement2(dummyType<uint32_t>, uint32_t element);

uint32_t deserializeElement2(dummyType<uint32_t>, std::string serialized);

std::string getTypeName2(dummyType<uint32_t>);

std::string serializeElement2(dummyType<uint16_t>, uint16_t element);

uint16_t deserializeElement2(dummyType<uint16_t>, std::string serialized);

std::string getTypeName2(dummyType<uint16_t>);

std::string serializeElement2(dummyType<int64_t>, int64_t element);

uint64_t deserializeElement2(dummyType<int64_t>, std::string serialized);

std::string getTypeName2(dummyType<int64_t>);

std::string serializeElement2(dummyType<int32_t>, int32_t element);

uint32_t deserializeElement2(dummyType<int32_t>, std::string serialized);

std::string getTypeName2(dummyType<int32_t>);

std::string serializeElement2(dummyType<int16_t>, int16_t element);

uint16_t deserializeElement2(dummyType<int16_t>, std::string serialized);

std::string getTypeName2(dummyType<int16_t>);

// InfinityWrapper

template <typename T>
std::string serializeElement2(dummyType<InfinityWrapper<T>>, InfinityWrapper<T> number) {
	if (number.isInfinite()) {
		return "∞";
	}
	return serializeElement2(dummyType<T>{}, number.getValue());
}

template <typename T>
InfinityWrapper<T> deserializeElement2(dummyType<InfinityWrapper<T>>, std::string serialized) {
	if (serialized == "∞" || serialized == "INF") {
		return InfinityWrapper<T>::getInfinite();
	}
	return InfinityWrapper<T>(deserializeElement<T>(serialized));
}

template <typename T>
std::string getTypeName2(dummyType<InfinityWrapper<T>>) {
	return "INF " + getTypeName<T>();
}

} // namespace pbqppapa
