#ifndef API_H
#define API_H

#include <string>
#include <algorithm>

using namespace std;

const short STRING_BUFFER_SIZE = 1024;

static const int API_SIZE = 18;

static const string API [API_SIZE] = 
{
	"Yes",
	"Noop",
	"Answer",
	"Start",
	"Init",
	"Operation Code",
	"Exit",
	"Register",
	"Login",
	"Logout",
	"Send mes",
	"Delete user",
	"Delete mes",
	"Show unread",
	"Show all",
	"Show exact",
	"Resend",
	"In system"
};
/// !!! MANAGE ALL CHANGES AT ServerWorker::parseOpCode

enum STATE
{
	SERV_OK,
	NO_OPERATION,
	ANSWER,
	START,
	INIT,
	OPCODE,
	EXIT,
	REG,
	LOG,
	LOGOUT,
        SND,
	DEL_US,
	DEL_MSG,
	SH_UNR,
	SH_ALL,
	SH_EX,
	RSND,
	INSYS
};

#endif