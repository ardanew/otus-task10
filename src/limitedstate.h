#pragma once
#include "basestate.h"

/// \brief This state will produce a batch when maxCmds reached
/// \details Also produces a batch on eof and on startBlock
class LimitedState : public BaseState
{
public:
	LimitedState(std::deque<std::shared_ptr<IOutput>>& outputs, size_t maxCmds);

	bool startBlock() override;
	bool endBlock() override;
	void eof() override;
	void process(std::shared_ptr<Command>&& cmd) override;

protected:
	size_t m_maxCmds;
};
