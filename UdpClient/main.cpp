#include "ClientWorker.h"

int main(int argc, char** argv)
{
	std::cout << "Enter <host>:<port> of Mail Server: " << std::endl;
	std::string s;
	std::cin >> s;
    ClientWorker f;
    f.StartThread(s);
	std::cout << "Mail client terminated!" << std::endl;
	return 0;
}