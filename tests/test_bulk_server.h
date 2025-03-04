#pragma once
#include <gtest/gtest.h>
#include "server.h"

struct TestServer : public ::testing::Test, public Server
{
	TestServer() : Server(5555) {}
};