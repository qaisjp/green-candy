#include "main.h"

AccountManager::AccountManager()
{
}

AccountManager::~AccountManager()
{
}

Account& AccountManager::CreateAccount( const std::string& name, const std::string& password )
{
	Account acc( name, password );

	m_accounts.push_back( acc );
	return acc;
}