#pragma once
#include <string>
#include <ctime>

/// \brief Command with timestamp
/// \details There are only one type of commands => no ID field
struct Command
{
	std::time_t m_time;
	std::string m_name;
};