#ifndef SHELL_COMMANDHANDLER_HPP_
#define SHELL_COMMANDHANDLER_HPP_

#include <iostream>
#include <map>
#include <string>

#include "math/InfinityWrapper.hpp"
#include "shell/CheckSolvableCommand.hpp"
#include "shell/Command.hpp"
#include "shell/DumpCommand.hpp"
#include "shell/FullySolveCommand.hpp"
#include "shell/InfoCommand.hpp"
#include "shell/LoadCommand.hpp"
#include "shell/StepBackwardCommand.hpp"
#include "shell/StepForwardCommand.hpp"
#include "shell/StopCommand.hpp"
#include "shell/VisualizeCommand.hpp"

namespace pbqppapa {

template <typename T>
class Command;
template <typename T>
class StepByStepSolver;
template <typename T>
class DumpCommand;
template <typename T>
class StepForwardCommand;
template <typename T>
class StepBackwardCommand;
template <typename T>
class InfoCommand;
template <typename T>
class LoadCommand;
template <typename T>
class StopCommand;
template <typename T>
class FullySolveCommand;
template <typename T>
class VisualizeCommand;

/**
 * Used for step by step debugging this is command handler will take text input and call the method associated
 * with the given input if one is known
 */
template <typename T>
class CommandHandler {
private:
	std::map<std::string, Command<T>*> commands;
	std::string error = "Unknown command: ";
	std::unique_ptr<StepByStepSolver<T>> solver;

public:
	CommandHandler(PBQPGraph<InfinityWrapper<T>>* graph) : solver(std::make_unique<StepByStepSolver<T>>(graph)) {
		registerCommand(new DumpCommand<T>());
		registerCommand(new StepForwardCommand<T>());
		registerCommand(new StepBackwardCommand<T>());
		registerCommand(new InfoCommand<T>());
		registerCommand(new LoadCommand<T>());
		registerCommand(new StopCommand<T>());
		registerCommand(new FullySolveCommand<T>());
		registerCommand(new CheckSolvableCommand<T>());
		registerCommand(new VisualizeCommand<T>());
	}

	std::string execute(std::string input) {
		size_t spaceIndex = input.find(" ");
		std::string commandString;
		std::string commandArgs;
		if (spaceIndex == input.npos) {
			// not found, so take the entire string
			commandString = input;
		} else {
			commandString = input.substr(0, spaceIndex);
			commandArgs = input.substr(spaceIndex + 1);
		}
		auto iter = commands.find(commandString);
		if (iter == commands.end()) {
			// command doesnt exist
			return error + input;
		}
		Command<T>* command = iter->second;
		return command->run(commandArgs, this);
	}

	void registerCommand(Command<T>* command) {
		commands.insert({command->getIdentifier(), command});
	}

	[[nodiscard]] StepByStepSolver<T>* getSolver() {
		return solver.get();
	}

	void setGraph(PBQPGraph<InfinityWrapper<T>>* graph) {
		StepByStepSolver<T>* newSolver = new StepByStepSolver<T>(graph);
		solver.reset(newSolver);
	}
};

} // namespace pbqppapa

#endif /* SHELL_COMMANDHANDLER_HPP_ */
