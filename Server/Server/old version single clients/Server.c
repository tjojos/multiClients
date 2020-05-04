#include "Server.h"
#include<stdlib.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <windows.h>
#include <tchar.h>
#include <iphlpapi.h>
#include <time.h> 
#include <stdbool.h>

#define DEFAULT_BUFLEN 512
#define BUF_SIZE 255

#define DEFAULT_IP "127.0.0.1"
#define DEFAULT_PORT "27015"

//Private functions declarations
ServerResult serverInit(Server);
ServerResult serverBind(Server);
ServerResult serverListen(Server);
ServerResult serverAccept(Server);


ServerResult serverDisconnect(Server);
void serverCleanup(Server);


struct server_t {
	WSADATA wsaData;
	struct addrinfo* result, hints;

	SOCKET listenSocket;

	SOCKET clientSocket;

	int size;
	char buffer[DEFAULT_BUFLEN];
};


// === API functions ===

Server serverCreate(const char* ip, const char* port) {
	Server server = malloc(sizeof(*server));

	if (!server)
		return NULL;

	if (!ip)
		ip = DEFAULT_IP;

	if (!port)
		port = DEFAULT_PORT;

	if (serverInit(server) != SERVER_SUCCESS) {
		return NULL;
	}


	int iResult = getaddrinfo(ip, port, &server->hints, &server->result);
	if (iResult != 0) {
		serverCleanup(server);
		return NULL;
	}

	// Create a SOCKET for the server to listen for client connections
	server->listenSocket = socket(server->result->ai_family, server->result->ai_socktype, server->result->ai_protocol);

	if (server->listenSocket == INVALID_SOCKET) {
		serverCleanup(server);
		return NULL;
	}


	printf("Server created\n");

	ServerResult res = serverBind(server);
	if (res != SERVER_SUCCESS) {
		serverCleanup(server);
		return NULL;
	}

	res = serverListen(server);
	if (res != SERVER_SUCCESS) {
		serverCleanup(server);
		return NULL;
	}

	printf("Server launched\n");

	res = serverAccept(server);
	if (res != SERVER_SUCCESS) {
		serverCleanup(server);
		return NULL;
	}

	return server;
}


ServerResult serverGetNextPacket(Server server, char* buffer, int bufferLen, int* incomeSize) {
	if (!server || !buffer)
		return SERVER_ERR_NULL_PTR;

	ZeroMemory(buffer, bufferLen);
	int bytesReceived;
	bytesReceived = recv(server->clientSocket, buffer, bufferLen, 0);

	if (incomeSize != NULL)
		*incomeSize = bytesReceived;
	
	if (bytesReceived > 0)
		return SERVER_SUCCESS;

	if (bytesReceived == 0)
		return SERVER_DONE;

	return SERVER_ERR_COMMU;
}

void serverDestroy(Server server) {
	if (!server)
		return;


	serverDisconnect(server);
	printf("Server disconnected\n");

	serverCleanup(server);
	free(server);
}







// === C'tor sequence ===
ServerResult serverInit(Server server) {
	if (!server)
		return SERVER_ERR_NULL_PTR;

	int res;

	res = WSAStartup(MAKEWORD(2, 2), &(server->wsaData));
	if (res != 0)
		return SERVER_ERR_COMMU;

	server->result = NULL;

	ZeroMemory(&server->hints, sizeof(server->hints));
	server->hints.ai_family = AF_INET;
	server->hints.ai_socktype = SOCK_STREAM;
	server->hints.ai_protocol = IPPROTO_TCP;
	server->hints.ai_flags = AI_PASSIVE;

	server->listenSocket = INVALID_SOCKET;

	return SERVER_SUCCESS;
}

ServerResult serverBind(Server server){
	if (!server)
		return SERVER_ERR_NULL_PTR;

	// Setup the TCP listening socket
	int iResult = bind(server->listenSocket, server->result->ai_addr, (int)(server->result->ai_addrlen));
	if (iResult == SOCKET_ERROR) {
		return SERVER_ERR_COMMU;
	}

	return SERVER_SUCCESS;
}

ServerResult serverListen(Server server) {
	if (!server)
		return SERVER_ERR_NULL_PTR;

	if (listen(server->listenSocket, SOMAXCONN) == SOCKET_ERROR) {
		return SERVER_ERR_COMMU;
	}

	return SERVER_SUCCESS;
}

ServerResult serverAccept(Server server) {
	if (!server)
		return SERVER_ERR_NULL_PTR;

	server->clientSocket = accept(server->listenSocket, NULL, NULL);
	if (server->clientSocket == INVALID_SOCKET) {
		return SERVER_ERR_COMMU;
	}

	return SERVER_SUCCESS;
}


// === D'tor sequence ===
ServerResult serverDisconnect(Server server) {
	if (!server)
		return SERVER_ERR_NULL_PTR;
	
	shutdown(server->clientSocket, SD_SEND);
	

	return SERVER_SUCCESS;
}

void serverCleanup(Server server) {
	if (!server)
		return;;

	WSACleanup();

	if (server->result != NULL)
		freeaddrinfo(server->result);

	if (server->listenSocket != INVALID_SOCKET)
		closesocket(server->listenSocket);

	if (server->clientSocket != INVALID_SOCKET)
		closesocket(server->clientSocket);
}