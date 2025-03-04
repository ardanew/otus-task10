#include "logoutput.h"
#include <fstream>
using namespace std;

LogOutput::LogOutput()
{
	m_thread1 = std::thread(&LogOutput::threadFunc, this, "_t1");
	m_thread2 = std::thread(&LogOutput::threadFunc, this, "_t2");
}

LogOutput::~LogOutput()
{
	{
		lock_guard l(m_mut);
		m_mustStop = true;
		m_cv.notify_all();
	}
	m_thread1.join();
	m_thread2.join();
}

void LogOutput::writeBlock(const std::deque<std::shared_ptr<Command>>& block)
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

std::string LogOutput::makeFileName(std::time_t t, const std::string &suffix) const
{
	return std::string("bulk") + std::to_string(t) + suffix + ".log";
}

void LogOutput::threadFunc(const std::string &suffix)
{
	while (true)
	{
		deque<shared_ptr<Command>> block;
		{
			unique_lock l(m_mut);
			m_cv.wait(l, [&]() { return m_mustStop || !m_blocks.empty(); });
			if (m_mustStop && m_blocks.empty())
				break;

			block = m_blocks.front();
			m_blocks.pop_front();
		}

		std::time_t first = block[0]->m_time;
		std::string fname = makeFileName(first, suffix);

		std::ofstream ofs(fname, ios::app | ios::out);

		ofs << "bulk: ";
		for (size_t i = 0; i < block.size(); i++)
		{
			ofs << block[i]->m_name;
			if (i != block.size() - 1)
				ofs << ", ";
		}
		ofs << "\n";

		ofs.flush();
		ofs.close();
	}
}