#include "main.h"

void NewClient::OnLogin( Account& myacc )
{
    Client::OnLogin( myacc );
    
}

void NewClient::OnLogout()
{
    Client::OnLogout();
}