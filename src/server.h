#pragma once
#include <cstdint>
#include <future>
#include <map>
#include <boost/asio.hpp>
#include <string>
#include "icmdprocessor.h"
#include "basestate.h"
#include "icmdprocessor.h"
#include "clientdatafactory.h"

class Server
{
public:
	Server(uint16_t port);
	void setClientDataFactory(std::unique_ptr<ClientDataFactory> clientDataFactory);
	void start();
	void stop();

protected:
	uint16_t m_port;
	boost::asio::io_context m_ioContext;
	std::future<void> m_serverResult;
	void contextThread();

	std::unique_ptr<boost::asio::ip::tcp::acceptor> m_acceptor;
	void restartAsyncAccept();
	void onAccept(boost::system::error_code ec, std::shared_ptr<boost::asio::ip::tcp::socket> socket);

	std::unique_ptr<ClientDataFactory> m_clientDataFactory;
	std::map<boost::asio::ip::tcp::endpoint, std::unique_ptr<ClientData>> m_clients;

	void startRecv(std::shared_ptr<boost::asio::ip::tcp::socket> client);
	void onRecv(boost::system::error_code ec, size_t len, std::shared_ptr<boost::asio::ip::tcp::socket> socket);
	static const size_t MAX_RECV_BUFFER_SIZE = 1024;
	char m_recvBuffer[1024] = {};

	std::string parseInputString(const std::string& inputString, const std::string &tail, ICmdProcessor* cmdProcessor);
	void processToken(const std::string& token, ICmdProcessor* cmdProcessor);
};