// This is a security macro to make sure that this header gets included into the program only once
// We will be adding "main.h" into every code file.
#ifndef _MAIN_
#define _MAIN_

// Include basic libraries we are using
#include <string>
#include <list>
#include <vector>
#include <sstream>
#include <iostream>

// Simple client represenation for our account system!
// In our example we create the root client; you know, that is an all access client on linux
// Every account system should have a empowered client (like in your system it is the localhost) and other sub clients like guests.
// You could try to add access levels to this, like root, guest, admin, moderator...
class Client
{
public:
	enum eAccess
	{
		root,
		guest,
		admin,
		moderator
	};
	// You could implement this; try to print the username of which account you logged into!
		void				OnLogin( class Account& acc );
		void				OnLogout();

	// Dummy data.
	int stuff;
};

// An Account.
class Account
{
	// We want the manager to have full access to the accounts
	friend class AccountManager;

								Account( const std::string& name, const std::string& password );
								~Account();
public:
	const std::string&			GetName();

	// We check whether an client is already logged in and log him out if we want to
	// Basically a client should be able to log himself out without the need of account manager access!
	// You might want to add security to that; by taking like the client as argument and checking whether it matches
	// But then again you do not give away your account to bad people, do you?
	bool						IsBeingUsed()	{ return m_client != NULL; }
	void						Logout();

protected:
	// Only the manager (the authorized instance) can access the password; the user has to guess or know it
	const std::string&			GetPassword();

	// Loggin in is also restricted to the manager; else you could log in without a password! D:
	bool						Login( Client& user );

	std::string					m_name;
	std::string					m_password;

	Client*						m_client;
};

class AccountManager
{
public:
								AccountManager();
								~AccountManager();

	// Create an account for usage; everyone should be able to do that, so it is public!
	Account*					CreateAccount( const std::string& name, const std::string& password );
	
	// We want to avoid double accounts
	bool						Exists( const std::string& name );

	// Everyone should also be able to log in; so we provide the AccountManager as interface to do so
	Account*					Login( Client& user, const std::string& name, const std::string& password );

private:
	// Read about the STL library, the standard C++ library; it has alot of useful tools to help you
	// One example is the std::vector; instead of reallocating or managing the memory yourself, the vector does that automatically
	// It extends in size once it needs to: In C you had to do that yourself...
	typedef std::vector <Account*> accountList_t;

	// Here we store our accounts
	accountList_t				m_accounts;
};

#endif //_MAIN_