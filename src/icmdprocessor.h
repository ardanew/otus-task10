#pragma once
#include <deque>
#include <memory>
#include "command.h"
#include "ioutput.h"
#include "basestate.h"

/// \brief Interface for class that produces batches of commands
class ICmdProcessor
{
public:
	virtual ~ICmdProcessor() = default;
	/// \brief Called on '{' input symbol
	virtual void startBlock() = 0;
	/// \brief Called on '}' output symbol
	virtual void endBlock() = 0;
	/// \brief Called when input ends
	virtual void eof() = 0;
	/// \brief Called on each command
	virtual void process(std::unique_ptr<Command> cmd) = 0;
	// \brief Initializes shared state (the one for all clients)
	virtual void setLimitedState(std::shared_ptr<BaseState> limitedState) = 0;
	/// \brief Adds classes to handle batch command lists
	virtual void addOutput(std::shared_ptr<IOutput> output) = 0;
};