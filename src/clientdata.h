#pragma once
#include <boost/asio.hpp>
#include <string>
#include <memory>
#include "icmdprocessor.h"

struct ClientData
{
	std::shared_ptr<boost::asio::ip::tcp::socket> socket;
	std::string tail;
	std::unique_ptr<ICmdProcessor> cmdProcessor;
};