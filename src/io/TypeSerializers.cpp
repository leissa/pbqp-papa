#include "io/TypeSerializers.hpp"

#include <cstdlib> // std::strtoul/std::strtoull (libc++ needs this explicitly)

namespace pbqppapa {

// The getTypeName strings below are the persisted serialization identifiers and are kept
// stable across the switch to fixed-width types so existing JSON files still load.

// uint64_t
std::string serializeElement2(dummyType<uint64_t>, uint64_t element) {
	return std::to_string(element);
}

uint64_t deserializeElement2(dummyType<uint64_t>, std::string serialized) {
	return static_cast<uint64_t>(std::strtoull(serialized.c_str(), nullptr, 0));
}

std::string getTypeName2(dummyType<uint64_t>) {
	return "unsigned long";
}

// uint32_t
std::string serializeElement2(dummyType<uint32_t>, uint32_t element) {
	return std::to_string(element);
}

uint32_t deserializeElement2(dummyType<uint32_t>, std::string serialized) {
	return static_cast<uint32_t>(std::strtoul(serialized.c_str(), nullptr, 0));
}

std::string getTypeName2(dummyType<uint32_t>) {
	return "unsigned int";
}

// uint16_t
std::string serializeElement2(dummyType<uint16_t>, uint16_t element) {
	return std::to_string(element);
}

uint16_t deserializeElement2(dummyType<uint16_t>, std::string serialized) {
	return static_cast<uint16_t>(std::strtoul(serialized.c_str(), nullptr, 0));
}

std::string getTypeName2(dummyType<uint16_t>) {
	return "unsigned short";
}

// int64_t
std::string serializeElement2(dummyType<int64_t>, int64_t element) {
	return std::to_string(element);
}

uint64_t deserializeElement2(dummyType<int64_t>, std::string serialized) {
	return static_cast<uint64_t>(std::strtoull(serialized.c_str(), nullptr, 0));
}

std::string getTypeName2(dummyType<int64_t>) {
	return "signed long";
}

// int32_t
std::string serializeElement2(dummyType<int32_t>, int32_t element) {
	return std::to_string(element);
}

uint32_t deserializeElement2(dummyType<int32_t>, std::string serialized) {
	return static_cast<int32_t>(std::strtoul(serialized.c_str(), nullptr, 0));
}

std::string getTypeName2(dummyType<int32_t>) {
	return "signed int";
}

// int16_t
std::string serializeElement2(dummyType<int16_t>, int16_t element) {
	return std::to_string(element);
}

uint16_t deserializeElement2(dummyType<int16_t>, std::string serialized) {
	return static_cast<int16_t>(std::strtoul(serialized.c_str(), nullptr, 0));
}

std::string getTypeName2(dummyType<int16_t>) {
	return "signed short";
}

} // namespace pbqppapa
