#pragma once

#include <memory>

#include "io/TypeSerializers.hpp"
#include "shell/Command.hpp"

namespace pbqppapa {

template <typename T>
class Command;
template <typename T>
class CommandHandler;
template <typename T>
class PBQP_Serializer;
template <typename T>
class InfinityWrapper;
template <typename T>
class TypeSerializers;

template <typename T>
class FullySolveCommand: public Command<T> {

public:
	FullySolveCommand() : Command<T>("fullysolve") {}

	~FullySolveCommand() = default;

	std::string run(std::string input, CommandHandler<T>* cmdHandler) {
		auto solution = std::unique_ptr<PBQPSolution<InfinityWrapper<T>>>(cmdHandler->getSolver()->solveFully());
		return "Solved fully, total cost: " + serializeElement<InfinityWrapper<T>>(solution->getTotalCost(
													  cmdHandler->getSolver()->getOriginalGraph()));
	}
};

} // namespace pbqppapa
