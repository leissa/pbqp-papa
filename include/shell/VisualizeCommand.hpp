#ifndef SHELL_VISUALIZECOMMAND_HPP_
#define SHELL_VISUALIZECOMMAND_HPP_

#include <string>

#include "debug/GraphVisualizer.hpp"
#include "shell/Command.hpp"

namespace pbqppapa {

template <typename T>
class Command;
template <typename T>
class PBQPGraph;
template <typename T>
class CommandHandler;

template <typename T>
class VisualizeCommand: public Command<T> {

public:
	VisualizeCommand() : Command<T>("visualize") {}

	~VisualizeCommand() {}

	std::string run(std::string input, CommandHandler<T>* cmdHandler) {
		dump(cmdHandler->getSolver()->getGraph(), input);
		return "Dumped to " + input;
	}
};

} // namespace pbqppapa

#endif /* SHELL_VISUALIZECOMMAND_HPP_ */
