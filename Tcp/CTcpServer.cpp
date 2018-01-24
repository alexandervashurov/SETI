#include "stdinc.h"
#include "CTcpServer.h"

CTcpServer::CTcpServer() {
    AcceptSock = 0;
    LastClientID = 0;
}

CTcpServer::CTcpServer(const CTcpServer &orig) {
}

CTcpServer::~CTcpServer() = default;

void AcceptThread(AcceptThInput pData) {
    WSADATA wsaData{};
    int iResult;

    auto ListenSocket = INVALID_SOCKET;


    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return ;

    }

    ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ListenSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
        return;
    }

    SOCKADDR_IN target{}; //Socket address information
    target.sin_family = AF_INET;
    target.sin_port = htons(pData.ThPort);
    target.sin_addr.s_addr = INADDR_ANY;
    auto _target = reinterpret_cast<sockaddr *>(&target);iResult = ::bind( ListenSocket, _target, sizeof(sockaddr));
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return;
    }

    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return;
    }

    pData.pParent->set_socket(ListenSocket);

    std::cout << "Server started" << std::endl;
    while (true) {
        auto AcceptSocket = accept(ListenSocket, nullptr, nullptr);
        std::cout << "Client accepted" << std::endl;
        if (AcceptSocket == -1) {
            printf("Accept closed\r\n");
            break;
        } else {
            printf("New client accepted\r\n");
            pData.pParent->StartListenTh(AcceptSocket);
        };
        pData.pParent->RemoveDisconnected();
    }
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
    pData.pParent->HandleDisconnection(pData.CliID);
};


void CTcpServer::StartAccept(USHORT Port) {
    std::unique_lock<std::mutex> lock(Mut);

    AcceptThInfo.ThPort = Port;
    AcceptThInfo.bCreated = true;

    AcceptThInput ThInput;
    ThInput.ThPort = Port;
    ThInput.pAcceptSock = AcceptSock;
    ThInput.pParent = this;

    AcceptThInfo.ThHandle = std::make_shared<std::thread>(AcceptThread, ThInput);
}

void CTcpServer::HandleDisconnection(ClientID id) {
    std::unique_lock<std::mutex> lock(Mut);
    disconnected_clients.emplace_back(id);
}

void CTcpServer::RemoveDisconnected() {
    if (disconnected_clients.empty()) return;
    std::unique_lock<std::mutex> lock(Mut);
    for (auto &&id: disconnected_clients) {
        auto &&client = clients[id];
        client.ClientThreadInfo.ThHandle->join();
        clients.erase(id);
    }
    disconnected_clients.clear();
}

void CTcpServer::StartListenTh(SOCKET Sock) {
    std::unique_lock<std::mutex> lock(Mut);
    ClientInfo CliInfo;
    CliInfo.ID = LastClientID++;
    CliInfo.ClientSocket = Sock;

    ListenThInput pThInput;
    pThInput.CliID = CliInfo.ID;
    pThInput.ClientSocket = Sock;
    pThInput.pParent = this;
    CliInfo.ClientThreadInfo.ThHandle = std::make_shared<std::thread>(ListenThread, pThInput);
    clients[CliInfo.ID] = CliInfo;
}

void CTcpServer::shutdown() {
    closesocket(AcceptSock);
    AcceptThInfo.ThHandle->join();
    for (auto &&client: clients) {
        closesocket(client.second.ClientSocket);
    }
    std::this_thread::sleep_for(std::chrono::seconds(3));
    RemoveDisconnected();
    WSACleanup();
}

std::vector<ClientID> CTcpServer::ListClients() {
    std::vector<ClientID> result;
    for(auto&& client: clients){
        result.emplace_back(client.first);
    }
    return result;
}

void CTcpServer::killClient(ClientID id) {
    closesocket(clients[id].ClientSocket);
}
