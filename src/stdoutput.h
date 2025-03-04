#pragma once
#include "ioutput.h"
#include <thread>
#include <mutex>
#include <condition_variable>

/// \brief Outputs a batch to the console
class StdOutput : public IOutput
{
public:
	StdOutput();
	~StdOutput();
	void writeBlock(const std::deque<std::shared_ptr<Command>>& block) override;

protected:
	std::thread m_thread = {};
	std::mutex m_mut = {};
	std::condition_variable m_cv = {};
	std::deque<std::deque<std::shared_ptr<Command>>> m_blocks = {};
	bool m_mustStop = false;
	void threadFunc();
};

