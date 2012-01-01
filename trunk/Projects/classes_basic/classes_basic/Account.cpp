#include "main.h"

Account::Account( const std::string& name, const std::string& password )
{
	m_name = name;
	m_password = password;
}

Account::~Account()
{
}

const std::string& Account::GetName()
{
	return m_name;
}

const std::string& Account::GetPassword()
{
	return m_password;
}