#include "main.h"


void Client::OnLogin(class Account& acc)
{
	std::cout << "Successfully logged in!\n";
}

void Client::OnLogout()
{
    std::cout << "Logged out\n";
}