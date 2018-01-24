#include <algorithm>
#include <direct.h>
#include "ServerWorker.h"

ServerWorker::ServerWorker(CTcpServer * parent): pParent(parent){
}

ServerWorker::~ServerWorker() = default;

void ServerWorker::Init(SOCKET s) {
    socket = s;
}

bool ServerWorker::MainLoop() {
    STATE State = NO_OPERATION;
    string MsgStr;
    string currentUserName;
    string errMessage;
    int mesId;
    bool RegisterState = false, LoginState = false;
    std::vector<std::string> args;
    std::vector<std::string> args2;
    std::vector<char> resBuf;
    const string servOk = std::to_string(SERV_OK);
    const string noOp = std::to_string(NO_OPERATION);

    unsigned long mesCount = 0;
    Message *m = NULL;

    while (true) {
        bool res = ListenRecv(resBuf);
        if (!res)
            return false;
        args2.clear();
        State = Parse(resBuf, args);
        switch (State) {
            case START:
                args2.emplace_back(servOk);
                break;
            case EXIT:
                args2.emplace_back(servOk);
                printf("Client with ID: %d is disconnect!\n", socket);
                return true;
            case REG:
                if (args.size() > 1) {
                    errMessage = RegisterNewUser(args[0], args[1], RegisterState);
                    if (RegisterState) {
                        args2.emplace_back(servOk);
                    } else {
                        args2.emplace_back(noOp);
                        args2.emplace_back(errMessage);
                    }
                } else {
                    args2.emplace_back(noOp);
                    args2.emplace_back("Not valid args.");
                }
                break;
            case LOG:
                if (args.size() > 1) {
                    errMessage = LoginNewUser(args[0], args[1], LoginState);
                    if (LoginState) {
                        args2.emplace_back(servOk);
                        currentUserName = args[0];
                    } else {
                        args2.emplace_back(noOp);
                        args2.emplace_back(errMessage);
                    }
                } else {
                    args2.emplace_back(noOp);
                    args2.emplace_back("Not valid args.");
                }
                break;
            case LOGOUT:
                cout << "Logging out." << endl;
                currentUserName = "";
                args2.emplace_back(servOk);
                break;
            case SND:
                cout << "Sending the message." << endl;
                if (args.size() > 1) {
                    m = new Message();
                    if (m->Deserialize(args[1]) && !args[0].empty()) {
                        if (!currentUserName.empty())
                            mesId = AddMessage(m, args[0], currentUserName, errMessage);
                        else { mesId = 0; }
                        if (mesId == 0) {
                            args2.emplace_back(noOp);
                            args2[1] = "Error while sending the message [" + errMessage + "]";
                        } else {
                            m->body = "";
                            args2.emplace_back(servOk);
                            args2.emplace_back(m->Serialize());
                        }
                    }
                    delete m;
                }
                break;
            case DEL_US:
                cout << "Deleting user." << endl;
                if (!currentUserName.empty()) {
                    errMessage = DeleteUser(currentUserName);
                    if (errMessage.empty()) {
                        args2.emplace_back(servOk);
                        args2.emplace_back(currentUserName);
                    } else {
                        args2.emplace_back(noOp);
                        args2.emplace_back(errMessage);
                    }
                    currentUserName = "";
                }
                break;
            case DEL_MSG:
                cout << "Deleting message." << endl;
                if (!args.empty()) {
                    if (!currentUserName.empty()) {
                        errMessage = DeleteMes(currentUserName, args[0]);
                        if (errMessage.empty()) {
                            args2.emplace_back(servOk);
                            args2.emplace_back("");
                        } else {
                            args2.emplace_back(noOp);
                            args2.emplace_back(errMessage);
                        }
                    }
                }
                break;
            case SH_UNR:;
                cout << "Showing unread messages." << endl;
                if (!currentUserName.empty()) {
                    errMessage = ShowUnreadMes(currentUserName, mesCount, args);
                    if (errMessage.empty()) {
                        args2.emplace_back(servOk);
                        if (mesCount > 0) {
                            for (int i = 0; i < mesCount; i++)
                                args2.emplace_back(args[i]);
                        }
                    } else {
                        args2.emplace_back(noOp);
                        args2.emplace_back(errMessage);
                    }
                }
                break;
            case SH_ALL:;
                cout << "Showing all messages." << endl;
                if (!currentUserName.empty()) {
                    errMessage = ShowAllMes(currentUserName, mesCount, args);
                    if (errMessage.empty()) {
                        args2.emplace_back(servOk);
                        if (mesCount > 0) {
                            for (int i = 0; i < mesCount; i++)
                                args2.emplace_back(args[i]);
                        }
                    } else {
                        args2.emplace_back(noOp);
                        args2.emplace_back(errMessage);
                    }
                }
                break;
            case SH_EX:
                cout << "Showing the exact message." << endl;
                if (args.size() > 1) {
                    if (!currentUserName.empty()) {
                        errMessage = ShowExactMes(currentUserName, args2[1], args[0]);
                        if (errMessage.empty()) {
                            args2.emplace_back(servOk);
                        } else {
                            args2.emplace_back(noOp);
                            args2.emplace_back(errMessage);
                        }
                    }
                }
                break;
            case RSND:
                cout << "Resending the exact message." << endl;
                if (args.size() > 1) {
                    if (!currentUserName.empty()) {
                        errMessage = ResendMes(currentUserName, args[1], args2[1], args[0]);
                        if (errMessage.empty()) {
                            args2.emplace_back(servOk);
                        } else {
                            args2.emplace_back(noOp);
                            args2.emplace_back(errMessage);
                        }
                    }
                }
                break;
            default:
                cout << "Unknown state: " << State << endl;
                break;
        }
        auto &&serialized = Serialize(ANSWER, args2);
        SendTo(serialized);
    }
    return true;
}
//
//void ServerWorker::OpenSem(const string &name) {
//	pParent->LockClient(name);
//}
//
//void ServerWorker::CloseSem(const string &name) {
//	pParent->UnlockClient(name);
//}

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
//
//void check_user(std::string& pth, std::string& pass2) {
//	ifstream fin(pth.c_str());
//	if (fin.good())
//		fin >> pass2;
//	fin.close();
//}

bool ServerWorker::CheckUser(const string &name) {
    string pth = GetPasswFilePth(name);
    string pass2;
	//WithLock<bool>(name, [&]() {check_user(pth, pass2); return true; });
	ifstream fin(pth.c_str());
	if (fin.good())
		fin >> pass2;
	fin.close();
    return pass2.size() > 0;
}

//
//std::string register_new_user(const string &uname,std::ifstream& fin, const string &passw) {
//	int stat;
//	if (fin.good()) {
//		printf("ERROR: User already exists.\n");
//		return "Username is already used by another user. Please, choose other option for username.\n";
//	}
//	string pth = "./users/";
//	_mkdir(pth.c_str());
//	pth += uname;
//
//	stat = _mkdir(pth.c_str());
//	if (stat != 0) {
//		printf("ERROR: Failed to create dir. ErrCode = %d\n", stat);
//		return "Internal server issue. Please, try again.\n";
//	}
//	printf("Dir %s created successfully.\n", uname.c_str());
//	pth += PASSW_FILE;
//	ofstream out(pth.c_str());
//	if (!out.good()) {
//		printf("ERROR: Password is not saved.\n");
//		return "ERROR: Password is not saved.\n";
//	}
//	out << passw;
//	out.close();
//	return "";
//}

string ServerWorker::RegisterNewUser(const string &uname, const string &passw, bool &res) {
    int stat;
    res = false;
    if (uname.length() > 0 && passw.length() > 0) {

        ifstream fin(GetPasswFilePth(uname).c_str());
		//auto&& result = WithLock<std::string>(uname, [&]() {return register_new_user(uname, fin, passw); });
		int stat;
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
		//if (!result.empty()) return result;
        res = true;
        ofstream mFile(GetMessageFilePth(uname).c_str());
        mFile.close();
    }
    return "";
}


//
//std::string login_user(const string &uname, const string &passw, std::string& pth, bool &res) {
//	ifstream fin(pth.c_str());
//	string pass2;
//
//	if (!fin.good()) {
//		printf("ERROR: Could not load file %s.\n", pth.c_str());
//		return "Internal server issue. Please, try again.\n";
//	}
//	fin >> pass2;
//	if (pass2.compare(passw) != 0) {
//		printf("ERROR: Password is not correct or there is no access to the pass. path = \"%s\"", pth.c_str());
//		return "Internal server issue. Please, try again.\n";
//	}
//	res = true;
//	printf("Successfully logged in! User: %s\n", uname.c_str());
//	fin.close();
//}

string ServerWorker::LoginNewUser(const string &uname, const string &passw, bool &res) {
    res = false;
    if (uname.length() > 0 && passw.length() > 0) {
        string pth = GetPasswFilePth(uname);
		//return WithLock<std::string>(uname, [&]() {return login_user(uname, passw, pth, res); });
		ifstream fin(pth.c_str());
		string pass2;

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
	}
    return "";
}

string ServerWorker::DeleteUser(const string &username) {
    string buf;
    time_t seconds = time(NULL);
    tm timeinfo;
    localtime_s(&timeinfo, &seconds);

    buf.append(asctime(&timeinfo));
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
            tm timeinfo;
            localtime_s(&timeinfo, &seconds);
            message->date_time = asctime(&timeinfo);
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
	//WithLock<bool>(username, [&]() {
	//	ofstream out(GetMessageFilePth(username).c_str(), ios_base::app);
	//if (out.good() && message != NULL) {
	//	out << MES_ID << message->id << endl;
	//	out << MES_ADDR << message->username << endl;
	//	out << MES_DATE_TIME << message->date_time; // << endl;
	//	out << MES_LEN << message->len << endl;
	//	out << MES_STATE << MESSAGE_STATES[message->state] << endl;
	//	out << message->body << endl;
	//
	//}
	//out.close();
	//return true;
	//});
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

string ServerWorker::ShowUnreadMes(const string &username, unsigned long &cc, std::vector<std::string> &buf) {
    int unread = 0;
    unsigned long size = 0;
    Message **mm = ReadAllMes(username, size);
    bool changes = false;
    buf.clear();
    if (size > 0) {
        for (unsigned long i = 0; i < size; i++) {
            if (mm[i] != nullptr) {
                if (mm[i][0].state == MSTATE_UNREAD) {
                    unread++;
                }
            }
        }
        cc = 0;
        for (unsigned int i = 0; i < size; i++) {
            if (mm[i][0].state == MSTATE_UNREAD) {
                buf.emplace_back(mm[i][0].Serialize());
                changes = true;
                mm[i][0].state = MSTATE_NORMAL;
                cc++;
            }
        }
        if (changes)
            WriteMessages(username, mm, size, true);
        for (unsigned long i = 0; i < size; i++) {
            if (mm[i] != nullptr)
                delete mm[i];
        }
    }

    if (unread == 0)
        return "Error while showing unread the messages. No messages found.";
    return "";
}

string ServerWorker::ShowAllMes(const string &username, unsigned long &size, vector<string> &buf) {
    Message **mm = ReadAllMes(username, size);
    bool changes = false;
    buf.clear();
    if (size > 0) {
        for (unsigned int i = 0; i < size; i++) {
            buf.emplace_back(mm[i][0].Serialize());
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
        if (res) {
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

bool ServerWorker::ListenRecv(vector<char> &MsgStr) {
    return __ListenRecv(socket, MsgStr);
}

void ServerWorker::SendTo(string &message) {
    sendToSocket(socket, message);
}


string ServerWorker::Serialize(STATE opcode, std::vector<std::string> &args) {
    return __Serialize(opcode, args);
}

STATE ServerWorker::Parse(std::vector<char> &input, std::vector<std::string> &args) {
    return __Parse(input, args);
}

STATE ServerWorker::ParseOpCode(const std::string &buf) {
    return __ParseOpCode(buf);
}

void ServerWorker::CloseSocket() {
    auto &&message = std::string("Closing connection.");
    SendTo(message);
    if (closesocket(socket) == -1)
        printf("Socket #%d close failed\n", socket);
}