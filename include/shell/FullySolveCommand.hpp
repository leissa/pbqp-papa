#ifndef SHELL_FULLYSOLVECOMMAND_HPP_
#define SHELL_FULLYSOLVECOMMAND_HPP_

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

	~FullySolveCommand() {}

	std::string run(std::string input, CommandHandler<T>* cmdHandler) {
		PBQPSolution<InfinityWrapper<T>>* solution = cmdHandler->getSolver()->solveFully();
		return "Solved fully, total cost: " + serializeElement<InfinityWrapper<T>>(solution->getTotalCost(
													  cmdHandler->getSolver()->getOriginalGraph()));
	}
};

} // namespace pbqppapa

#endif /* SHELL_FULLYSOLVECOMMAND_HPP_ */
