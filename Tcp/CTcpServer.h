#pragma once

#include "stdinc.h"
#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <ctime>
#include <mutex>
#include "ServerWorker.h"

typedef unsigned short  USHORT;
typedef int             ClientID;

using namespace std;
class CTcpServer;


struct AcceptThInput
{
    AcceptThInput()
    {
        ThPort=0;
        pParent=0;
        pAcceptSock=0;
    };

    USHORT ThPort;
    SOCKET* pAcceptSock;
    CTcpServer* pParent;
};

struct ListenThInput
{
    ListenThInput()
    {
        ClientSocket=0;
        pParent=0;
        CliID=0;
        State=1;
        CurQst=1;
        result=0;
    };

    SOCKET ClientSocket;
    ClientID CliID;
    CTcpServer* pParent;
    int State;
    int CurQst;
    int result;
};


struct ThreadInfo
{
    ThreadInfo()
    {
        ThHandle=0;
        ThPort=0;
        bCreated=false;
    };

    std::shared_ptr<std::thread> ThHandle;
    USHORT ThPort;
    bool bCreated;
};

struct ClientInfo
{
    ClientInfo()
    {
        ID=0;
        ClientSocket=0;
    };

    ClientID ID;
    SOCKET ClientSocket;
    ThreadInfo ClientThreadInfo;
};

bool ListenProc(ListenThInput& pData,int& State,int& CurQst,int& result);
bool ListenRecv(ClientID& From, std::string& MsgStr,ListenThInput& pData);

class CTcpServer {
public:
    CTcpServer();
    CTcpServer(const CTcpServer& orig);
    virtual ~CTcpServer();
    void StartAccept(USHORT Port);
    void StartListenTh(SOCKET Sock);
private:
    ThreadInfo AcceptThInfo;
    ClientID LastClientID;
    SOCKET AcceptSock;
    std::mutex Mut;
};