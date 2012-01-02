#include "main.h"

using namespace std;

// The main entrypoint to our program, let the account management begin!
int main( int argc, char *argv[] )
{
	// We create the manager and root client on the stack
	// Their constructors get called here
	AccountManager manager;
	Client root;

	// We create the account into our system so we can login :)
	manager.CreateAccount( "hrla", "lol" );

	Account *myacc = manager.Login( root, "hrla", "lol" );	// works
	Account *myacc2 = manager.Login( root, "hrla", "troll" );	// fails, returns NULL

	cout << "Hello world!";

	return EXIT_SUCCESS;

	// Now that we returned, the destructors of manager and root get called!
}

/*
	So that is the account management system we created!
	It is very basic; you usually do not create manager in the stack as you want the whole program to have global access to it
	(Creating it in the program memory is better.)

	Feel free to ask and expand this system! I am hyped to see your extension of Client to print "Hello, hrla!" :P
*/
