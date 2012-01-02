#include "main.h"

// Now we have to initialize the account data segment
Account::Account( const std::string& name, const std::string& password )
{
	m_name = name;
	m_password = password;

	// Make sure we do not have a client logged in to begin with
	m_client = NULL;
}

// Practically we do not need to destroy the account, it would be cool if you added a way to cleanup the account from the manager if destroyed
// Account destruction does not make sense in the long run anyways; you can decide.
Account::~Account()
{
}

// Public members to allow constant access to username
const std::string& Account::GetName()
{
	return m_name;
}

// ... and password!
const std::string& Account::GetPassword()
{
	return m_password;
}

// Basic login routine
bool Account::Login( Client& user )
{
	// Only one client can use an account at a time
	if ( IsBeingUsed() )
		return false;

	// Store the logged in client for future usage
	m_client = &user;
	return true;
}

void Account::Logout()
{
	// Terminate the client D:
	// Come to think of it, how about you expand the client class to notify events? OnLogin and OnLogout?
	m_client = NULL;
}