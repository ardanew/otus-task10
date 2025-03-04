#include "stdoutput.h"
#include <iostream>
using namespace std;

StdOutput::StdOutput() 
{
	m_thread = std::thread(&StdOutput::threadFunc, this);
}

StdOutput::~StdOutput() 
{
	{
		lock_guard l(m_mut);
		m_mustStop = true;
		m_cv.notify_one();
	}
	m_thread.join();
}

void StdOutput::writeBlock(const std::deque<std::shared_ptr<Command>>& block)
{
	if (block.empty())
		return;

	std::deque<std::shared_ptr<Command>> blockToPush;
	for (auto& cmd : block)
		blockToPush.push_back(cmd);

	{
		lock_guard l(m_mut);
		m_blocks.push_back(blockToPush);
		m_cv.notify_one();
	}
}

void StdOutput::threadFunc()
{
	while (true)
	{
		deque<deque<shared_ptr<Command>>> blocks;
		bool mustStop;
		{
			unique_lock l(m_mut);
			m_cv.wait(l, [&]() { return m_mustStop || !m_blocks.empty(); });
			mustStop = m_mustStop;
			std::swap(blocks, m_blocks);
		}

		if (!blocks.empty())
		{
			for (auto& block : blocks)
			{
				cout << "bulk: ";
				for (size_t i = 0; i < block.size(); i++)
				{
					cout << block[i]->m_name;
					if (i != block.size() - 1)
						cout << ", ";
				}
				cout << "\n";
			}
		}

		if (mustStop)
			break;
	}
}