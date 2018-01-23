#include <cstdlib>
#include <cstdio>
#include <iostream>

#include "CTcpServer.h"

int main(int argc, char **argv) {
    std::string str;
    CTcpServer Server;
    Server.StartAccept(5555);
    while (true) {
        std::cin >> str;
        if (str == "exit") {
            std::exit(0);
            break;
        }
    }
    return (EXIT_SUCCESS);
}

