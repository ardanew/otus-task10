#pragma once
#include <memory>
#include <vector>
#include <deque>
#include "clientdata.h"
#include "basestate.h"
#include "ioutput.h"

class ClientDataFactory
{
public:
	void addOutput(std::shared_ptr<IOutput> output);
	void init(size_t bulk_size);
	std::unique_ptr<ClientData> createClientData(std::shared_ptr<boost::asio::ip::tcp::socket> socket);

protected:
	std::shared_ptr<BaseState> m_limitedState;
	std::deque<std::shared_ptr<IOutput>> m_outputs;
};