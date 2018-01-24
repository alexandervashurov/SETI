#pragma once

#include <cstring>
#include <vector>
#include <sstream>
#include <iostream>
#include <tuple>
#include "Message.h"
#ifndef WIN32

#include <sys/socket.h>
#include "unistd.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define SOCKET int
#else

#include <winsock2.h>

#endif

const short BUFSIZE = 10;
const char EOF_SYM = (char) 2;

const short UDP_DG_LEN = 32;     // UNICODE symbol count in one packet.
const short TECH_DG_SIZE = 10;     // Char count, in which the length of the packet and it's number are secured.

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

#define MESSAGE_SIZE 512
#define MESSAGE_END "\r\n\r\n"

#define CHUNK_SUCCESS_MESSAGE 32
#define CONTENT_MESSAGE 16

#define UDP_PACKET_SIZE 10

#define success_header_size (sizeof(int) + sizeof(char))
#define content_header_size (sizeof(int) * 2 + sizeof(char))

static std::string make_chunk_success_packet(int chunk_number) {
    char info_str[success_header_size];
    info_str[0] = CHUNK_SUCCESS_MESSAGE;
    auto packet_info = (int *) (info_str + sizeof(char));
    packet_info[0] = chunk_number;
    return std::string(info_str, success_header_size);
}


static std::string make_content_message(int chunk_number, int total, std::string &content) {
    char info_str[content_header_size];
    info_str[0] = CONTENT_MESSAGE;
    auto packet_info = reinterpret_cast<int *>(info_str + sizeof(char));
    packet_info[0] = chunk_number;
    packet_info[1] = total;
    auto packet = std::string(info_str, content_header_size) + content;
    return packet;
}

static std::tuple<int, int, std::string> parse_content_message(char *received_message, int message_size) {
    auto as_int_array = reinterpret_cast<int *>(received_message + 1);
    int message_number = as_int_array[0];
    int message_total = as_int_array[1];
    char *content = received_message + content_header_size;
    std::string message(content, message_size - content_header_size);
    return std::make_tuple(message_number, message_total, message);
};

static int send_udp(SOCKET socket, std::string &packet, sockaddr *addr) {
    return sendto(socket, packet.data(), packet.size(), 0, addr, sizeof(sockaddr));
}

static int receive_udp(SOCKET socket, char *buffer, sockaddr *addr) {
    auto client_info_size = sizeof(sockaddr);
#ifndef WIN32
    auto client_info_size_ptr = reinterpret_cast<socklen_t *>(&client_info_size);
#else
    auto client_info_size_ptr = reinterpret_cast<int *>(&client_info_size);
#endif
    return recvfrom(socket, buffer, MESSAGE_SIZE, 0, addr, client_info_size_ptr);
}

static int select_udp(SOCKET socket, int timeout_ms) {
    fd_set rfds{};
    FD_ZERO(&rfds);
    FD_SET(socket, &rfds);
    timeval tv{};
    tv.tv_sec = 0;
    tv.tv_usec = timeout_ms * 1000;
    return select(socket + 1, &rfds, NULL, NULL, &tv);
}

static int send_message_udp(sockaddr_in &server_addr, SOCKET socket, std::string &message) {
    std::vector<std::string> send_buffer;
    auto mess_size = message.size();
    int size = mess_size / UDP_PACKET_SIZE;
    int reminder = mess_size % UDP_PACKET_SIZE;
    if (reminder > 0) ++size;
    for (auto i = 0; i < size; ++i) {
        auto chunk = message.substr(i * UDP_PACKET_SIZE, UDP_PACKET_SIZE);
        auto packet = make_content_message(i, size, chunk);
        send_buffer.emplace_back(packet);
    }
    auto _server_addr = reinterpret_cast<sockaddr *>(&server_addr);
//    for (auto &&packet: send_buffer) {
//        auto &&send_stat = send_udp(socket, packet, _server_addr);
//    }
    for (int l = 0; l < send_buffer.size(); ++l) {
        if (l == 0) continue;
        auto &&packet = send_buffer[l];
        auto &&send_stat = send_udp(socket, packet, _server_addr);
    }
//    for (int l = 0; l < send_buffer.size(); ++l) {
//        auto&& packet = send_buffer[l];
//        auto &&send_stat = send_udp(socket, packet, _server_addr);
//        if(l == 1) {
//            auto &&send_stat = send_udp(socket, packet, _server_addr);
//        }
//    }
    char receive_buffer[MESSAGE_SIZE + 1];
    sockaddr client_info{};
    int send_index = 0;

    int k;
    for (k = 0; k < 100; ++k) {
        memset(receive_buffer, 0, MESSAGE_SIZE + 1);
        auto select_status = select_udp(socket, 300);
        if (select_status < 0) {
            std::cerr << "Error in select" << std::endl;
            break;
        } else if (select_status) {
            auto bytes = receive_udp(socket, receive_buffer, &client_info);
            auto address = reinterpret_cast<sockaddr_in *>(&client_info);
            if (address->sin_addr.s_addr != server_addr.sin_addr.s_addr) continue;
            if (bytes < 0) {
                std::cerr << "Receive error" << std::endl;
                continue;
            }
            if (bytes == 0) continue;
            auto message_type = receive_buffer[0];
            if (message_type == CONTENT_MESSAGE) continue;
            if (message_type == CHUNK_SUCCESS_MESSAGE) {
                int message_number = *(int *) (receive_buffer + 1);
                if (message_number < send_index) continue;
                if (message_number == send_index) {
                    send_index++;
                    if (send_index == send_buffer.size()) break;
                    continue;
                }
            }
        }
        for (int i = send_index; i < send_buffer.size(); ++i) {
            auto &&packet = send_buffer[i];
            send_udp(socket, packet, _server_addr);
        }
    }
    if (k == 100) {
        std::cerr << "server unreachable" << std::endl;
        return -1;
    }
    return 0;
}

static int receive_message_udp(sockaddr_in &server_addr, SOCKET socket, std::vector<char> &received_mes,
                               bool udp_client_specific = false) {
    received_mes.clear();
    char buffer[MESSAGE_SIZE + 1];
    std::vector<std::string> receive_buffer;
    sockaddr client_info{};
    auto _server_addr = reinterpret_cast<sockaddr *>(&server_addr);
    bool first_packet = true;
    while (true) {
        memset(buffer, 0, MESSAGE_SIZE + 1);
        if (!first_packet) {
            auto select_status = select_udp(socket, 30000);
            if (select_status <= 0)
                return -2;
        }
        int bytes;
        if(!udp_client_specific) {
            bytes = receive_udp(socket, buffer, &client_info);
        } else{
#ifndef WIN32
            bytes = read(socket, buffer, MESSAGE_SIZE + 1);
#endif
        }
        first_packet = false;
        auto address = reinterpret_cast<sockaddr_in *>(&client_info);
//        if (address->sin_addr.s_addr != server_addr.sin_addr.s_addr) continue;
        if (bytes < 0) {
            std::cerr << "Receive error" << std::endl;
            continue;
        }
        if (bytes == 0) continue;
        auto message_type = buffer[0];
        if (message_type == CHUNK_SUCCESS_MESSAGE) continue;
        if (message_type == CONTENT_MESSAGE) {
            auto expected_chunk = receive_buffer.size();
            int message_number;
            int message_total;
            std::string message;
            std::tie(message_number, message_total, message) = parse_content_message(buffer, bytes);
            if (message_number <= expected_chunk) {
                auto packet = make_chunk_success_packet(message_number);
                auto &&send_stat = send_udp(socket, packet, _server_addr);
            }
            if (message_number == expected_chunk) {
                receive_buffer.emplace_back(message);
            }
            if (receive_buffer.size() == message_total) {
                auto packet = make_chunk_success_packet(message_total);
                auto &&send_stat = send_udp(socket, packet, _server_addr);
                break;
            }
        }
    }
    std::string received;
    for (auto &&chunk : receive_buffer) {
        received += chunk;
    }
    auto &&message_end = received.find(MESSAGE_END);
    if (message_end != std::string::npos) {
        received_mes.reserve(received.size());
        for (int i = 0; i < message_end; ++i) {
            received_mes.emplace_back(received[i]);
        }
        return 0;
    } else {
        return -1;
    }
}