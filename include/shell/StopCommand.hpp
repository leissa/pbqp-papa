#pragma once

#include <string>

#include "shell/Command.hpp"

namespace pbqppapa {

template <typename T>
class Command;
template <typename T>
class PBQPGraph;
template <typename T>
class CommandHandler;

template <typename T>
class StopCommand: public Command<T> {

public:
	StopCommand() : Command<T>("stop") {}

	~StopCommand() = default;

	std::string run(std::string input, CommandHandler<T>* cmdHandler) {
		std::exit(0);
		return "";
	}
};

} // namespace pbqppapa
