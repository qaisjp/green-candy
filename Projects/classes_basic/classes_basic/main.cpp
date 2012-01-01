#include "main.h"

int* wrong()
{
	int num = 1;

	return &num;
}

int& correct()
{
	int num = 1;
	return num;
}

int main( int argc, char *argv[] )
{
	AccountManager manager;

	int data = *wrong();
	int data2 = correct();
}