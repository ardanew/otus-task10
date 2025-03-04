#include "builder.h"
#include "cmdprocessor.h"
#include "stdoutput.h"
#include "logoutput.h"
#include "clientdatafactory.h"

std::unique_ptr<Server> Builder::build(uint16_t port, int bulk_size)
{
	using namespace std;
	
	auto stdOutput = make_shared<StdOutput>();
	auto logOutput = make_shared<LogOutput>();

	auto clientDataFactory = make_unique<ClientDataFactory>();
	clientDataFactory->addOutput(stdOutput);
	clientDataFactory->addOutput(logOutput);
	clientDataFactory->init(bulk_size);

	auto server = make_unique<Server>(port);
	server->setClientDataFactory(std::move(clientDataFactory));

	return server;
}
