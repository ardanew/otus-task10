#include "test_bulk_server.h"
#include <fstream>
#include <regex>
#include <filesystem>
#include <memory>
#include "cmdprocessor.h"
#include "stdoutput.h"
#include "logoutput.h"

using namespace std;

TEST_F(TestServer, OneClientLimitedState)
{
	testing::internal::CaptureStdout();

	const size_t BULK_SIZE = 3;
	{
		auto stdOutput = make_shared<StdOutput>();
		auto logOutput = make_shared<LogOutput>();
		auto clientDataFactory = make_unique<ClientDataFactory>();
		clientDataFactory->addOutput(stdOutput);
		clientDataFactory->addOutput(logOutput);
		clientDataFactory->init(BULK_SIZE);
		this->setClientDataFactory(std::move(clientDataFactory));
	}

	boost::asio::io_context context;
	auto s = make_shared<boost::asio::ip::tcp::socket>(context);
	boost::asio::ip::tcp::endpoint ep{ boost::asio::ip::address_v4::loopback(), 5555 };
	m_clients[ep] = m_clientDataFactory->createClientData(s);
	auto& clientData = m_clients[ep];

	
	std::string tail;
	tail = this->parseInputString("1\n2\n3\n", "", clientData->cmdProcessor.get());
	tail = this->parseInputString("4\n", tail, clientData->cmdProcessor.get());
	this->stop();

	std::string outStr = testing::internal::GetCapturedStdout();
	EXPECT_EQ(outStr, "bulk: 1, 2, 3\nbulk: 4\n");
}

TEST_F(TestServer, OneClientLimitedStateCmdBreak)
{
	const size_t BULK_SIZE = 3;

	{
		auto stdOutput = make_shared<StdOutput>();
		auto logOutput = make_shared<LogOutput>();
		auto clientDataFactory = make_unique<ClientDataFactory>();
		clientDataFactory->addOutput(stdOutput);
		clientDataFactory->addOutput(logOutput);
		clientDataFactory->init(BULK_SIZE);
		this->setClientDataFactory(std::move(clientDataFactory));
	}

	boost::asio::io_context context;
	auto s = make_shared<boost::asio::ip::tcp::socket>(context);
	boost::asio::ip::tcp::endpoint ep{ boost::asio::ip::address_v4::loopback(), 5555 };
	m_clients[ep] = m_clientDataFactory->createClientData(s);
	auto& clientData = m_clients[ep];

	testing::internal::CaptureStdout();

	std::string tail;
	// NOTE 3 isn't followed by \n immediately
	tail = this->parseInputString("1\n2\n3", "", clientData->cmdProcessor.get());
	tail = this->parseInputString("\n4\n", tail, clientData->cmdProcessor.get());

	this->stop();

	std::string outStr = testing::internal::GetCapturedStdout();
	EXPECT_EQ(outStr, "bulk: 1, 2, 3\nbulk: 4\n");
}

TEST_F(TestServer, OneClientUnLimitedStateSimple)
{
	const size_t BULK_SIZE = 3;

	{
		auto stdOutput = make_shared<StdOutput>();
		auto logOutput = make_shared<LogOutput>();
		auto clientDataFactory = make_unique<ClientDataFactory>();
		clientDataFactory->addOutput(stdOutput);
		clientDataFactory->addOutput(logOutput);
		clientDataFactory->init(BULK_SIZE);
		this->setClientDataFactory(std::move(clientDataFactory));
	}

	boost::asio::io_context context;
	auto s = make_shared<boost::asio::ip::tcp::socket>(context);
	boost::asio::ip::tcp::endpoint ep{ boost::asio::ip::address_v4::loopback(), 5555 };
	m_clients[ep] = m_clientDataFactory->createClientData(s);
	auto& clientData = m_clients[ep];

	testing::internal::CaptureStdout();

	std::string tail;
	tail = this->parseInputString("{\n1\n2\n3\n4\n}\n", "", clientData->cmdProcessor.get());
	this->stop();

	std::string outStr = testing::internal::GetCapturedStdout();
	EXPECT_EQ(outStr, "bulk: 1, 2, 3, 4\n");
}

TEST_F(TestServer, OneClientMixedStates)
{
	const size_t BULK_SIZE = 3;
	{
		auto stdOutput = make_shared<StdOutput>();
		auto logOutput = make_shared<LogOutput>();
		auto clientDataFactory = make_unique<ClientDataFactory>();
		clientDataFactory->addOutput(stdOutput);
		clientDataFactory->addOutput(logOutput);
		clientDataFactory->init(BULK_SIZE);
		this->setClientDataFactory(std::move(clientDataFactory));
	}

	boost::asio::io_context context;
	auto s = make_shared<boost::asio::ip::tcp::socket>(context);
	boost::asio::ip::tcp::endpoint ep{ boost::asio::ip::address_v4::loopback(), 5555 };
	m_clients[ep] = m_clientDataFactory->createClientData(s);
	auto& clientData = m_clients[ep];

	testing::internal::CaptureStdout();

	std::string tail;
	tail = this->parseInputString("1\n2\n{\n3\n4\n}\n{\n5\n6\n", "", clientData->cmdProcessor.get());
	tail = this->parseInputString("{\n7\n8\n}\n", tail, clientData->cmdProcessor.get()); // NOTE this { and } must be ignored (inner bulk)
	tail = this->parseInputString("9\n}\n{\n10\n11\n", tail, clientData->cmdProcessor.get()); // NOTE 10 and 11 must be ignored, no trailing '}'

	this->stop();

	std::string outStr = testing::internal::GetCapturedStdout();
	EXPECT_EQ(outStr, "bulk: 1, 2\nbulk: 3, 4\nbulk: 5, 6, 7, 8, 9\n");
}

TEST_F(TestServer, TwoClientsMixLimitedOutput)
{
	const size_t BULK_SIZE = 3;
	{
		auto stdOutput = make_shared<StdOutput>();
		auto logOutput = make_shared<LogOutput>();
		auto clientDataFactory = make_unique<ClientDataFactory>();
		clientDataFactory->addOutput(stdOutput);
		clientDataFactory->addOutput(logOutput);
		clientDataFactory->init(BULK_SIZE);
		this->setClientDataFactory(std::move(clientDataFactory));
	}

	boost::asio::io_context context;
	auto s1 = make_shared<boost::asio::ip::tcp::socket>(context);
	boost::asio::ip::tcp::endpoint ep1{ boost::asio::ip::address_v4::loopback(), 5555 };
	m_clients[ep1] = m_clientDataFactory->createClientData(s1);
	auto& clientData1 = m_clients[ep1];

	auto s2 = make_shared<boost::asio::ip::tcp::socket>(context);
	boost::asio::ip::tcp::endpoint ep2{ boost::asio::ip::address_v4::loopback(), 6666 };
	m_clients[ep2] = m_clientDataFactory->createClientData(s2);
	auto& clientData2 = m_clients[ep2];


	testing::internal::CaptureStdout();

	std::string tail1;
	tail1 = this->parseInputString("1\n2\n", "", clientData1->cmdProcessor.get()); // from client1
	std::string tail2;
	tail2 = this->parseInputString("3\n", "", clientData2->cmdProcessor.get()); // from client2

	this->stop();

	std::string outStr = testing::internal::GetCapturedStdout();
	EXPECT_EQ(outStr, "bulk: 1, 2, 3\n");
}

TEST_F(TestServer, TwoClientsDontMixUnlimitedMode)
{
	const size_t BULK_SIZE = 3;
	{
		auto stdOutput = make_shared<StdOutput>();
		auto logOutput = make_shared<LogOutput>();
		auto clientDataFactory = make_unique<ClientDataFactory>();
		clientDataFactory->addOutput(stdOutput);
		clientDataFactory->addOutput(logOutput);
		clientDataFactory->init(BULK_SIZE);
		this->setClientDataFactory(std::move(clientDataFactory));
	}

	boost::asio::io_context context;
	auto s1 = make_shared<boost::asio::ip::tcp::socket>(context);
	boost::asio::ip::tcp::endpoint ep1{ boost::asio::ip::address_v4::loopback(), 5555 };
	m_clients[ep1] = m_clientDataFactory->createClientData(s1);
	auto& clientData1 = m_clients[ep1];

	auto s2 = make_shared<boost::asio::ip::tcp::socket>(context);
	boost::asio::ip::tcp::endpoint ep2{ boost::asio::ip::address_v4::loopback(), 6666 };
	m_clients[ep2] = m_clientDataFactory->createClientData(s2);
	auto& clientData2 = m_clients[ep2];


	testing::internal::CaptureStdout();

	std::string tail1;
	tail1 = this->parseInputString("1\n2\n", "", clientData1->cmdProcessor.get()); // from client1
	std::string tail2;
	tail2 = this->parseInputString("{\n11\n", "", clientData2->cmdProcessor.get()); // from client2 - starts unlimited mode

	tail1 = this->parseInputString("3\n4\n", tail1, clientData1->cmdProcessor.get()); // from client1 - must not break client2 unlimited mode
	tail2 = this->parseInputString("22\n33\n44\n}\n", tail2, clientData2->cmdProcessor.get()); // from client2 - end limited state

	this->stop();

	std::string outStr = testing::internal::GetCapturedStdout();
	EXPECT_NE(string::npos, outStr.find("bulk: 11, 22, 33, 44\n"));
}


void deleteAllLogs()
{
	namespace fs = std::filesystem;
	fs::path dir = fs::current_path();
	regex regExpName{ "(.*\\.log)" };
	for (const auto& element : fs::directory_iterator(dir))
		if (!element.is_directory() && regex_match(element.path().filename().string(), regExpName))
			fs::remove(element.path());
}

string readFile(const string& fname)
{
	string res;
	ifstream file{ fname };
	string line;
	while (getline(file, line))
		res += line;
	file.close();
	return res;
}

string readAllLogs()
{
	string res;
	namespace fs = std::filesystem;
	fs::path dir = fs::current_path();
	regex regExpName{ "(.*\\.log)" };
	for (const auto& element : fs::directory_iterator(dir))
		if (!element.is_directory() && regex_match(element.path().filename().string(), regExpName))
			res += readFile(element.path().string());
	return res;
}

TEST_F(TestServer, OneClientUnLimitedStateTask1) // test1 from readme
{
	deleteAllLogs();

	const size_t BULK_SIZE = 3;
	{
		auto stdOutput = make_shared<StdOutput>();
		auto logOutput = make_shared<LogOutput>();
		auto clientDataFactory = make_unique<ClientDataFactory>();
		clientDataFactory->addOutput(stdOutput);
		clientDataFactory->addOutput(logOutput);
		clientDataFactory->init(BULK_SIZE);
		this->setClientDataFactory(std::move(clientDataFactory));
	}

	boost::asio::io_context context;
	auto s = make_shared<boost::asio::ip::tcp::socket>(context);
	boost::asio::ip::tcp::endpoint ep{ boost::asio::ip::address_v4::loopback(), 5555 };
	m_clients[ep] = m_clientDataFactory->createClientData(s);
	auto& clientData = m_clients[ep];

	testing::internal::CaptureStdout();

	std::string tail;
	tail = this->parseInputString("0\n1\n2\n", "", clientData->cmdProcessor.get());
	this_thread::sleep_for(1s); // time_t resolution is too low
	tail = this->parseInputString("3\n4\n5\n", tail, clientData->cmdProcessor.get());
	this_thread::sleep_for(1s);
	tail = this->parseInputString("6\n7\n8\n", tail, clientData->cmdProcessor.get());
	this_thread::sleep_for(1s);
	tail = this->parseInputString("9\n", tail, clientData->cmdProcessor.get());
	this_thread::sleep_for(1s);
	this->stop();

	std::string outStr = testing::internal::GetCapturedStdout();
	EXPECT_EQ(outStr, "bulk: 0, 1, 2\nbulk: 3, 4, 5\nbulk: 6, 7, 8\nbulk: 9\n");

	string textFromLogFiles = readAllLogs();
	for (int i = 0; i < 10; i++)
		EXPECT_NE(string::npos, textFromLogFiles.find(to_string(i)));
}


TEST_F(TestServer, TestClientDisconnect)
{
	const size_t BULK_SIZE = 3;
	{
		auto stdOutput = make_shared<StdOutput>();
		auto logOutput = make_shared<LogOutput>();
		auto clientDataFactory = make_unique<ClientDataFactory>();
		clientDataFactory->addOutput(stdOutput);
		clientDataFactory->addOutput(logOutput);
		clientDataFactory->init(BULK_SIZE);
		this->setClientDataFactory(std::move(clientDataFactory));
	}

	this->start();

	boost::asio::io_context context;
	//auto s = make_shared<boost::asio::ip::tcp::socket>(context);
	auto client = make_shared<boost::asio::ip::tcp::socket>(context);
	boost::asio::ip::tcp::endpoint ep{ boost::asio::ip::address_v4::loopback(), 5555 };

	bool stop = false;
	std::thread t([&] {
		while(!stop)
			context.run();
	});

	boost::system::error_code connect_err;
	for (int i = 0; i < 5; i++)
	{
		client->connect(ep, connect_err);
		if (!connect_err)
			break; // connected
		this_thread::sleep_for(10ms); // retry
	}
	EXPECT_FALSE(connect_err);
	this_thread::sleep_for(100ms);

	auto& clientData = m_clients.begin()->second;

	testing::internal::CaptureStdout();

	std::string tail;
	tail = this->parseInputString("0\n1\n", "", clientData->cmdProcessor.get());

	client->close(connect_err);
	EXPECT_FALSE(connect_err);
	this_thread::sleep_for(100ms);

	std::string outStr = testing::internal::GetCapturedStdout();
	EXPECT_EQ("bulk: 0, 1\n", outStr);

	this->stop();
	stop = true;
	t.join();
}