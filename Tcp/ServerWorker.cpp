/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <algorithm>
#include <direct.h>
#include "ServerWorker.h"


//http://www.strudel.org.uk/itoa/
/**
	 * C++ version 0.4 std::string style "itoa":
	 * Contributions from Stuart Lowe, Ray-Yuan Sheu,

	 * Rodrigo de Salvo Braz, Luc Gallant, John Maloney
	 * and Brian Hunt
	 */
string itoa(int value) {
    int base = 10;
    std::string buf;
    enum {
        kMaxDigits = 35
    };
    buf.reserve(kMaxDigits);
    int quotient = value;
    // Translating number to string with base:
    do {
        buf += (char) (((int) '0') + abs(quotient % base));
        quotient /= base;
    } while (quotient);
    // Append the negative sign
    if (value < 0) buf += '-';
    std::reverse(buf.begin(), buf.end());
    return buf;
}

ServerWorker::ServerWorker() {
}

ServerWorker::~ServerWorker() {
    CloseSocket();
}

void ServerWorker::Init(SOCKET s) {
    socket = s;
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
};

bool Message::Deserialize(const string &input) {
    bool res = true;
    int numarg = 0;
    string *args = NULL;
    if (input.size() > 0) {
        stringstream buf;
        numarg = 0;
        // find all delimeters
        for (int i = 0; i < input.size(); i++) {
            if (input[i] == DELIM_SERIALIZE)
                numarg++;
        }
        // find all parts
        if (numarg > 0) {
            args = new string[numarg];
            unsigned short cc = 0;
            for (int i = 0; i < input.size(); i++) {
                if (input[i] == DELIM_SERIALIZE) {
                    args[cc] = buf.str();
                    cc++;
                    buf.str(std::string());
                } else {
                    buf << input[i];
                }
            }
        }
    }
    if (numarg == MESSAGE_FIELDS_COUNT && args != NULL) {
        id = strtoul(args[0].c_str(), NULL, 10);
        username = args[1];
        date_time = args[2];
        len = strtoul(args[3].c_str(), NULL, 10);
        state = atoi(args[4].c_str());
        body = args[5];
    } else
        res = false;
    return res;
};

bool ServerWorker::MainLoop() {
    STATE State = NO_OPERATION;
    string MsgStr;
    string currentUserName;
    string errMessage;
    int mesId;
    bool RegisterState = false, LoginState = false;
    unsigned short numarg;
    string *args = NULL;
    string *args2 = new string[STRING_BUFFER_SIZE];
    char *resBuf;
    int argsCount = 0;
    const string servOk = itoa(SERV_OK);
    const string noOp = itoa(NO_OPERATION);

    unsigned long mesCount = 0;
    unsigned long size;
    Message *m = NULL;

    while (true) {
        bool res = ListenRecv(resBuf);
        MsgStr = string(resBuf);
        if (res) {
            argsCount = 1;
            State = Parse(MsgStr, numarg, args);
            switch (State) {
                case START:
                    args2[0] = servOk;
                    break;
                case EXIT:
                    args2[0] = servOk;
                    printf("Client with ID: %d is disconnect!\n", socket);
                    break;
                case REG:
                    if (args != NULL && numarg > 1) {
                        errMessage = RegisterNewUser(args[0], args[1], RegisterState);
                        if (RegisterState) {
                            args2[0] = servOk;
                        } else {
                            argsCount = 2;
                            args2[0] = noOp;
                            args2[1] = errMessage;
                        }
                    } else {
                        argsCount = 2;
                        args2[0] = noOp;
                        args2[1] = "Not valid args.";
                    }
                    break;
                case LOG:
                    if (args != NULL && numarg > 1) {
                        errMessage = LoginNewUser(args[0], args[1], LoginState);
                        if (LoginState) {
                            args2[0] = servOk;
                            currentUserName = args[0];
                        } else {
                            argsCount = 2;
                            args2[0] = noOp;
                            args2[1] = errMessage;
                        }
                    } else {
                        argsCount = 2;
                        args2[0] = noOp;
                        args2[1] = "Not valid args.";
                    }
                    break;
                case LOGOUT:
                    cout << "Logging out." << endl;
                    currentUserName = "";
                    args2[0] = servOk;
                    break;
                case SND:
                    cout << "Sending the message." << endl;
                    if (args != NULL && numarg > 1) {
                        m = new Message();
                        if (m->Deserialize(args[1]) && args[0].size() > 0) {
                            if (currentUserName.size() > 0)
                                mesId = AddMessage(m, args[0], currentUserName, errMessage);
                            else { mesId = 0; }
                            if (mesId == 0) {
                                argsCount = 2;
                                args2[0] = noOp;
                                args2[1] = "Error while sending the message [" + errMessage + "]";
                            } else {
                                m->body = "";
                                argsCount = 2;
                                args2[0] = servOk;
                                args2[1] = m->Serialize();
                            }
                        }
                        delete m;
                    }
                    break;
                case DEL_US:
                    cout << "Deleting user." << endl;
                    if (currentUserName.size() > 0) {
                        errMessage = DeleteUser(currentUserName);
                        argsCount = 2;
                        if (errMessage.size() == 0) {
                            args2[0] = servOk;
                            args2[1] = currentUserName;
                        } else {
                            args2[0] = noOp;
                            args2[1] = errMessage;
                        }
                        currentUserName = "";
                    }
                    break;
                case DEL_MSG:
                    cout << "Deleting message." << endl;
                    if (args != NULL && numarg > 0) {
                        if (currentUserName.size() > 0) {
                            errMessage = DeleteMes(currentUserName, args[0]);
                            argsCount = 2;
                            if (errMessage.size() == 0) {
                                args2[0] = servOk;
                                args2[1] = "";
                            } else {
                                args2[0] = noOp;
                                args2[1] = errMessage;
                            }
                        }
                    }
                    break;
                case SH_UNR:
                    size = 0;
                    cout << "Showing unread messages." << endl;
                    if (args != NULL && numarg >= 0) {
                        if (currentUserName.size() > 0) {
                            if (args != NULL)
                                delete[] args;
                            errMessage = ShowUnreadMes(currentUserName, mesCount, args);
                            argsCount = 2;
                            if (errMessage.size() == 0) {
                                args2[0] = servOk;
                                if (mesCount > 0) {
                                    argsCount = mesCount + 1;
                                    for (int i = 0; i < mesCount; i++)
                                        args2[i + 1] = args[i];
                                }
                            } else {
                                args2[0] = noOp;
                                args2[1] = errMessage;
                            }
                        }
                    }
                    break;
                case SH_ALL:
                    size = 0;
                    cout << "Showing all messages." << endl;
                    if (args != NULL && numarg >= 0) {
                        if (currentUserName.size() > 0) {
                            if (args != NULL)
                                delete[] args;
                            errMessage = ShowAllMes(currentUserName, mesCount, args);
                            argsCount = 2;
                            if (errMessage.size() == 0) {
                                args2[0] = servOk;
                                if (mesCount > 0) {
                                    argsCount = mesCount + 1;
                                    for (int i = 0; i < mesCount; i++)
                                        args2[i + 1] = args[i];
                                }
                            } else {
                                args2[0] = noOp;
                                args2[1] = errMessage;
                            }
                        }
                    }
                    break;
                case SH_EX:
                    cout << "Showing the exact message." << endl;
                    size = 0;
                    if (args != NULL && numarg >= 1) {
                        if (currentUserName.size() > 0) {
                            errMessage = ShowExactMes(currentUserName, args2[1], args[0]);
                            argsCount = 2;
                            if (errMessage.size() == 0) {
                                args2[0] = servOk;
                            } else {
                                args2[0] = noOp;
                                args2[1] = errMessage;
                            }
                        }
                    }
                    break;
                case RSND:
                    cout << "Resending the exact message." << endl;
                    size = 0;
                    if (args != NULL && numarg >= 2) {
                        if (currentUserName.size() > 0) {
                            errMessage = ResendMes(currentUserName, args[1], args2[1], args[0]);
                            argsCount = 2;
                            if (errMessage.size() == 0) {
                                args2[0] = servOk;
                            } else {
                                args2[0] = noOp;
                                args2[1] = errMessage;
                            }
                        }
                    }
                    break;
                default:
                    cout << "Unknown state: " << State << endl;
                    break;
            }
            SendTo(Serialize(ANSWER, argsCount, args2).c_str());
        }
    }
    if (args != NULL)
        delete[] args;
    if (args2 != NULL)
        delete[] args2;
    return true;
}

void ServerWorker::OpenSem(const string &name) {
    auto &&lock = locks.find(name);
    if (lock == std::end(locks)) {
        locks[name] = std::make_shared<std::mutex>();
    }
    locks[name]->lock();
}

void ServerWorker::CloseSem(const string &name) {
    auto &&lock = locks.find(name);
    if (lock != std::end(locks)) locks[name]->unlock();
}

string ServerWorker::GetPasswFilePth(const string &username) {
    string pth = USERS_FOLDER;
    pth += username;
    pth += PASSW_FILE;
    return pth;
}

string ServerWorker::GetMessageFilePth(const string &username) {
    string pth = USERS_FOLDER;
    pth += username;
    pth += MESSAGE_FILE;
    return pth;
}

bool ServerWorker::CheckUser(const string &name) {
    string pth = GetPasswFilePth(name);
    string pass2;
    OpenSem(name);
    ifstream fin(pth.c_str());
    if (fin.good())
        fin >> pass2;
    fin.close();
    CloseSem(name);
    return pass2.size() > 0;
}

string ServerWorker::RegisterNewUser(const string &uname, const string &passw, bool &res) {
    int stat;
    res = false;
    if (uname.length() > 0 && passw.length() > 0) {

        ifstream fin(GetPasswFilePth(uname).c_str());
        if (fin.good()) {
            printf("ERROR: User already exists.\n");
            return "Username is already used by another user. Please, choose other option for username.\n";
        }
        string pth = "./users/";
        _mkdir(pth.c_str());
        pth += uname;

        stat = _mkdir(pth.c_str());
        if (stat != 0) {
            printf("ERROR: Failed to create dir. ErrCode = %d\n", stat);
            return "Internal server issue. Please, try again.\n";
        }
        printf("Dir %s created successfully.\n", uname.c_str());
        pth += PASSW_FILE;
        ofstream out(pth.c_str());
        if (!out.good()) {
            printf("ERROR: Password is not saved.\n");
            return "ERROR: Password is not saved.\n";
        }
        out << passw;
        out.close();
        res = true;
        ofstream mFile(GetMessageFilePth(uname).c_str());
        mFile.close();
    }
    return "";
}

string ServerWorker::LoginNewUser(const string &uname, const string &passw, bool &res) {
    res = false;
    if (uname.length() > 0 && passw.length() > 0) {
        string pass2;
        string pth = GetPasswFilePth(uname);
        OpenSem(uname);
        ifstream fin(pth.c_str());
        if (!fin.good()) {
            printf("ERROR: Could not load file %s.\n", pth.c_str());
            return "Internal server issue. Please, try again.\n";


        }
        fin >> pass2;
        if (pass2.compare(passw) != 0) {
            printf("ERROR: Password is not correct or there is no access to the pass. path = \"%s\"", pth.c_str());
            return "Internal server issue. Please, try again.\n";
        }
        res = true;
        printf("Successfully logged in! User: %s\n", uname.c_str());
        fin.close();
        CloseSem(uname);
    }
    return "";
}

string ServerWorker::DeleteUser(const string &username) {
    string buf;
    time_t seconds = time(NULL);
    tm *timeinfo = localtime(&seconds);

    buf.append(asctime(timeinfo));
    int position = buf.find('\n');
    buf.replace(position, 1, "");
    position = buf.find(" ");
    while (position != string::npos) {
        buf.replace(position, 1, "_");
        position = buf.find(" ", position + 1);
    }
    buf.append("_");
    buf.append(username);
    string oldname = USERS_FOLDER;
    oldname.append(username);
    string newname = USERS_FOLDER;
    newname.append(buf);
    int res = rename(oldname.c_str(), newname.c_str());
    if (res != 0)
        return "An error occured while removing existing user! Please, try again later!";
    return "";
}

unsigned long ServerWorker::AddMessage(Message *message, const string &username, const string &from, string &err) {
    unsigned long lastId = LastMesID(username) + 1;
    bool isNameValid = false;
    isNameValid = CheckUser(username);
    if (isNameValid) {
        if (lastId > 0 && message != NULL) {
            time_t seconds = time(NULL);
            tm *timeinfo = localtime(&seconds);
            message->date_time = asctime(timeinfo);
            message->id = lastId;
            message->state = MSTATE_UNREAD;
            message->username = from;
            message->len = message->body.length();
            WriteToFile(username, message);
        }
    } else return 0;
    return lastId;
}

void ServerWorker::WriteToFile(const string &username, Message *message) {
    OpenSem(username);
    ofstream out(GetMessageFilePth(username).c_str(), ios_base::app);
    if (out.good() && message != NULL) {
        out << MES_ID << message->id << endl;
        out << MES_ADDR << message->username << endl;
        out << MES_DATE_TIME << message->date_time; // << endl;
        out << MES_LEN << message->len << endl;
        out << MES_STATE << MESSAGE_STATES[message->state] << endl;
        out << message->body << endl;

    }
    out.close();
    CloseSem(username);
}

bool ServerWorker::WriteMessages(const string &username, Message **m, const unsigned long &size, bool ioMode) {
    if (m != NULL && size >= 0) {
        ofstream out;
        if (ioMode) out.open(GetMessageFilePth(username).c_str(), ios_base::trunc);
        else out.open(GetMessageFilePth(username).c_str(), ios_base::app);
        if (out.good()) {
            if (size > 0)
                for (unsigned long i = 0; i < size; i++) {
                    if (m[i] != NULL) {
                        out << MES_ID << m[i][0].id << endl;
                        out << MES_ADDR << m[i][0].username << endl;
                        out << MES_DATE_TIME << m[i][0].date_time << endl;
                        out << MES_LEN << m[i][0].len << endl;
                        out << MES_STATE << m[i][0].state << endl;
                        out << m[i][0].body << endl;
                    }
                }
        } else {
            out.close();
            return false;
        }
        out.close();
        return true;
    }
    return false;
}

string ServerWorker::ShowUnreadMes(const string &username, unsigned long &cc, string *&buf) {
    int unread = 0;
    unsigned long size = 0;
    Message **mm = ReadAllMes(username, size);
    bool changes = false;
    if (size > 0) {
        for (unsigned long i = 0; i < size; i++) {
            if (mm[i] != NULL) {
                if (mm[i][0].state == MSTATE_UNREAD) {
                    unread++;
                }
            }
        }
        buf = new string[unread];
        cc = 0;
        for (unsigned int i = 0; i < size; i++) {
            if (mm[i][0].state == MSTATE_UNREAD) {
                buf[cc] = mm[i][0].Serialize();
                changes = true;
                mm[i][0].state = MSTATE_NORMAL;
                cc++;
            }
        }
        if (changes)
            WriteMessages(username, mm, size, true);
        for (unsigned long i = 0; i < size; i++) {
            if (mm[i] != NULL)
                delete mm[i];
        }
    }

    if (unread == 0)
        return "Error while showing unread the messages. No messages found.";
    return "";
}

string ServerWorker::ShowAllMes(const string &username, unsigned long &size, string *&buf) {
    Message **mm = ReadAllMes(username, size);
    bool changes = false;
    if (size > 0) {
        buf = new string[size];
        for (unsigned int i = 0; i < size; i++) {
            buf[i] = mm[i][0].Serialize();
            if (mm[i][0].state == MSTATE_UNREAD) {
                changes = true;
                mm[i][0].state = MSTATE_NORMAL;
            }
        }
        if (changes)
            WriteMessages(username, mm, size, true);
        for (unsigned long i = 0; i < size; i++) {
            if (mm[i] != NULL)
                delete mm[i];
        }
    } else
        return "Error while showing unread the messages. No messages found.";
    return "";
}

string ServerWorker::ShowExactMes(const string &username, string &buf, const string &mesNumber) {
    unsigned long size = 0;
    int mesId = atoi(mesNumber.c_str());
    Message **mm = ReadAllMes(username, size);
    bool changes = false;
    bool found = false;
    if (size > 0) {
        for (unsigned long i = 0; i < size; i++) {
            if (mm[i] != NULL) {
                if (mm[i][0].id == mesId) {
                    buf = mm[i][0].Serialize();
                    if (mm[i][0].state == MSTATE_UNREAD) {
                        mm[i][0].state = MSTATE_NORMAL;
                        changes = true;
                    }
                    found = true;
                    break;
                }
            }
        }
        if (changes)
            WriteMessages(username, mm, size, true);
        for (unsigned long i = 0; i < size; i++)
            if (mm[i] != NULL)
                delete mm[i];
    }
    if (!found)
        return "Error while showing the messages. No messages found.";
    return "";
}

string ServerWorker::DeleteMes(const string &username, const string &mesNumber) {
    unsigned long num = strtoul(mesNumber.c_str(), NULL, 10);
    if (num > 0)
        if (!DeleteOneMes(username, num))
            return "Internal error occured!";
    return "";
}

string ServerWorker::ResendMes(const string &from, const string &to, string &buf, const string &mesNumber) {
    unsigned long size = 0;
    string res;
    int mesId = atoi(mesNumber.c_str());
    Message **mm = ReadAllMes(from, size);
    if (size > 0) {
        for (unsigned long i = 0; i < size; i++) {
            if (mm[i] != NULL) {
                if (mm[i][0].id == mesId) {
                    mesId = AddMessage(mm[i], to, from, res);
                    if (mesId == 0) {
                        res = "Error while resending the messages. Aim user not found.";
                        break;
                    } else
                        buf = mm[i][0].Serialize();
                    break;
                }
            }
        }
        for (unsigned long i = 0; i < size; i++) {
            if (mm[i] != NULL) {
                delete mm[i];
            }
        }
    } else
        res = "Error while resending the messages. Message not found.";
    return res;
}

string ServerWorker::MessageToString(const Message &m) {
    stringstream res;
    res << "ID: " << m.id << "\n";

    res << "TIME: " << m.date_time << "\n";

    res << "FROM: " << m.username << "\n";

    res << "LEN: " << m.len << "\n";

    res << "STATE: " << m.state << "\n";

    res << "BODY: " << m.body << "\n";
    return res.str();
}

unsigned long ServerWorker::LastMesID(const string &username) {
    unsigned long res = 0;
    unsigned long size = 0;
    Message **buf = ReadAllMes(username, size);
    if (size > 0 && buf != NULL) {
        for (unsigned long i = 0; i < size; i++)
            if (buf[i] != NULL) {
                if (res < buf[i][0].id)
                    res = buf[i][0].id;
                delete buf[i];
            }
    }
    return res;
}

void ServerWorker::ReplaceBuf(string &buf, const string &s) {
    buf.replace(0, string(s).size(), "");
}

Message **ServerWorker::ReadAllMes(const string &username, unsigned long &res) {
    Message **mes = NULL;
    res = 0;
    string buf;
    long size = 0;
    ifstream inp(GetMessageFilePth(username).c_str());
    int state = 0;
    if (inp.good()) {
        while (!inp.eof()) {
            buf.clear();
            std::getline(inp, buf);
            if (buf.size() > 0)
                switch (state) {
                    case 0: // id
                        res++;
                        mes = (Message **) realloc(mes, res * sizeof(Message *));
                        mes[res - 1] = new Message();
                        ReplaceBuf(buf, MES_ID);
                        mes[res - 1][0].id = strtoul(buf.c_str(), NULL, 10);
                        printf("ID = %lu\n", mes[res - 1][0].id);
                        state++;
                        break;
                    case 1: // user name
                        ReplaceBuf(buf, MES_ADDR);
                        mes[res - 1][0].username = buf;
                        printf("USERNAME = %s\n", mes[res - 1][0].username.c_str());
                        state++;
                        break;
                    case 2: // date_time
                        ReplaceBuf(buf, MES_DATE_TIME);
                        mes[res - 1][0].date_time.append(buf);
                        printf("DATE = %s\n", mes[res - 1][0].date_time.c_str());
                        state++;
                        break;
                    case 3: // len
                        ReplaceBuf(buf, MES_LEN);
                        mes[res - 1][0].len = strtoul(buf.c_str(), NULL, 10);
                        size = mes[res - 1][0].len;
                        printf("SIZE = %lu\n", mes[res - 1][0].len);
                        state++;
                        break;
                    case 4: // state
                        ReplaceBuf(buf, MES_STATE);
                        mes[res - 1][0].state = strtoul(buf.c_str(), NULL, 10);
                        printf("STATE = %d\n", mes[res - 1][0].state);
                        state++;
                        break;
                    case 5: // body
                        mes[res - 1][0].body += buf;
                        mes[res - 1][0].body += "\n";
                        size -= buf.size() + 1;
                        if (size <= 0) {
                            state = 0;
                            printf("BODY = %s\n", mes[res - 1][0].body.c_str());
                        }
                        break;
                }
        }
    }
    inp.close();

    return mes;
}

Message *ServerWorker::ReadOneMes(const string &username, const unsigned long &id, bool &res) {
    Message *mes = NULL;
    res = false;
    string buf;
    long size = 0;
    ifstream inp(GetMessageFilePth(username).c_str());
    int state = 0;
    if (inp.good()) {
        while (!inp.eof()) {
            buf.clear();
            std::getline(inp, buf);
            if (buf.size() > 0)
                switch (state) {
                    case 0: // id
                        if (mes != NULL)
                            if (mes->id == id) {
                                res = true;
                                return mes;
                            }
                        delete mes;
                        mes = new Message();
                        ReplaceBuf(buf, MES_ID);
                        mes->id = strtoul(buf.c_str(), NULL, 10);
                        printf("ID = %lu\n", mes->id);
                        state++;
                        break;
                    case 1: // user name
                        ReplaceBuf(buf, MES_ADDR);
                        mes->username = buf;
                        printf("USERNAME = %s\n", mes->username.c_str());
                        state++;
                        break;
                    case 2: // date_time
                        ReplaceBuf(buf, MES_DATE_TIME);
                        mes->date_time = buf;
                        printf("DATE = %s\n", mes->date_time.c_str());
                        state++;
                        break;
                    case 3: // len
                        ReplaceBuf(buf, MES_LEN);
                        mes->len = strtoul(buf.c_str(), NULL, 10);
                        size = mes->len;
                        printf("SIZE = %lu\n", mes->len);
                        state++;
                        break;
                    case 4: // state
                        ReplaceBuf(buf, MES_STATE);
                        mes->state = strtoul(buf.c_str(), NULL, 10);
                        printf("STATE = %d\n", mes->state);
                        state++;
                        break;
                    case 5: // body
                        mes->body += buf;
                        mes->body += "\n";
                        size -= buf.size() + 1;
                        if (size <= 0) {
                            state = 0;
                            printf("BODY = %s\n", mes->body.c_str());
                        }
                        break;
                }
        }
    }
    inp.close();
    if (mes != NULL) {
        if (mes->id == id) {
            res = true;
            return mes;
        } else
            delete mes;
    }
    return NULL;
}

bool ServerWorker::DeleteOneMes(const string &username, const unsigned long &id) {
    bool res = false;
    unsigned long size = 0;
    Message **buf = ReadAllMes(username, size);
    if (size > 0 && buf != NULL) {
        for (unsigned long i = 0; i < size; i++)
            if (buf[i] != NULL)
                if (buf[i][0].id == id) {
                    for (unsigned long t = i; t < size - 1; t++) {
                        buf[t] = buf[t + 1];
                    }
                    res = true;
                    break;
                }
        if (res == true) {
            size--;
            if (size > 0) {
                buf = (Message **) realloc(buf, (size) * sizeof(Message *));
                if (buf == NULL)
                    res = false;
            }
        }
    }
    if (res)
        res = WriteMessages(username, buf, size, true);
    for (unsigned long i = 0; i < size; i++) {
        if (buf[i] != NULL) {
            delete buf[i];
        }
    }
    return res;
}

bool ServerWorker::ListenRecv(char *&MsgStr) {
    char c[BUFSIZE];
    unsigned int size = 0;
    int res = recv(socket, c, BUFSIZE, 0);
    if (res == BUFSIZE) {
        size = atoi(c);

        char* recvbuf = new char[size];
        int res = recv(socket, recvbuf, size, 0);
        printf("String received: %s\n", recvbuf);
        if (res > 0) {
            int count = 0;
            for (int i = 0; i < res; i++)
                if (recvbuf[i] != '\r' && recvbuf[i] != '\0')
                    count++;
            MsgStr = new char[count];
            count = 0;
            for (int i = 0; i < res; i++)
                if (recvbuf[i] != '\r' && recvbuf[i] != '\0')
                    MsgStr[count++] = recvbuf[i];

        }
        delete recvbuf;
    } else return false;
    return true;
}

void ServerWorker::SendTo(const char *message) {
    if (message != NULL) {
        int res = 0;
        int size = strlen(message);
        char *ss = new char[size + BUFSIZE];
        char *sizeBuf = new char[BUFSIZE];
        snprintf(sizeBuf, BUFSIZE, "%d", size);
        printf("SB = %s\n", sizeBuf);
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
        delete[] ss;
    } else
        printf("Failed to send. Received NULL message.");
}


string ServerWorker::Serialize(STATE opcode, unsigned short numarg, const string *ss) {
    stringstream sstr;
    sstr << (int) opcode << DELIM_PARSE << (int) numarg << DELIM_PARSE;
    if (numarg > 0 && ss != NULL)
        for (int i = 0; i <= numarg - 1; i++) {
            string temp = ss[i];
            std::replace(temp.begin(), temp.end(), DELIM_PARSE, ' ');
            sstr << temp << DELIM_PARSE;
        }
    return sstr.str();
}

STATE ServerWorker::Parse(const string &input, unsigned short &numarg, string *&args) {
    STATE res = NO_OPERATION;
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

STATE ServerWorker::ParseOpCode(const string &buf) {
    int res = atoi(buf.c_str());
    STATE st = NO_OPERATION;
    for (int i = 0; i < API_SIZE; i++)
        if (res == i)
            return static_cast<STATE>(i);

    return st;
}

void ServerWorker::CloseSocket() {
    SendTo("Closing connection.");
    if (closesocket(socket) == -1)
        printf("Socket #%d close failed\n", socket);
}