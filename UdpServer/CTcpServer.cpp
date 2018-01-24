#include "stdinc.h"
#include "CTcpServer.h"

CTcpServer::CTcpServer() {
    server_socket = 0;
    LastClientID = 0;
}

CTcpServer::CTcpServer(const CTcpServer &orig) {
}

CTcpServer::~CTcpServer() = default;

ClientID get_id_for_client_info(sockaddr_in *client_addr) {
    ClientID result = client_addr->sin_addr.s_addr;
    return result << 32 | client_addr->sin_port;
}

ClientID CTcpServer::get_client_id(sockaddr *client_addr) {
    auto &&cl_addr = reinterpret_cast<sockaddr_in *>(client_addr);
    auto id = get_id_for_client_info(cl_addr);
    if (clients.find(id) == std::end(clients)) {
        char ip_buf[INET_ADDRSTRLEN];
        inet_ntop(cl_addr->sin_family, &cl_addr->sin_addr, ip_buf, sizeof(ip_buf));
        std::string ip_str(ip_buf);
        SOCKET pipe_d[2];
        pipe(pipe_d);
        StartListenTh(id, *cl_addr, pipe_d);
    }
    return id;
}

void AcceptThread(AcceptThInput pData) {
    sockaddr_in addr{};
    SOCKET ListenSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (ListenSocket < 0) {
        std::exit(EXIT_FAILURE);
    }
    addr.sin_family = AF_INET;
    addr.sin_port = htons(pData.ThPort);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(ListenSocket, (struct sockaddr *) &addr, sizeof(sockaddr)) == -1) {
        std::exit(EXIT_FAILURE);
    }
    pData.pParent->set_socket(ListenSocket);

    std::cout << "Server started" << std::endl;
    char buffer[MESSAGE_SIZE + 1];
    while (true) {
        memset(buffer, 0, MESSAGE_SIZE + 1);
        sockaddr client_addr{};
        auto receive_status = receive_udp(ListenSocket, buffer, &client_addr);
        if (receive_status == -1) {
            printf("receive error");
            break;
        }
        auto &&client_id = pData.pParent->get_client_id(&client_addr);
        auto&& pipe_id = pData.pParent->pipes[client_id];
        write(pipe_id, buffer, receive_status);
        pData.pParent->RemoveDisconnected();
    }
};

void ListenThread(ListenThInput pData) {
    printf("Listen thread is run\r\n");
    ServerWorker w(pData.pParent);
    w.Init(pData.ClientSocket, pData.addr);
    bool res = w.MainLoop();
    if (res)
        printf("Client #%li terminated successfully!\n", pData.CliID);
    else
        printf("Client #%li terminated abnormally!\n", pData.CliID);
    pData.pParent->HandleDisconnection(pData.CliID);
};


void CTcpServer::StartAccept(USHORT Port) {
    std::unique_lock<std::mutex> lock(Mut);

    AcceptThInfo.ThPort = Port;
    AcceptThInfo.bCreated = true;

    AcceptThInput ThInput;
    ThInput.ThPort = Port;
    ThInput.pAcceptSock = server_socket;
    ThInput.pParent = this;

    AcceptThInfo.ThHandle = std::make_shared<std::thread>(AcceptThread, ThInput);
}

void CTcpServer::HandleDisconnection(ClientID id) {
    std::unique_lock<std::mutex> lock(Mut);
    auto &&client = clients[id];
    close(client.socket);
    disconnected_clients.emplace_back(client);
    clients.erase(id);
    close(pipes[id]);
    pipes.erase(id);
}

void CTcpServer::RemoveDisconnected() {
    if (disconnected_clients.empty()) return;
    std::unique_lock<std::mutex> lock(Mut);
    for (auto &&client: disconnected_clients) {
        client.ClientThreadInfo.ThHandle->join();
    }
    disconnected_clients.clear();
}

void CTcpServer::LockClient(const std::string &name) {
    std::lock_guard<std::mutex> g_lock(Mut);
    locks[name].lock();
}

void CTcpServer::UnlockClient(const std::string &name) {
    std::lock_guard<std::mutex> g_lock(Mut);
    auto &&lock = locks.find(name);
    if (lock != locks.end()) {
        lock->second.unlock();
    }
}

void CTcpServer::StartListenTh(ClientID id, sockaddr_in addr, const SOCKET *pipe_d) {
    std::unique_lock<std::mutex> lock(Mut);
    ClientInfo CliInfo;
    CliInfo.ID = id;
    CliInfo.addr = addr;
    CliInfo.socket = pipe_d[1];

    ListenThInput pThInput;
    pThInput.CliID = CliInfo.ID;
    pThInput.pParent = this;
    pThInput.addr = addr;
    pThInput.ClientSocket = pipe_d[0];
    CliInfo.ClientThreadInfo.ThHandle = std::make_shared<std::thread>(ListenThread, pThInput);
    clients[CliInfo.ID] = CliInfo;
    pipes[CliInfo.ID] = CliInfo.socket;
}

void CTcpServer::shutdown() {
    close(server_socket);
    AcceptThInfo.ThHandle->join();
    for (auto &&client: clients) {
        close(client.second.socket);
        close(pipes[client.first]);
    }
    std::this_thread::sleep_for(std::chrono::seconds(3));
    RemoveDisconnected();
}

std::vector<ClientID> CTcpServer::ListClients() {
    std::vector<ClientID> result;
    for (auto &&client: clients) {
        result.emplace_back(client.first);
    }
    return result;
}

void CTcpServer::killClient(ClientID id) {
    close(clients[id].socket);
    close(pipes[id]);
}
