#pragma once
#include "Libs.h"

typedef struct client_t* Client;

/* clientCreate:
	Create a Client, and connect it to a server.

	@param: ip - if NULL so ip will be defined DEFAULT_IP.
	@param: port - if NULL so port will be defined DEFAULT_port
	@param: path - the path for the data file

	@return: client or NULL in case of failure
*/
Client clientCreate(const char* ip, const char* port, const char* path);

Result clientRun(Client client);

void clientDestroy(Client client);

