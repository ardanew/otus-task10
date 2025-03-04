#include "ioutput.h"
#include <thread>
#include <mutex>
#include <string>
#include <condition_variable>

/// \brief Outputs commands batch to a log file
class LogOutput : public IOutput
{
public:
	LogOutput();
	~LogOutput();
	void writeBlock(const std::deque<std::shared_ptr<Command>>& block) override;

protected:
	std::thread m_thread1 = {};
	std::thread m_thread2 = {};
	std::mutex m_mut = {};
	std::condition_variable m_cv = {};
	std::deque<std::deque<std::shared_ptr<Command>>> m_blocks = {};
	bool m_mustStop = false;
	void threadFunc(const std::string &suffix);
	std::string makeFileName(std::time_t t, const std::string& suffix) const;
};