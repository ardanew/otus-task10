#pragma once
#include "basestate.h"

/// \brief This state will output a batch of commands on block end
/// \details Ignores inner start/end blocks
class UnlimitedState : public BaseState
{
public:
	UnlimitedState(std::deque<std::shared_ptr<IOutput>>& outputs);

	bool startBlock() override;
	bool endBlock() override;
	void eof() override;
	void process(std::shared_ptr<Command>&& cmd) override;

protected:
	/// \brief At state start there are no startBlock, so use 1 as block start counters
	int m_starts = 1;
};
