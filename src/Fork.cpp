/*
** EPITECH PROJECT, 2018
** cpp_plazza
** File description:
** Created by Benjamin
*/

#include <iostream>
#include <zconf.h>
#include <chrono>
#include <sys/wait.h>
#include <unordered_map>
#include <cstring>
#include <arpa/inet.h>
#include "Fork.hpp"

Plazza::Fork::Fork(struct sockaddr master, size_t maxThreads) : _master(master),
	_maxThread(maxThreads), _socket(true)
{
}

Plazza::Fork::~Fork()
{
	if (_forkPid) {
		exitThreads();
		waitpid(_forkPid, nullptr, 0);
	}
};

void Plazza::Fork::proceedFork()
{
	_forkPid = fork();
	if (!_forkPid) {
		_threadPool = std::unique_ptr<Plazza::ThreadPool>(
			new Plazza::ThreadPool(_master, _maxThread));
		awaitCmd();
		exit(EXIT_SUCCESS);
	}
}

void Plazza::Fork::exitThreads()
{
	auto srv = dynamic_cast<ServerSocket *>(_socket.getServer());
	std::string exitCmd = "0";

	exitCmd[0] += Plazza::EXIT;
	srv->send(exitCmd);
}

void Plazza::Fork::proceedCmd(std::string filePath, ::Plazza::dataTypes dT)
{
	std::string temp;
	auto srv = dynamic_cast<ServerSocket *>(_socket.getServer());

	srv->send("1");
	srv->send(filePath + std::to_string(dT));
}

size_t Plazza::Fork::getWorkLoad()
{
	std::string workLoad;
	auto srv = dynamic_cast<ServerSocket *>(_socket.getServer());

	srv->send("0");
	srv->receive(workLoad);
	return std::stoul(workLoad);
}

void Plazza::Fork::clientCmdProceed(std::string &cmd)
{
	std::string buf;
	size_t idx;
	auto client = dynamic_cast<ClientSocket *>(_socket.getClient());

	if (!(cmd[0] - '0'))
		client->send(std::to_string(_threadPool->getWorkLoad()));
	else {
		client->receive(buf);
		idx = static_cast<size_t>(buf.back() - '0');
		buf.pop_back();
		_threadPool->assignTask(buf, idx);
	}
}

void Plazza::Fork::awaitCmd()
{
	std::string cmd;
	auto client = dynamic_cast<ClientSocket *>(_socket.getClient());

	while (true) {
		client->receive(cmd);
		if (cmd.size() == 1 && (cmd[0] - '0' == Plazza::EXIT))
			break;
		clientCmdProceed(cmd);
	}
	_threadPool->awaitThreads();
}

int Plazza::Fork::getPid() const
{
	return _forkPid;
}