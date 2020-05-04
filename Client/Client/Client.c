#include "Client.h"
#include "FManager.h"


//Private functions signatures
Result clientInit(Client client);
void clientCleanup(Client client);
Result clientConnect(Client client);
Result clientSend(Client client);
Result clientDisconnect(Client client);
DWORD WINAPI run(Client client);

struct client_t {
	WSADATA wsaData;
	SOCKET ConnectSocket;
	struct addrinfo* result, hints;

	DWORD clientTID;
	HANDLE clientThread;

	char buffer[DEFAULT_BUFLEN];

	FManager fm;
};


//=== API Functions ===

Client clientCreate(const char* ip, const char* port, const char* path) {
	Client client = malloc(sizeof(*client));

	if (!client)
		return NULL;

	if (!ip)
		ip = DEFAULT_IP;

	if (!port)
		port = DEFAULT_PORT;

	client->fm = FMCreate(path, DEFAULT_BUFLEN);
	if (client->fm == NULL) {
		free(client);
		PRNT_ERR(ERR_MEMORY, __FILE__, __LINE__);
		return NULL;
	}

	if (clientInit(client) !=  SUCCESS) {
		FMDestroy(client->fm);
		free(client);
		PRNT_ERR(ERR_MEMORY, __FILE__, __LINE__);
		return NULL;
	}

	int res;

	res = getaddrinfo(ip, port, &client->hints, &client->result);

	if (res != 0) {
		clientCleanup(client);
		return NULL;
	}

	// Create a SOCKET for connecting to server
	client->ConnectSocket = socket(client->result->ai_family, client->result->ai_socktype,
		client->result->ai_protocol);


	//client->ConnectSocket = socket(,)
	if (client->ConnectSocket == INVALID_SOCKET) {
		clientCleanup(client);
		return NULL;
	}
	//printf("Client created\n");

	res = clientConnect(client);
	if (res !=  SUCCESS) {
		clientCleanup(client);
		return NULL;
	}
	//printf("Client connected\n");

	return client;
}

Result clientRun(Client client) {
	if (!client)
		return ERR_NULL_PTR;

	client->clientThread = CreateThread(NULL, 0, run, client, 0, &(client->clientTID));
}

void clientDestroy(Client client) {
	if (!client)
		return;

	FMDestroy(client->fm);
	clientDisconnect(client);
	//printf("Client disconneted\n");

	clientCleanup(client);
	free(client);
}



// === C'tor sequence ===
Result clientInit(Client client) {
	if (!client)
		return  ERR_NULL_PTR;

	int res;
	res = WSAStartup(MAKEWORD(2, 2), &(client->wsaData));

	if (res != 0) {
		printf("WSAStartup failed: %d\n", res);
		return  ERR_COMMU;
	}


	client->result = NULL;

	ZeroMemory(&(client->hints), sizeof(client->hints));
	client->hints.ai_family = AF_UNSPEC;
	client->hints.ai_socktype = SOCK_STREAM;
	client->hints.ai_protocol = IPPROTO_TCP;

	client->ConnectSocket = INVALID_SOCKET;

	ZeroMemory(client->buffer, DEFAULT_BUFLEN);

	return  SUCCESS;
}

Result clientConnect(Client client) {
	int res = connect(client->ConnectSocket, client->result->ai_addr, (int)client->result->ai_addrlen);
	if (res == SOCKET_ERROR) {
		return  ERR_COMMU;
	}

	if (client->ConnectSocket == INVALID_SOCKET) {
		return  ERR_COMMU;
	}

	return  SUCCESS;
}


// === Send Helpers ===
DWORD WINAPI run(Client client) {
	Result res;
	do {
		res = clientSend(client);
		if (res == DONE) {
			//printf("Client Done\n");
			break;
		}
		if (res == SUCCESS) {
			//Sleep(1*SEC_IN_MILLI);
			continue;
		}


		PRNT_ERR(res, __FILE__, __LINE__);
		return -1;

	} while (true);

	clientDestroy(client);

	return DONE;
}

Result clientSend(Client client) {
	if (!client) {
		return  ERR_NULL_PTR;
	}

	int size;
	char* buffer = FMGetNextPacket(client->fm, &size);

	if (size == 0)
		return  DONE;

	int res = send(client->ConnectSocket, buffer, DEFAULT_BUFLEN, 0);

	if (res > 0) {
		return  SUCCESS;
	}

	return  ERR_COMMU;
}

// === D'tor sequence ===
void clientCleanup(Client client) {
	if (!client)
		return;

	WSACleanup();

	if (client->result != NULL)
		freeaddrinfo(client->result);

	if (client->ConnectSocket != INVALID_SOCKET)
		closesocket(client->ConnectSocket);
}

Result clientDisconnect(Client client) {
	// shutdown the send half of the connection since no more data will be sent
	int res = shutdown(client->ConnectSocket, SD_SEND);
	if (res == SOCKET_ERROR) {
		return  ERR_COMMU;
	}
	return  SUCCESS;
}