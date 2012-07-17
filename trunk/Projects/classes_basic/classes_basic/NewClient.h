#ifndef _NEW_CLIENT_
#define _NEW_CLIENT_

class NewClient : public Client
{
public:
	void		    OnLogin( class Account& acc );
	void		    OnLogout();
};

#endif //_NEW_CLIENT_