#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <arpa/inet.h>
#include "ClientWorker.h"

ClientWorker::ClientWorker() = default;

ClientWorker::~ClientWorker() = default;

using namespace std;

void ClientWorker::HandleThread(std::string &s) {
    if (s.empty()) {
        std::cerr << "Missing the data" << std::endl;
        return;
    }
    auto &&split_position = s.find(DELIM);
    if (split_position == std::string::npos) {
        std::cerr << "Invalid" << std::endl;
        return;
    }
    auto &&host_str = s.substr(0, split_position);
    auto &&port_str = s.substr(split_position + 1);
    if (host_str.empty() || port_str.empty()) {
        std::cerr << "Missing host_str or port_str definition" << std::endl;
        return;
    }
    auto port = std::stoi(port_str);
    if (port <= 0) {
        std::cerr << "Port is not valid" << std::endl;
        return;
    }
    static ClientWorker instance;
    instance.Run(host_str, port);
}

void ClientWorker::StartThread(std::string &params) {
    HandleThread(params);
}

void ClientWorker::ListenLoop(const int &socket) {
    std::string buffer;
    short state = 0;
    int error = 0;
    int c = 0;
    state = STATE::START;
    unsigned int answerCode;
    unsigned short numArgCount;

    std::string log;
    std::string pass;
    bool df, ds;

    std::string uname;
    Message m;
    std::string mes;

    std::vector<std::string> bufs;
    std::vector<char> buf;
    while (true) {
        if (error != 0) {
            printf("Socket error: %d", error);
            break;
        }
        switch (state) {
            case START:
                std::cout << "Starting the application...\n";
                break;
            case INIT:
                df = false;
                while (!df) {
                    std::cout << "* MAIL *\n" << "Select the following items:\n" << "2 - Exit\n" << "3 - Register\n"
                              << "4 - Login\n" << OPENT << std::endl;
                    short op;
                    std::cin >> op;
                    df = true;
                    switch (op) {
                        case 2:
                            state = EXIT;
                            break;
                        case 3:
                            state = REG;
                            break;
                        case 4:
                            state = LOG;
                            break;
                        default:
                            df = false;
                            printf("Not valid operation number.");
                            getchar();
                    }
                }
                continue;
                break;
            case EXIT:
                std::cout << "Exiting...\n";
                break;
            case REG:
                std::cout << "You are about to sign up. Enter the <username>: ";
                std::cin >> log;
                std::cout << "Enter the <password>: ";
                std::cin >> pass;
                if (!log.empty() && !pass.empty()) {
                    bufs.emplace_back(log);
                    bufs.emplace_back(pass);
                } else {
                    printf("Login or password missing.\n");
                    getchar();
                }
                break;
            case LOG:
                dr = false;
                cout << "You are about to sign in. Enter the <username>: ";
                cin >> log;
                cout << "Enter the <password>: ";
                cin >> pass;
                if (!log.empty() && !pass.empty()) {
                    bufs.emplace_back(log);
                    bufs.emplace_back(pass);
                } else {
                    printf("Login or password missing.\n");
                    getchar();
                }
                break;
            case INSYS:
                ds = false;
                while (!ds) {
                    cout << "* MAIL *\n" << "Select the following items:\n" << "1 - Send message\n" << "2 - Exit\n"
                         << "3 - Register\n" << "4 - Logout\n" << "5 - Delete user\n" << "6 - Show unread messages\n"
                         << "7 - Show all messages\n" << "8 - Show the exact message\n" << "9 - Delete message\n"
                         << "10 - Resend message\n" << endl;
                    short op;
                    cin >> op;
                    ds = true;
                    switch (op) {
                        case 1:
                            state = SND;
                            break;
                        case 2:
                            state = EXIT;
                            break;
                        case 3:
                            state = REG;
                            break;
                        case 4:
                            state = LOGOUT;
                            break;
                        case 5:
                            state = DEL_US;
                            break;
                        case 6:
                            state = SH_UNR;
                            break;
                        case 7:
                            state = SH_ALL;
                            break;
                        case 8:
                            state = SH_EX;
                            break;
                        case 9:
                            state = DEL_MSG;
                            break;
                        case 10:
                            state = RSND;
                            break;
                        default:
                            ds = false;
                            printf("Not valid operation number.");
                            getchar();
                    }
                }
                continue;
                break;
            case LOGOUT:
                cout << "Logging out.\n" << endl;
                break;
            case SND:
                cout << "Sending the message. Enter the username of the user you would like to send: " << endl;
                cin >> uname;
                cout << "Sending the message. Enter the message to send: " << endl;
                cin >> mes;
                m.Clear();
                m.body = mes;
                if (!uname.empty()) {
                    bufs.emplace_back(uname);
                    bufs.emplace_back(m.Serialize());
                }
                break;
            case DEL_US:
                cout << "Deleting user." << endl;
                break;
            case DEL_MSG:
                cout << "Deleting message." << endl;
                cout << "Enter the message ID which you would like to delete:" << endl;
                cin >> mesId;
                if (mesId > 0) {
                    bufs.emplace_back(std::to_string(mesId));
                }
                break;
            case SH_UNR:
                cout << "Showing unread messages." << endl;
                break;
            case SH_ALL:
                cout << "Showing all messages." << endl;
                break;
            case SH_EX:
                cout << "Showing the exact message." << endl;
                cout << "Enter the message ID which you would like to get: " << endl;
                cin >> mesId;
                if (mesId > 0) {
                    bufs.emplace_back(std::to_string(mesId));
                }
                break;
            case RSND:
                cout << "Resending the exact message." << endl;
                cout << "Resending the message. Enter the username of the user you would like to send: " << endl;
                cin >> uname;
                cout << "Enter the message ID to resend: " << endl;
                cin >> mesId;
                if (!uname.empty()) {
                    bufs.emplace_back(std::to_string(mesId));
                    bufs.emplace_back(uname);
                }
                break;
            default:
                printf("Smth went wrong [non existant state].");
                getchar();
        }
        if (state == INSYS || state == INIT) continue;
        else {
            auto serialized = Serialize((STATE) state, bufs);
            SendTo(socket, serialized);
            bufs.clear();
            ListenRecv(socket, buf);
        }
        ProcessRes(state, buf, m, mes);
    }
}

void ClientWorker::ProcessRes(short &state, vector<char> &buf, Message &m, const string &mes) {
    unsigned int answerCode;
    std::vector<std::string> args;
    switch (state) {
        case START:
            answerCode = Parse(buf, args);
            if (answerCode != NO_OPERATION) {
                if (!args.empty()) {
                    if (std::stoi(args[0]) == SERV_OK) {
                        cout << "Connected to server successfully." << endl;
                        state = INIT;
                        getchar();
                    } else {
                        cout << "Smth went wrong [stcmp]" << endl;
                        getchar();
                    }
                } else {
                    cout << "Smth went wrong [numarg || args]" << endl;
                    getchar();
                }
            } else {
                cout << "Smth went wrong [ansCode]" << endl;
                getchar();
            }
            break;
        case NO_OPERATION:
            answerCode = Parse(buf, args);
            cout << "Enter a valid operation number.\n";
            break;
        case ANSWER:
            cout << "Got the ANSWER message from server." << endl;
            break;
        case EXIT:
            answerCode = Parse(buf, args);
            return;
            break;
        case REG:
            answerCode = Parse(buf, args);
            if (answerCode != NO_OPERATION) {
                if (!args.empty()) {
                    if (std::stoi(args[0]) == SERV_OK) {
                        state = INIT;
                        cout << "User created successfully. Press any key.\n" << endl;
                        getchar();
                    } else {
                        if (args.size() > 1) {
                            cout << "ERROR: while signing up" << args[1] << "]\n";
                        } else cout << "Error while signing up.";
                    }
                }
            } else {
                if (!args.empty())
                    cout << "ERROR: while creating user err[" << args[0] << "]\n";
                else cout << "Unknown error.\n";
                getchar();
            }
            break;
        case LOG:
            answerCode = Parse(buf, args);
            if (answerCode != NO_OPERATION) {
                if (!args.empty()) {
                    if (std::stoi(args[0]) == SERV_OK) {
                        state = INSYS;
                        cout << "User signed in successfully. Press any key.\n" << endl;
                        getchar();
                    } else {
                        if (args.size() > 1) {
                            cout << "ERROR: while signing in" << args[1] << "]\n";
                        } else cout << "Error while signing in.";
                    }
                }
            } else {
                if (!args.empty())
                    cout << "ERROR: while logging in err[" << args[0] << "]\n";
                else cout << "Unknown error.\n";
                getchar();
            }
            break;
        case LOGOUT:
            answerCode = Parse(buf, args);
            if (std::stoi(args[0]) == SERV_OK) {
                cout << "Log out successfully. Press any key." << endl;
                state = INIT;
            } else cout << "Error while logging out. Press any key." << endl;
            getchar();
            break;
        case SND:
            answerCode = Parse(buf, args);
            if (answerCode != NO_OPERATION) {
                if (!args.empty()) {
                    if (std::stoi(args[0]) == SERV_OK) {
                        if (m.Deserialize(args[1])) {
                            m.body = mes;
                            cout << "Message successfully sent to user " << uname << endl;
                            cout << MessageToString(m) << endl;
                            cout << "\nPress any key." << endl;
                            state = INSYS;
                            getchar();
                        }
                    } else {
                        if (args.size() > 1) {
                            cout << "ERROR: while sending [" << args[1] << "]\n";
                        } else cout << "Error while sending.";
                        state = INSYS;
                    }
                }
            } else {
                if (!args.empty())
                    cout << "ERROR: while sending err[" << args[0] << "]\n";
                else cout << "Unknown error.\n";
                state = INSYS;
                getchar();
            }
            break;
        case DEL_US:
            answerCode = Parse(buf, args);
            if (answerCode != NO_OPERATION) {
                if (!args.empty()) {
                    if (std::stoi(args[0]) == SERV_OK) {
                        cout << "User <" << uname << "> deleted successfully." << endl;
                        cout << "\nPress any key." << endl;
                        state = INIT;
                        getchar();
                    } else {
                        if (args.size() > 1) {
                            cout << "ERROR: while deleting [" << args[1] << "]\n";
                        } else cout << "Error while deleting.";
                        state = INSYS;
                    }
                }
            } else {
                if (!args.empty())
                    cout << "ERROR: while deleting err[" << args[0] << "]\n";
                else cout << "Unknown error.\n";
                state = INSYS;
                getchar();
            }
            break;
        case DEL_MSG:
            answerCode = Parse(buf, args);
            if (answerCode != NO_OPERATION) {
                if (!args.empty()) {
                    if (std::stoi(args[0]) == SERV_OK) {
                        cout << "Message <" << mesId << "> deleted successfully." << endl;
                        cout << "\nPress any key." << endl;
                        state = INSYS;
                        getchar();
                    } else {
                        if (args.size() > 1) {
                            cout << "ERROR: while deleting [" << args[1] << "]\n";
                        } else cout << "Error while deleting.";
                        state = INSYS;
                    }
                }
            } else {
                if (!args.empty())
                    cout << "ERROR: while deleting err[" << args[0] << "]\n";
                else cout << "Unknown error.\n";
                state = INSYS;
                getchar();
            }
            break;
        case SH_UNR:
            answerCode = Parse(buf, args);
            if (answerCode != NO_OPERATION) {
                if (!args.empty()) {
                    if (std::stoi(args[0]) == SERV_OK) {
                        for (int i = 1; i < args.size(); i++) {
                            if (m.Deserialize(args[i])) {
                                cout << "Unread message with ID = " << m.id << endl;
                                cout << MessageToString(m) << endl;
                            }
                        }
                        cout << "\nPress any key." << endl;
                        state = INSYS;
                        getchar();

                    } else {
                        if (args.size() > 1) {
                            cout << "ERROR: while showing unread messages [" << args[1] << "]\n";
                        } else cout << "Error while showing unread messages.";
                        state = INSYS;
                    }
                }
            } else {
                if (!args.empty())
                    cout << "ERROR: while showing unread messages err[" << args[0] << "]\n";
                else cout << "Unknown error.\n";
                state = INSYS;
                getchar();
            }
            break;
        case SH_ALL:
            answerCode = Parse(buf, args);
            if (answerCode != NO_OPERATION) {
                if (!args.empty()) {
                    if (std::stoi(args[0]) == SERV_OK) {
                        for (int i = 1; i < args.size(); i++) {
                            if (m.Deserialize(args[i])) {
                                cout << "Message with ID = " << m.id << endl;
                                cout << MessageToString(m) << endl;
                            }
                        }
                        cout << "\nPress any key." << endl;
                        state = INSYS;
                        getchar();

                    } else {
                        if (args.size() > 1) {
                            cout << "ERROR: while showing all messages [" << args[1] << "]\n";
                        } else cout << "Error while showing all messages.";
                        state = INSYS;
                    }
                }
            } else {
                if (!args.empty())
                    cout << "ERROR: while showing unread messages err[" << args[0] << "]\n";
                else cout << "Unknown error.\n";
                state = INSYS;
                getchar();
            }
            break;
        case SH_EX:
            answerCode = Parse(buf, args);
            if (answerCode != NO_OPERATION) {
                if (!args.empty()) {
                    if (std::stoi(args[0]) == SERV_OK) {
                        if (m.Deserialize(args[1])) {
                            cout << "Message with ID = " << m.id << endl;
                            cout << MessageToString(m) << endl;
                        }
                        cout << "\nPress any key." << endl;
                        state = INSYS;
                        getchar();

                    } else {
                        if (args.size() > 1) {
                            cout << "ERROR: while showing unread messages [" << args[1] << "]\n";
                        } else cout << "Error while showing unread messages.";
                        state = INSYS;
                    }
                }
            } else {
                if (!args.empty())
                    cout << "ERROR: while showing unread messages err[" << args[0] << "]\n";
                else cout << "Unknown error.\n";
                state = INSYS;
                getchar();
            }
            break;
        case RSND:
            answerCode = Parse(buf, args);
            if (answerCode != NO_OPERATION) {
                if (args.size() > 1) {
                    if (std::stoi(args[0]) == SERV_OK) {
                        if (m.Deserialize(args[1])) {
                            cout << "Message successfully resent to user " << uname << endl;
                            cout << MessageToString(m) << endl;
                            cout << "\nPress any key." << endl;
                            state = INSYS;
                            getchar();
                        }
                    } else {
                        cout << "ERROR: while resending [" << args[1] << "]\n";
                        state = INSYS;
                    }
                }
            } else {
                if (!args.empty())
                    cout << "ERROR: while resending err[" << args[0] << "]\n";
                else cout << "Unknown error.\n";
                getchar();
            }
            break;
        default:
            printf("Smth went wrong [non existant state].");
            getchar();
    }
}


string ClientWorker::MessageToString(const Message &m) {
    stringstream res;
    res << "ID: " << m.id << "\n";

    res << "TIME: " << m.date_time << "\n";

    res << "FROM: " << m.username << "\n";

    res << "LEN: " << m.len << "\n";

    res << "STATE: " << MESSAGE_STATES[m.state] << "\n";

    res << "BODY: " << m.body << "\n";
    return res.str();
}

string ClientWorker::Serialize(STATE opcode, std::vector<std::string> &args) {
    return __Serialize(opcode, args);
}

STATE ClientWorker::Parse(vector<char> &input, std::vector<std::string> &args) {
    return __Parse(input, args);
}

STATE ClientWorker::ParseOpCode(const string &buf) {
    return __ParseOpCode(buf);
}

void ClientWorker::Run(string host, unsigned short port) {
    printf("Starting new client thread with HOST=%s, PORT=%u\n", host.c_str(), port);
    struct sockaddr_in serv_addr{}, client_addr{};
    auto sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("ERROR opening socket");
        return;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(host.c_str());
    serv_addr.sin_port = htons(port);

    auto status = connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
    if (status < 0)
        return;
    ListenLoop(sockfd);
    close(sockfd);

}

void ClientWorker::SendTo(int socket, std::string &message) {
    sendToSocket(socket, message);
}

bool ClientWorker::ListenRecv(int socket, vector<char> &message) {
    return __ListenRecv(socket, message);
}
