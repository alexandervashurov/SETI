#include "stdinc.h"
#include "CTcpServer.h"

CTcpServer::CTcpServer() {
    AcceptSock = 0;
    LastClientID = 0;
}

CTcpServer::CTcpServer(const CTcpServer& orig) {
}

CTcpServer::~CTcpServer() = default;

void AcceptThread(AcceptThInput pData)
{
    WSADATA wsaData;
    int iResult;

    SOCKET ListenSocket = INVALID_SOCKET;

    struct addrinfo *result = NULL;
    struct addrinfo hints;


    // Initialize Winsock
    auto wrd = MAKEWORD(2,2);
    iResult = WSAStartup(wrd, &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return ;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, "5555", &hints, &result);
    if ( iResult != 0 ) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return ;
    }

    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return ;
    }

    iResult = ::bind( ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return ;
    }

    freeaddrinfo(result);

    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return ;
    }

    SOCKET AcceptSocket;
    bool bQuit = false;
    while (!bQuit) {
        AcceptSocket = accept(ListenSocket, nullptr, nullptr);
        if (AcceptSocket == -1) {
            printf("Accept closed\r\n");
            bQuit = true;
        } else {
            printf("New client accepted\r\n");
            pData.pParent->StartListenTh(AcceptSocket);
        };
    }
    return;
};

void ListenThread(ListenThInput pData) {
    printf("Listen thread is run\r\n");
    ServerWorker w;
    w.Init(pData.ClientSocket);
    bool res = w.MainLoop();
    if (res)
        printf("Client #%d terminated successfully!\n", pData.CliID);
    else
        printf("Client #%d terminated abnormally!\n", pData.CliID);
};


void CTcpServer::StartAccept(USHORT Port)
{
    std::unique_lock<std::mutex> lock(Mut);

    AcceptThInfo.ThPort = Port;
    AcceptThInfo.bCreated = true;

    AcceptThInput ThInput;
    ThInput.ThPort = Port;
    ThInput.pAcceptSock = &AcceptSock;
    ThInput.pParent = this;

    AcceptThInfo.ThHandle = std::make_shared<std::thread> (AcceptThread, ThInput);
}

void CTcpServer::StartListenTh(SOCKET Sock)
{
    std::unique_lock<std::mutex> lock(Mut);
    ClientInfo CliInfo;
    CliInfo.ID = LastClientID++;
    CliInfo.ClientSocket = Sock;

    ListenThInput pThInput;
    pThInput.CliID = CliInfo.ID;
    pThInput.ClientSocket = Sock;
    pThInput.pParent = this;

    CliInfo.ClientThreadInfo.ThHandle = std::make_shared<std::thread>(ListenThread, pThInput);

}
