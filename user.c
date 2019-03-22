#include <stdlib.h>
#include <errno.h>
#include <netdb.h>
#include <string.h>

struct User{
	char name[15];
	char ipAddress[13];
};	//puerto
	//cp

struct UserSocket{
	char name[15];
	int sockNumber;
};


