#include <cstdlib>
#include <iostream>

#include "CTcpServer.h"

int main(int argc, char **argv) {
    std::string str;
    CTcpServer Server;
    Server.StartAccept(5555);
    while (true) {
        std::cin >> str;
        if (str == "help") {
            std::cout
                    << "help" << std::endl
                    << "list" << std::endl
                    << "kill" << std::endl
                    << "exit" << std::endl;
        } else if (str == "list") {
            auto &&clients = Server.ListClients();
            if(clients.empty()) std::cout << "No clients" << std::endl;
            for (auto &&client: clients) {
                std::cout << client << std::endl;
            }
        } else if (str == "kill") {
            ClientID client;
            std::cout << "Enter client id: " << std::flush;
            std::cin >> client;
            Server.killClient(client);
        } else if (str == "exit") {
            break;
        }
    }
    Server.shutdown();
    return (EXIT_SUCCESS);
}

