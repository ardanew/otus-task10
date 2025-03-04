#pragma once
#include <deque>
#include <memory>
#include "command.h"

/// \brief Interface for outputs
struct IOutput
{
	virtual ~IOutput() = default;

	virtual void writeBlock(const std::deque<std::shared_ptr<Command>> &block) = 0;
};