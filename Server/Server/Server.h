#pragma once
#include "Libs.h"

typedef struct server_t* Server;

/* serverCreate:
	Create a server, bind it to the ip and port, and listen.
	Anytime the server accept a new client connection request it's create a ClientListener that start gettig data from it.

	@param: ip - if NULL so ip will be defined DEFAULT_IP.
	@param: ip - if NULL so port will be defined DEFAULT_port

	@return: server or NULL in case of failure
*/
Server serverCreate(const char* ip, const char* port);

Result serverRun(Server server);

Result serverPrint(Server server);

Result serverCleanDoneClients(Server server);

int serverGetClientCounter(Server server);

int serverGetNumberOfActiveClient(Server server);


/* serverDestroy:
	disconnect and free all memory
*/
void serverDestroy(Server);




// === for debug ===
void printClients(Server server);
