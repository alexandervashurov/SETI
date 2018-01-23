#pragma once

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <vector>
#include <cstdint>
#include <unistd.h>
#include <string>
#include <sstream> 
#include <algorithm>

#include "API.h"

const char DELIM = ':';
const short BUFSIZE = 10;
const short STRING_BUFFER_SIZE = 1024;

static bool dr = false;
static string uname;
static int mesId = -1;

using namespace std;

class ClientWorker
{
public:
	ClientWorker();
	~ClientWorker();

	void StartThread(string &params);

    static void HandleThread(std::string& args);
	//void SendTo(SOCKET s, const string& message);
	//bool ListenRecv(SOCKET s, std::string& MsgStr);
	void SendTo(int s, std::string& message);
	bool ListenRecv(int s, vector<char> &message);
	string Serialize(STATE opcode, std::vector<std::string>& args);
	STATE Parse(vector<char> &input, unsigned short &numarg, string *&args);
	string MessageToString(const Message& m);
	void ProcessRes(short &state, vector<char> &buf, Message &m, const string &mes);
private:
	void Run(string host, unsigned short port);
	void ListenLoop(const int& socket);
	STATE ParseOpCode(const string& buf);
};
