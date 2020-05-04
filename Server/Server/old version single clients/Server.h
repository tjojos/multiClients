#pragma once

typedef struct server_t* Server;
typedef enum {
	SERVER_SUCCESS,
	SERVER_DONE,
	SERVER_ERR_NULL_PTR,
	SERVER_ERR_COMMU
}ServerResult;

/* serverCreate:
	Create a server, bind it to the ip and port, and listen.
	Anytime the server accept a new client connection request it's create a ClientListener that start gettig data from it.

	@param: ip - if NULL so ip will be defined DEFAULT_IP.
	@param: ip - if NULL so port will be defined DEFAULT_port

	@return: server or NULL in case of failure
*/
Server serverCreate(const char* ip, const char* port);

/* serverGetNextPacket
	Get the next packet that had been entered
	Note that the current architecture will choose to switch bitween clients rather than focus on one
	@param buffer : the destination which in the new data will be stored
	@param incomeSize : number of bytes that had been received
*/
ServerResult serverGetNextPacket(Server server, char* buffer, int bufferLen, int* incomeSize);

/* serverDestroy:
	disconnect and free all memory
*/
void serverDestroy(Server);