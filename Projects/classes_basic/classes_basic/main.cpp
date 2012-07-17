#include "main.h"

using namespace std;

// The main entrypoint to our program, let the account management begin!
int main( int argc, char *argv[] )
{
    cout << "Hello guy! You have to login to proceed.\nUsername: ";

    std::string username;
    cin >> username;

    cout << "Password: ";

    std::string password;
    cin >> password;

	// We create the manager and root client on the stack
	// Their constructors get called here
	AccountManager manager;
	NewClient root;

	// We create the account into our system so we can login :)
	manager.CreateAccount( "hrla", "lol" );

    try
    {
	    Account *myacc = manager.Login( root, username, password );
    }
    catch( Rock& errObj )
    {
        cout << "We encountered an exception :(\n";
        cout << errObj.GetErrorInfo();
        cout << "\n";

        throw;
    }

	return EXIT_SUCCESS;

	// Now that we returned, the destructors of manager and root get called!
}

/*
	So that is the account management system we created!
	It is very basic; you usually do not create manager in the stack as you want the whole program to have global access to it
	(Creating it in the program memory is better.)

	Feel free to ask and expand this system! I am hyped to see your extension of Client to print "Hello, hrla!" :P
*/
