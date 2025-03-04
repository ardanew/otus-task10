#include "server.h"
#include <exception>
#include <iostream>
#include <sstream>
#include <chrono>
#include "limitedstate.h"
#include "cmdprocessor.h"
using namespace std;
namespace bs = boost::system;
namespace ba = boost::asio;
using namespace std::placeholders;

Server::Server(uint16_t port) : m_port(port) 
{
}

void Server::setClientDataFactory(std::unique_ptr<ClientDataFactory> clientDataFactory)
{
	m_clientDataFactory = std::move(clientDataFactory);
}

void Server::start()
{
	using namespace boost::asio::ip;
	tcp::endpoint ep{ tcp::v4(), m_port };
	m_acceptor = make_unique<tcp::acceptor>(m_ioContext, ep);

	restartAsyncAccept();

	m_serverResult = std::async(std::launch::async, &Server::contextThread, this);
}

void Server::restartAsyncAccept()
{
	std::shared_ptr<ba::ip::tcp::socket> socket = std::make_shared<ba::ip::tcp::socket>(m_ioContext);
	m_acceptor->async_accept(*socket, std::bind(&Server::onAccept, this, _1, socket));
}

void Server::stop()
{
	for (auto& kvp : m_clients)
	{
		auto& clientData = kvp.second;
		bs::error_code ec;
		clientData->socket->close(ec);
		clientData->cmdProcessor->eof();
	}

	if (m_acceptor)
	{
		try {
			m_acceptor->cancel();
			m_ioContext.stop();
			m_serverResult.get();
		}
		catch (exception& e) {
			cerr << "Exception: " << e.what() << endl;
		}
		m_acceptor.reset();
	}

	m_clients.clear();
	m_clientDataFactory.reset();
}

void Server::contextThread()
{
	m_ioContext.run();
}

void Server::onAccept(bs::error_code ec, shared_ptr<ba::ip::tcp::socket> socket)
{
	if (ec)
	{
		cerr << "onAccept: " << ec.message() << endl;
		return;
	}

	//cout << "connection from " << socket->remote_endpoint().address() << ":" << socket->remote_endpoint().port() << endl; 

 	auto ep = socket->remote_endpoint();
 	auto clientData = m_clientDataFactory->createClientData(socket); // тут важно сохранить куданть клиентский сокет, а то разорвёт соединение сразу
 	m_clients.emplace(ep, std::move(clientData));
 
	startRecv(socket);

 	restartAsyncAccept();
}

void Server::startRecv(std::shared_ptr<boost::asio::ip::tcp::socket> client)
{
 	client->async_read_some(ba::buffer(m_recvBuffer, std::size(m_recvBuffer)), 
 		std::bind(&Server::onRecv, this, _1, _2, client));
}

void Server::onRecv(boost::system::error_code ec, size_t len, shared_ptr<boost::asio::ip::tcp::socket> socket)
{
	if (ec)
	{
		//cerr << "onRecv: " << ec.what() << endl;
		if (auto it = m_clients.find(socket->remote_endpoint()); it != m_clients.end())
		{
			unique_ptr<ClientData>& cd = it->second;
			// тут можно не делать cd->cmdProcessor->eof(); при отключении
			// если клиент был в limited состоянии (когда ограничены команды размером bulk_size) то эти команды сохранятся
			// если был в unlimited состоянии (когда в фигурных скобках) - то по условиям задания их и не надо выводить
			bs::error_code ec;
			cd->socket->close(ec);
			m_clients.erase(it);
		}
		return;
	}

	std::string s{ m_recvBuffer, len };
	std::string tail;
	unique_ptr<ClientData>& clientData = m_clients.find(socket->remote_endpoint())->second;
	tail = clientData->tail;
	std::string newtail = parseInputString(s, tail, clientData->cmdProcessor.get());
	clientData->tail = std::move(newtail);
	startRecv(socket);
}

string Server::parseInputString(const string &inputString, const string &tail, ICmdProcessor *cmdProcessor)
{
	stringstream ss;
	ss << tail;
	ss << inputString;
	
	// в стандартной библиотеке тяжело токенизировать сохраняя \n
	// а хочется отлавливать случай когда по сетке прилетела команда порезанная пополам
	// т.е. хвост и \n от неё прилетят в следующем пакете
	// например первый пакет "2\n,3\n,4" второй пакет "4\n" - должно в итоге получится "2, 3, 44"
	string s;
	std::getline(ss, s, '\n');

	// поэтому сначала пытаемся считать следующий токен а потом обрабатываем предыдущий
	while(true) 
	{ 
		string last;
		swap(s, last); // сохраним предыдущий токен

		if (!std::getline(ss, s, '\n'))
		{
			swap(s, last); // вернём обратно
			break;
		}

		// обработка предыдущего токена
		processToken(last, cmdProcessor);
	}

	if (*inputString.rbegin() == '\n')
	{ // специальная обработка для случая если нет хвоста
		processToken(s, cmdProcessor);
		return ""; // no tail
	}
	return s; // сохраняем хвост чтоб приделать его в начало данных на следующей итерации
}

void Server::processToken(const std::string& token, ICmdProcessor* cmdProcessor)
{
	if (token.empty())
		return;
	else if (token == "{")
		cmdProcessor->startBlock();
	else if (token == "}")
		cmdProcessor->endBlock();
	else
	{
		auto cmd = make_unique<Command>();
		auto now = std::chrono::system_clock::now();
		cmd->m_name = token;
		cmd->m_time = std::chrono::system_clock::to_time_t(now);
		cmdProcessor->process(std::move(cmd));
	}
}