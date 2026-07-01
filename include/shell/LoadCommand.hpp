#pragma once

#include <string>

#include "shell/Command.hpp"

namespace pbqppapa {

template <typename T>
class Command;
template <typename T>
class CommandHandler;
template <typename T>
class PBQP_Serializer;

template <typename T>
class LoadCommand: public Command<T> {

public:
	LoadCommand() : Command<T>("load") {}

	~LoadCommand() = default;

	std::string run(std::string input, CommandHandler<T>* cmdHandler) override {
		PBQP_Serializer<InfinityWrapper<T>> serial;
		PBQPGraph<InfinityWrapper<T>>* graph = serial.loadFromFile(input);
		cmdHandler->setGraph(graph);
		return "Loaded " + input;
	}
};

} // namespace pbqppapa
