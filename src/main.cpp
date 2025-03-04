#include <string>
#include <iostream>
#include <cstdint>
#include <memory>
#include "server.h"
#include "builder.h"
using namespace std;

void printHelp()
{
	cout << "Usage: bulk_server <port> <bulk_size>" << endl;
	cout << "\tport - tcp port to use" << endl;
	cout << "\tbulk_size - max commands packet size" << endl;
	cout << "\tsee README.md for details" << endl;
}

int main(int argc, char** argv)
{
	std::locale::global(std::locale(""));
	if (argc <= 2 || string(argv[1]) == "--help")
	{
		printHelp();
		return 0;
	}

	uint16_t port = stoi(string(argv[1]));
	int bulk_size = stoi(string(argv[2]));

	unique_ptr<Server> server = Builder::build(port, bulk_size); // using builder pattern
	server->start();

	string s; // any input stops the server
	cin >> s;

	server->stop();

    return 0;
}