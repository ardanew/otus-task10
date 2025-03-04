#include "unlimitedstate.h"

UnlimitedState::UnlimitedState(std::deque<std::shared_ptr<IOutput>>& outputs) : BaseState(outputs) {}

bool UnlimitedState::startBlock()
{
	m_starts++;
	return false;
}

bool UnlimitedState::endBlock()
{
	m_starts--;
	if (m_starts == 0)
	{
		m_starts = 1; // for next time
		flush();
		return true;
	}
	return false;
}

void UnlimitedState::eof()
{
}

void UnlimitedState::process(std::shared_ptr<Command>&& cmd)
{
	m_commands.push_back(std::move(cmd));
}
