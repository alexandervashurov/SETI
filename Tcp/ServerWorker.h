#pragma once

#ifndef SERVERWORKER_H
#define SERVERWORKER_H

#include "stdinc.h"
#include "API.h"
#include "Message.h"
#include "SharedFunctions.h"
#include "CTcpServer.h"

#include <unordered_map>

#define USERS_FOLDER "./users/"
#define MESSAGE_FILE "/messages"
#define PASSW_FILE   "/password"

typedef unsigned short  USHORT;

using namespace std;

class CTcpServer;

class ServerWorker
{
public:
    explicit ServerWorker(CTcpServer* parent);
    ~ServerWorker();
    
    void Init(SOCKET ClientSocket);
    bool MainLoop();

	void SendTo(string &message);
    string GetPasswFilePth(const string& username);
    string GetMessageFilePth(const string& username);
    string LoginNewUser(const string &uname, const string &passw,  bool &res);
	string RegisterNewUser(const string &uname, const string &passw,  bool &res);
    string DeleteUser(const string& username);
	/*template<typename R,
		typename F,
		class... _Types> inline
		R WithLock(const std::string& key, F fun, _Types&&... _Args) {
		OpenSem(key);
		R result;
		try {
			result = fun(_Args...);
		}
		catch (std::exception &ex) {
			std::cerr << ex.what() << std::endl;
		}
		CloseSem(key);
		return result;
	}*/
    unsigned long AddMessage(Message* message, const string& username, const string& from, string& err);
    string ShowUnreadMes(const string& username, unsigned long &mesCount,std::vector<std::string> &buf);
    string ShowAllMes(const string &username, unsigned long &mesCount, vector<string> &buf);
    string ShowExactMes(const string& username, string& buf, const string& mesNumber);
    string DeleteMes(const string& username, const string& mesNumber);
    string ResendMes(const string& from, const string& to, string& buf, const string& mesNumber);
    string MessageToString(const Message& m);
    Message** ReadAllMes(const string& username, unsigned long& size);
    Message* ReadOneMes(const string& username, const unsigned long& id, bool& res);
    bool DeleteOneMes(const string& username, const unsigned long& id);
    bool WriteMessages(const string& username, Message** m, const unsigned long& size, bool ioMode);
    bool CheckUser(const string& name);
    unsigned long LastMesID(const string& username);
    STATE ParseOpCode(const string& buf);
    string Serialize(STATE opcode, std::vector<std::string> &args);
    STATE Parse(std::vector<char> &input, std::vector<std::string> &args);
    
    void ReplaceBuf(string& buf, const string& s);
    
    void WriteToFile(const string& username, Message* message);    
    /*
    void OpenSem(const string& name);
    void CloseSem(const string& name);*/
    
    bool ListenRecv(vector<char> &MsgStr);
    void CloseSocket();
	
	SOCKET socket;
	CTcpServer *pParent;

};


#endif /* SEVERWORKER_H */

