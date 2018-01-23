#pragma once
#include <cstring>
#include <vector>
#include <sstream>
#include <iostream>
#ifndef WIN32
#include <sys/socket.h>
#define SOCKET int
#else
#include <winsock2.h>
#endif

const short BUFSIZE = 10;

static void sendToSocket(SOCKET socket, std::string &message) {
    int res = 0;
    auto size = message.size();
    auto *ss = new char[size + BUFSIZE];
    auto *sizeBuf = new char[BUFSIZE];
    snprintf(sizeBuf, BUFSIZE, "%d", size);
    int shift = BUFSIZE - strlen(sizeBuf);
    for (int i = BUFSIZE - 1; i >= 0; i--) {
        if (i >= shift)
            ss[i] = sizeBuf[i - shift];
        else
            ss[i] = '0';
    }
    for (int i = BUFSIZE; i < size + BUFSIZE; i++) {
        ss[i] = message[i - BUFSIZE];
    }
    printf("String to send: %s\n", ss);
    res = send(socket, ss, size + BUFSIZE, 0);
    if (res != size + BUFSIZE)
        printf("Send failed: %s != %zd!\n", ss, size);
}

static bool __ListenRecv(SOCKET socket, std::vector<char> &message) {
    message.clear();
    char c[BUFSIZE];
    unsigned int size = 0;
    int res = recv(socket, c, BUFSIZE, MSG_WAITALL);
    if (res == BUFSIZE) {
        size = atoi(c);
        auto recvbuf = new char[size];
        int res = recv(socket, recvbuf, size, MSG_WAITALL);
        printf("String received: %s\n", recvbuf);
        if (res > 0) {
            for (int i = 0; i < res; i++)
                if (recvbuf[i] != '\r' && recvbuf[i] != '\0')
                    message.emplace_back(recvbuf[i]);

        }
        delete recvbuf;
    } else return false;
    return true;
}

static STATE __ParseOpCode(const std::string &buf) {
    int res = std::stoi(buf);
    if (res < API_SIZE) return static_cast<STATE>(res);
    return STATE::NO_OPERATION;
}

static STATE __Parse(std::vector<char> &input, std::vector<std::string> &args) {
    STATE res = STATE::NO_OPERATION;
    args.clear();
    if (input.empty()) return res;
    std::stringstream buf;
    std::string opcodeBuf;

    auto cc = 0;
    for (auto &&i : input) {
        if (i != DELIM_PARSE) {
            buf << i;
            continue;
        }
        if (cc == 0) {
            opcodeBuf = buf.str();
        } else if (cc > 1) {
            args.push_back(buf.str());
        }
        cc++;
        buf.str(std::string());
    }
    return __ParseOpCode(opcodeBuf);
}

static std::string __Serialize(STATE opcode, std::vector<std::string> &args) {
    std::stringstream sstr;
    sstr << (int) opcode << DELIM_PARSE << (int) args.size() << DELIM_PARSE;
    for (auto &&str: args) {
        std::replace(str.begin(), str.end(), DELIM_PARSE, ' ');
        sstr << str << DELIM_PARSE;
    }
    return sstr.str();
}