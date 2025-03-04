#pragma once
#include <deque>
#include <mutex>
#include "icmdprocessor.h"
#include "basestate.h"

/// \brief Process commands
/// \details Uses pattern 'state', changes state on start/end blocks
class CmdProcessor : public ICmdProcessor
{
public:
	CmdProcessor();
	void setLimitedState(std::shared_ptr<BaseState> limitedState) override;
	void startBlock() override;
	void endBlock() override;
	void eof() override;
	void process(std::unique_ptr<Command> cmd) override;
	void addOutput(std::shared_ptr<IOutput> output) override;

protected:
	std::deque<std::shared_ptr<IOutput>> m_outputs;

	std::shared_ptr<BaseState> m_currentState;
	std::shared_ptr<BaseState> m_otherState;
};