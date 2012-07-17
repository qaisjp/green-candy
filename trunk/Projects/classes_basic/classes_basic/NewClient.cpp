#include "main.h"

void NewClient::OnLogin( Account& myacc )
{
    Client::OnLogin( myacc );
    
    throw 1;
}

void NewClient::OnLogout()
{
    Client::OnLogout();
}