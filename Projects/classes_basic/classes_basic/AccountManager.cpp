#include "main.h"

// Dummy constructor and destructor, we do not need to do anything here
AccountManager::AccountManager()
{
}

AccountManager::~AccountManager()
{
	// Destroy the accounts and clear the heap memory
	accountList_t::iterator iter = m_accounts.begin();

	for ( ; iter != m_accounts.end(); iter++ )
		delete *iter;
}

Account* AccountManager::CreateAccount( const std::string& name, const std::string& password )
{
	// We do not want double accounts
	if ( Exists( name ) )
		return NULL;

	// Construct the account on the heap ;)
	Account *acc = new Account( name, password );	// declaring objects is legal everywhere in C++!

	// Add it to the live accounts list
	m_accounts.push_back( acc );
	return acc;
}

Account* AccountManager::Login( Client& user, const std::string& name, const std::string& password )
{
	// This is an iterator to browse through the STL vector
	// Iterators are used to jump from one data segment to the other safely!
	accountList_t::iterator iter = m_accounts.begin();

	// Iterate through the STL vector
	for ( ; iter != m_accounts.end(); iter++ )
	{
		// Does the username match?
		if ( (*iter)->GetName() != name ) 
			continue;

		// Does the password match of the account? If not, we know that there will not be any second account with the same name
		if ( (*iter)->GetPassword() != password )
			return NULL;

		// Try to login
		if ( !(*iter)->Login( user ) )
			return NULL;

		return *iter;
	}

	// We did not find any account with the matching name, woops!
	return NULL;
}

bool AccountManager::Exists( const std::string& name )
{
	accountList_t::iterator iter = m_accounts.begin();

	// Iterate through the vector and see if we find the matching account
	for ( ; iter != m_accounts.end(); iter++ )
	{
		// Username check, if true, we have our account :D
		if ( (*iter)->GetName() == name )
			return true;
	}

	// If all accounts do not match, tell it the program
	return false;
}