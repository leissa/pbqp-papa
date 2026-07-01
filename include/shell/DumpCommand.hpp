#pragma once

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
class DumpCommand: public Command<T> {

public:
	DumpCommand() : Command<T>("dump") {}

	~DumpCommand() = default;

	std::string run(std::string input, CommandHandler<T>* cmdHandler) {
		// TODO check/sanitize path
		PBQP_Serializer<InfinityWrapper<T>> serial;
		serial.saveToFile(input, cmdHandler->getSolver()->getGraph());
		return "Saved graph json to " + input;
	}
};

} // namespace pbqppapa
