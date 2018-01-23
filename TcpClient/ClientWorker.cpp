#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "ClientWorker.h"

ClientWorker::ClientWorker() = default;

ClientWorker::~ClientWorker() = default;

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

string Message::Serialize() {
    stringstream ss;
    ss << id << DELIM_SERIALIZE;
    ss << username << DELIM_SERIALIZE;
    ss << date_time << DELIM_SERIALIZE;
    ss << len << DELIM_SERIALIZE;
    ss << state << DELIM_SERIALIZE;
    std::replace(body.begin(), body.end(), DELIM_SERIALIZE, ' ');
    ss << body << DELIM_SERIALIZE;
    return ss.str();
}

void Message::Clear() {
    id = 0;
    username = "";
    date_time = "";
    len = 0;
    state = MSTATE_NORMAL;
    body = "";
};


bool Message::Deserialize(const string &input) {
    std::vector<std::string> args;
    size_t pos = 0;
    std::string token;
    std::string str = input;
    while ((pos = str.find(DELIM_SERIALIZE)) != std::string::npos) {
        token = str.substr(0, pos);
        args.emplace_back(token);
        str = str.substr(pos + sizeof(DELIM_SERIALIZE));
    }
    if (args.size() != MESSAGE_FIELDS_COUNT) return false;
    id = std::stoul(args[0]);
    username = args[1];
    date_time = args[2];
    len = std::stoul(args[3]);
    state = std::stoi(args[4]);
    body = args[5];
    return true;
};

void ClientWorker::ListenLoop(const int &socket) {
    string buffer;
    short state = 0;
    int error = 0;
    int c = 0;
    state = STATE::START;
    unsigned int answerCode;
    unsigned short numArgCount;

    bool isLogged = false;
    string log;
    string pass;
    bool df = false;
    bool ds = false;

    string uname;
    Message m;
    string mes;

    std::vector<std::string> bufs;
    std::vector<char> buf;
    while (true) {
        buf.clear();
        if (error != 0) {
            printf("Socket error: %d", error);
            break;
        }
        switch (state) {
            case START:
                cout << "Starting the application...\n";
                break;
            case INIT:
                df = false;
                while (!df) {
                    cout << "* MAIL *\n" << "Select the following items:\n" << "2 - Exit\n" << "3 - Register\n"
                         << "4 - Login\n" << OPENT << endl;
                    short op;
                    cin >> op;
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
                cout << "Exiting...\n";
                break;
            case REG:
                cout << "You are about to sign up. Enter the <username>: ";
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
    unsigned short numArgCount;
    string *args = NULL;

    switch (state) {
        case START:
            answerCode = Parse(buf, numArgCount, args);
            if (answerCode != NO_OPERATION) {
                if (numArgCount > 0 && args != NULL) {
                    if (atoi(args[0].c_str()) == SERV_OK) {
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
            answerCode = Parse(buf, numArgCount, args);
            cout << "Enter a valid operation number.\n";
            break;
        case ANSWER:
            cout << "Got the ANSWER message from server." << endl;
            break;
        case EXIT:
            answerCode = Parse(buf, numArgCount, args);
            return;
            break;
        case REG:
            answerCode = Parse(buf, numArgCount, args);
            if (answerCode != NO_OPERATION) {
                if (numArgCount > 0 && args != NULL) {
                    if (atoi(args[0].c_str()) == SERV_OK) {
                        state = INIT;
                        cout << "User created successfully. Press any key.\n" << endl;
                        getchar();
                    } else {
                        if (numArgCount > 1) {
                            cout << "ERROR: while signing up" << args[1] << "]\n";
                        } else cout << "Error while signing up.";
                    }
                }
            } else {
                if (numArgCount > 0 && args != NULL)
                    cout << "ERROR: while creating user err[" << args[0] << "]\n";
                else cout << "Unknown error.\n";
                getchar();
            }
            break;
        case LOG:
            answerCode = Parse(buf, numArgCount, args);
            if (answerCode != NO_OPERATION) {
                if (numArgCount > 0 && args != NULL) {
                    if (atoi(args[0].c_str()) == SERV_OK) {
                        state = INSYS;
                        cout << "User signed in successfully. Press any key.\n" << endl;
                        getchar();
                    } else {
                        if (numArgCount > 1) {
                            cout << "ERROR: while signing in" << args[1] << "]\n";
                        } else cout << "Error while signing in.";
                    }
                }
            } else {
                if (numArgCount > 0 && args != NULL)
                    cout << "ERROR: while logging in err[" << args[0] << "]\n";
                else cout << "Unknown error.\n";
                getchar();
            }
            break;
        case LOGOUT:
            answerCode = Parse(buf, numArgCount, args);
            if (atoi(args[0].c_str()) == SERV_OK) {
                cout << "Log out successfully. Press any key." << endl;
                state = INIT;
            } else cout << "Error while logging out. Press any key." << endl;
            getchar();
            break;
        case SND:
            answerCode = Parse(buf, numArgCount, args);
            if (answerCode != NO_OPERATION) {
                if (numArgCount > 0 && args != NULL) {
                    if (atoi(args[0].c_str()) == SERV_OK) {
                        if (m.Deserialize(args[1])) {
                            m.body = mes;
                            cout << "Message successfully sent to user " << uname << endl;
                            cout << MessageToString(m) << endl;
                            cout << "\nPress any key." << endl;
                            state = INSYS;
                            getchar();
                        }
                    } else {
                        if (numArgCount > 1) {
                            cout << "ERROR: while sending [" << args[1] << "]\n";
                        } else cout << "Error while sending.";
                        state = INSYS;
                    }
                }
            } else {
                if (numArgCount > 0 && args != NULL)
                    cout << "ERROR: while sending err[" << args[0] << "]\n";
                else cout << "Unknown error.\n";
                state = INSYS;
                getchar();
            }
            break;
        case DEL_US:
            answerCode = Parse(buf, numArgCount, args);
            if (answerCode != NO_OPERATION) {
                if (numArgCount > 0 && args != NULL) {
                    if (atoi(args[0].c_str()) == SERV_OK) {
                        cout << "User <" << uname << "> deleted successfully." << endl;
                        cout << "\nPress any key." << endl;
                        state = INIT;
                        getchar();
                    } else {
                        if (numArgCount > 1) {
                            cout << "ERROR: while deleting [" << args[1] << "]\n";
                        } else cout << "Error while deleting.";
                        state = INSYS;
                    }
                }
            } else {
                if (numArgCount > 0 && args != NULL)
                    cout << "ERROR: while deleting err[" << args[0] << "]\n";
                else cout << "Unknown error.\n";
                state = INSYS;
                getchar();
            }
            break;
        case DEL_MSG:
            answerCode = Parse(buf, numArgCount, args);
            if (answerCode != NO_OPERATION) {
                if (numArgCount > 0 && args != NULL) {
                    if (atoi(args[0].c_str()) == SERV_OK) {
                        cout << "Message <" << mesId << "> deleted successfully." << endl;
                        cout << "\nPress any key." << endl;
                        state = INSYS;
                        getchar();
                    } else {
                        if (numArgCount > 1) {
                            cout << "ERROR: while deleting [" << args[1] << "]\n";
                        } else cout << "Error while deleting.";
                        state = INSYS;
                    }
                }
            } else {
                if (numArgCount > 0 && args != NULL)
                    cout << "ERROR: while deleting err[" << args[0] << "]\n";
                else cout << "Unknown error.\n";
                state = INSYS;
                getchar();
            }
            break;
        case SH_UNR:
            answerCode = Parse(buf, numArgCount, args);
            if (answerCode != NO_OPERATION) {
                if (numArgCount > 0 && args != NULL) {
                    if (atoi(args[0].c_str()) == SERV_OK) {
                        for (int i = 1; i < numArgCount; i++) {
                            if (m.Deserialize(args[i])) {
                                cout << "Unread message with ID = " << m.id << endl;
                                cout << MessageToString(m) << endl;
                            }
                        }
                        cout << "\nPress any key." << endl;
                        state = INSYS;
                        getchar();

                    } else {
                        if (numArgCount > 1) {
                            cout << "ERROR: while showing unread messages [" << args[1] << "]\n";
                        } else cout << "Error while showing unread messages.";
                        state = INSYS;
                    }
                }
            } else {
                if (numArgCount > 0 && args != NULL)
                    cout << "ERROR: while showing unread messages err[" << args[0] << "]\n";
                else cout << "Unknown error.\n";
                state = INSYS;
                getchar();
            }
            break;
        case SH_ALL:
            answerCode = Parse(buf, numArgCount, args);
            if (answerCode != NO_OPERATION) {
                if (numArgCount > 0 && args != NULL) {
                    if (atoi(args[0].c_str()) == SERV_OK) {
                        for (int i = 1; i < numArgCount; i++) {
                            if (m.Deserialize(args[i])) {
                                cout << "Message with ID = " << m.id << endl;
                                cout << MessageToString(m) << endl;
                            }
                        }
                        cout << "\nPress any key." << endl;
                        state = INSYS;
                        getchar();

                    } else {
                        if (numArgCount > 1) {
                            cout << "ERROR: while showing all messages [" << args[1] << "]\n";
                        } else cout << "Error while showing all messages.";
                        state = INSYS;
                    }
                }
            } else {
                if (numArgCount > 0 && args != NULL)
                    cout << "ERROR: while showing unread messages err[" << args[0] << "]\n";
                else cout << "Unknown error.\n";
                state = INSYS;
                getchar();
            }
            break;
        case SH_EX:
            answerCode = Parse(buf, numArgCount, args);
            if (answerCode != NO_OPERATION) {
                if (numArgCount > 0 && args != NULL) {
                    if (atoi(args[0].c_str()) == SERV_OK) {
                        if (m.Deserialize(args[1])) {
                            cout << "Message with ID = " << m.id << endl;
                            cout << MessageToString(m) << endl;
                        }
                        cout << "\nPress any key." << endl;
                        state = INSYS;
                        getchar();

                    } else {
                        if (numArgCount > 1) {
                            cout << "ERROR: while showing unread messages [" << args[1] << "]\n";
                        } else cout << "Error while showing unread messages.";
                        state = INSYS;
                    }
                }
            } else {
                if (numArgCount > 0 && args != NULL)
                    cout << "ERROR: while showing unread messages err[" << args[0] << "]\n";
                else cout << "Unknown error.\n";
                state = INSYS;
                getchar();
            }
            break;
        case RSND:
            answerCode = Parse(buf, numArgCount, args);
            if (answerCode != NO_OPERATION) {
                if (numArgCount > 1 && args != NULL) {
                    if (atoi(args[0].c_str()) == SERV_OK) {
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
                if (numArgCount > 0 && args != NULL)
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
    stringstream sstr;
    sstr << (int) opcode << DELIM_PARSE << (int) args.size() << DELIM_PARSE;
    for (auto &&str: args) {
        std::replace(str.begin(), str.end(), DELIM_PARSE, ' ');
        sstr << str << DELIM_PARSE;
    }
    return sstr.str();
}

STATE ClientWorker::Parse(vector<char> &input, unsigned short &numarg, string *&args) {
    STATE res = STATE::NO_OPERATION;
    if (input.size() > 0) {
        stringstream buf;
        numarg = 0;
        // find all delimeters
        for (int i = 0; i < input.size(); i++) {
            if (input[i] == DELIM_PARSE)
                numarg++;
        }
        // find all parts
        if (numarg > 0) {
            args = new string[numarg - 1];
            string opcodeBuf;
            unsigned short cc = 0;
            for (int i = 0; i < input.size(); i++) {
                if (input[i] == DELIM_PARSE) {
                    if (cc == 0) {
                        opcodeBuf = buf.str();
                    } else if (cc > 1) {
                        args[cc - 2] = buf.str();
                    }
                    cc++;
                    buf.str(std::string());
                } else {
                    buf << input[i];
                }
            }
            // args[0] is operation code
            res = ParseOpCode(opcodeBuf);
            numarg -= 2;
        }
    }
    return res;
}

STATE ClientWorker::ParseOpCode(const string &buf) {
    int res = atoi(buf.c_str());
    if (API_SIZE > 0)
        for (int i = 0; i < API_SIZE; i++)
            if (res == i)
                return static_cast<STATE>(i);
    return STATE::NO_OPERATION;
}

void ClientWorker::Run(string host, unsigned short port) {
    printf("Starting new client thread with HOST=%s, PORT=%u\n", host.c_str(), port);
    int n;

    int sockfd;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("ERROR opening socket");
        return;
    }
    server = gethostbyname(host.c_str());
    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        return;
    }

    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy((char *) &serv_addr.sin_addr.s_addr, server->h_addr, (size_t) server->h_length);
    serv_addr.sin_port = htons(port);
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        return;
    }
    ListenLoop(sockfd);
    close(sockfd);

}

void ClientWorker::SendTo(int socket, std::string &message) {
    int res = 0;
    int size = message.size();
    char *ss = new char[size + BUFSIZE];
    char *sizeBuf = new char[BUFSIZE];
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

bool ClientWorker::ListenRecv(int socket, vector<char> &message) {
    char c[BUFSIZE];
    unsigned int size = 0;
    int res = recv(socket, c, BUFSIZE, 0);
    if (res == BUFSIZE) {
        size = atoi(c);
        char *recvbuf = new char[size];
        int res = recv(socket, recvbuf, size, 0);
        printf("String received: %s\n", recvbuf);
        if (res > 0) {
            message.clear();
            for (int i = 0; i < res; i++)
                if (recvbuf[i] != '\r' && recvbuf[i] != '\0')
                    message.emplace_back(recvbuf[i]);

        }
    } else return false;
    return true;
}
