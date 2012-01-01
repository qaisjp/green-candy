#ifndef _MAIN_
#define _MAIN_

#include <string>
#include <list>

class Account
{
	friend class AccountManager;
public:
	const std::string&			GetName();

protected:
	const std::string&			GetPassword();

	std::string					m_name;
	std::string					m_password;
};

class AccountManager
{
public:
								AccountManager();
								~AccountManager();

	Account*					CreateAccount( const std::string& name, const std::string& password );

private:
	typedef std::list <Account> accountList_t;

	accountList_t				m_accounts;
};

#endif //_MAIN_