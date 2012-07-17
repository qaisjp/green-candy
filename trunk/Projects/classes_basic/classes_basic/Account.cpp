#include "main.h"

// Now we have to initialize the account data segment
Account::Account( const std::string& name, const std::string& password )
{
	m_name = name;
	m_password = password;

	// Make sure we do not have a client logged in to begin with
	m_client = NULL;
}

// On destruction, we logout any existing clients, so they successfully terminate their session!
Account::~Account()
{
    std::cout << "Terminating account...\n";

    Logout();
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

    // Notify the client of the operation!
    user.OnLogin( *this );
	return true;
}

void Account::Logout()
{
	// Terminate the client D:
	// Come to think of it, how about you expand the client class to notify events? OnLogin and OnLogout?
	m_client = NULL;
}