#pragma once

#include "stdinc.h"
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <memory>
#include <unordered_map>
#include <ctime>
#include <mutex>
#include "ServerWorker.h"

typedef unsigned short USHORT;
typedef int ClientID;

using namespace std;

class CTcpServer;


struct AcceptThInput {
    AcceptThInput() {
        ThPort = 0;
        pParent = 0;
        pAcceptSock = 0;
    };

    USHORT ThPort;
    SOCKET pAcceptSock;
    CTcpServer *pParent;
};

struct ListenThInput {
    ListenThInput() {
        ClientSocket = 0;
        pParent = 0;
        CliID = 0;
        State = 1;
        CurQst = 1;
        result = 0;
    };

    SOCKET ClientSocket;
    ClientID CliID;
    CTcpServer *pParent;
    int State;
    int CurQst;
    int result;
};


struct ThreadInfo {
    ThreadInfo() {
        ThHandle = 0;
        ThPort = 0;
        bCreated = false;
    };

    std::shared_ptr<std::thread> ThHandle;
    USHORT ThPort;
    bool bCreated;
};

struct ClientInfo {
    ClientInfo() {
        ID = 0;
        ClientSocket = 0;
    };

    ClientID ID;
    SOCKET ClientSocket;
    ThreadInfo ClientThreadInfo;
};

bool ListenProc(ListenThInput &pData, int &State, int &CurQst, int &result);

bool ListenRecv(ClientID &From, std::string &MsgStr, ListenThInput &pData);

class CTcpServer {
public:
    CTcpServer();

    CTcpServer(const CTcpServer &orig);

    virtual ~CTcpServer();

    void StartAccept(USHORT Port);

    void StartListenTh(SOCKET Sock);

    void set_socket(SOCKET sock) {
        AcceptSock = sock;
    }

    std::vector<ClientID> ListClients();
    void killClient(ClientID id);
    void shutdown();

    void HandleDisconnection(ClientID id);
    void RemoveDisconnected();

	void LockClient(const std::string& name);
	void UnlockClient(const std::string& name);

private:
    ThreadInfo AcceptThInfo;
    ClientID LastClientID;
    SOCKET AcceptSock;
    std::mutex Mut;
    std::unordered_map<ClientID, ClientInfo> clients;
    std::vector<ClientInfo> disconnected_clients;
	std::unordered_map<std::string, std::mutex> locks;
};