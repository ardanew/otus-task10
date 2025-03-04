#include "clientdatafactory.h"
#include "cmdprocessor.h"
#include "limitedstate.h"

using namespace std;

void ClientDataFactory::addOutput(std::shared_ptr<IOutput> output)
{
	m_outputs.push_back(std::move(output));
}

void ClientDataFactory::init(size_t bulk_size)
{
	m_limitedState = make_shared<LimitedState>(m_outputs, bulk_size);
}

unique_ptr<ClientData> ClientDataFactory::createClientData(std::shared_ptr<boost::asio::ip::tcp::socket> socket)
{
	std::unique_ptr<ClientData> cd = make_unique<ClientData>();

	cd->socket = socket;
	cd->cmdProcessor = make_unique<CmdProcessor>();
	for (std::shared_ptr<IOutput>& output : m_outputs)
		cd->cmdProcessor->addOutput(output);
	cd->cmdProcessor->setLimitedState(m_limitedState);

	return std::move(cd);
}