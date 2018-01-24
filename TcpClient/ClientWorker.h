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
#include "Message.h"
#include "SharedFunctions.h"

const char DELIM = ':';

static bool dr = false;
static std::string uname;
static int mesId = -1;

class ClientWorker {
public:
    ClientWorker();

    ~ClientWorker();

    void StartThread(std::string &params);

    static void HandleThread(std::string &args);

    //void SendTo(SOCKET s, const string& message);
    //bool ListenRecv(SOCKET s, std::string& MsgStr);
    void SendTo(int s, std::string &message);

    bool ListenRecv(int s, std::vector<char> &message);

    std::string Serialize(STATE opcode, std::vector<std::string> &args);

    STATE Parse(std::vector<char> &input, std::vector<std::string> &args);

    std::string MessageToString(const Message &m);

    void ProcessRes(short &state, std::vector<char> &buf, Message &m, const std::string &mes);

private:
    void Run(std::string host, unsigned short port);

    void ListenLoop(const int &socket);

    STATE ParseOpCode(const std::string &buf);
};
