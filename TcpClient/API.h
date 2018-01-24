#pragma once
#include <string>
#include <algorithm>

const std::string OPENT = "Enter your option: ";

static const int API_SIZE = 18;

static const std::string API [API_SIZE] =
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
