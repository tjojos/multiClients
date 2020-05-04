#include "Server.h"
#include "ClientEntry.h"


#define DEFAULT_IP "127.0.0.1"
#define DEFAULT_PORT "27015"

//Private functions declarations
Result serverInit(Server);
Result serverBind(Server);
Result serverListen(Server);

Result serverAccept(Server);
DWORD WINAPI runCE(Server);
int findNextOpenSocket(Server server);

Result serverDisconnect(Server);
void serverCleanup(Server);

struct server_t {
	WSADATA wsaData;
	struct addrinfo* result, hints;

	SOCKET listenSocket;

	ClientEntry* clients[MAX_CLIENTS];
	HANDLE clientThreads[MAX_CLIENTS];

	int clientCounter;
	int packetCounter;
	HANDLE packetCounterLock;

};

struct Bundle {
	Server server;
	int CEIndex;
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

	if (serverInit(server) != SUCCESS) {
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

	Result res = serverBind(server);
	if (res != SUCCESS) {
		serverCleanup(server);
		return NULL;
	}

	res = serverListen(server);
	if (res != SUCCESS) {
		serverCleanup(server);
		return NULL;
	}

	printf("Server launched\n");

	return server;
}

Result serverRun(Server server) {
	if (!server)
		return ERR_NULL_PTR;

	Result res;
	do {
		res = serverAccept(server);
	}while ( res == SUCCESS );

	return res;
}

Result serverPrint(Server server) {
	if (!server)
		return ERR_NULL_PTR;

	int packets = 0, packet = 0, counter = 0;
	double sum = 0;


	for (int i = 0; i < MAX_CLIENTS; i++) {
		if (server->clients[i] != NULL) {

			sum += CEGetSingleAverage(server->clients[i]);
			counter++;

			printf("Client %d: ", i);
			CEPrint(server->clients[i]);
			packet = CEGetCounter(server->clients[i]);

			printf(" | Packets: %d", packet);
			packets += packet;
			printf("\n");
		}
	}



	printf("Packets: %d | PacketCounter: %d\n", packets, server->packetCounter);

	if (counter > 0)
		printf("Overall Average: %.2f\n", sum/counter);
	
	return SUCCESS;
}

Result serverCleanDoneClients(Server server) {
	if (!server)
		return ERR_NULL_PTR;

	for (int i = 0; i < MAX_CLIENTS; i++) {
		if (server->clients[i] != NULL && CEGetLastStatus(server->clients[i]) == DONE) {
			shutdown(CEGetSocket(server->clients[i]), SD_SEND);
			CEDestroy(server->clients[i]);
			server->clients[i] = NULL;
			server->clientCounter--;
		}
	}
	return SUCCESS;
}

void serverDestroy(Server server) {
	if (!server)
		return;
	//TODO: add wait for client threads
	WaitForMultipleObjects(MAX_CLIENTS, server->clientThreads, TRUE, INFINITE);

	serverCleanDoneClients(server);
	//TODO: add force clean up

	CloseHandle(server->packetCounterLock);

	serverCleanup(server);
	printf("Server disconnected\n");

	free(server);
}

int serverGetClientCounter(Server server) {
	if (!server)
		return -1;

	return server->clientCounter;
}

int serverGetNumberOfActiveClient(Server server){ 
	//TODO: save counter to work in O(1)
	if (!server)
		return -1;

	int counter = 0;
	for (int i = 0; i < MAX_CLIENTS; i++) {
		if (server->clients[i] != NULL && CEGetLastStatus(server->clients[i]) != DONE)
			counter++;
	}
	return counter;
}


// === for debug ===
void printClients(Server server) {
	assert(server != NULL);

	for (int i = 0; i < MAX_CLIENTS; i++) {
		if(server->clients[i] != NULL)
		printf("Client %d: %d\n", i, CEGetLastStatus(server->clients[i]));
	}
}





// === C'tor sequence ===
Result serverInit(Server server) {
	if (!server)
		return  ERR_NULL_PTR;

	int res;

	res = WSAStartup(MAKEWORD(2, 2), &(server->wsaData));
	if (res != 0)
		return  ERR_COMMU;

	server->result = NULL;

	ZeroMemory(&server->hints, sizeof(server->hints));
	server->hints.ai_family = AF_INET;
	server->hints.ai_socktype = SOCK_STREAM;
	server->hints.ai_protocol = IPPROTO_TCP;
	server->hints.ai_flags = AI_PASSIVE;

	server->listenSocket = INVALID_SOCKET;

	for (int i = 0; i < MAX_CLIENTS; i++)
		server->clients[i] = NULL;

	server->clientCounter = 0;
	server->packetCounter = 0;
	server->packetCounterLock = CreateMutexA(NULL, false, NULL);
	assert(server->packetCounterLock != 0);

	return SUCCESS;
}

Result serverBind(Server server){
	if (!server)
		return  ERR_NULL_PTR;

	// Setup the TCP listening socket
	int iResult = bind(server->listenSocket, server->result->ai_addr, (int)(server->result->ai_addrlen));
	if (iResult == SOCKET_ERROR) {
		return  ERR_COMMU;
	}

	return SUCCESS;
}

Result serverListen(Server server) {
	if (!server)
		return  ERR_NULL_PTR;

	if (listen(server->listenSocket, SOMAXCONN) == SOCKET_ERROR) {
		return  ERR_COMMU;
	}

	return SUCCESS;
}

Result serverAccept(Server server) {
	if (!server)
		return  ERR_NULL_PTR;
	int nextIndex = findNextOpenSocket(server);
	SOCKET newClientSocket = INVALID_SOCKET;
	if (nextIndex == -1)
		return SERVER_FULL;

	struct Bundle* bundle = malloc(sizeof(*bundle));
	bundle->CEIndex = nextIndex;
	bundle->server = server;



	newClientSocket = accept(server->listenSocket, NULL, NULL);
	if (newClientSocket == INVALID_SOCKET) {
		return  ERR_COMMU;
	}

	server->clients[nextIndex] = CECreate(newClientSocket);
	//server->nextIndex = nextClient;
	server->clientThreads[nextIndex] =  CreateThread(NULL, 0, runCE, bundle, 0, NULL);

	
	if (server->clients[nextIndex] == NULL) {
		//TODO: handle
		return ERR_MEMORY;
	}

	server->clientCounter++;

	return SUCCESS;
}


//	=== Running helpers ===

//New thread for client
DWORD WINAPI runCE(struct Bundle* bundle) {
	Server server = bundle->server;
	assert(server != NULL);
	if (!server)
		return -1;

	ClientEntry* ce = server->clients[bundle->CEIndex];
	free(bundle);

	assert(ce != NULL);
	assert(CEGetLastStatus(ce) == WAIT);
	if (!ce)
		return -1;
	


	Result res = CEGetNextPacket(ce);
	while (res == SUCCESS) {
		WaitForSingleObject(server->packetCounterLock, INFINITE);
		for (int i = 0; i < 10000; i++) {
			server->packetCounter++;
			CEIncrementCounter(ce);
		}
		ReleaseMutex(server->packetCounterLock);
		res = CEGetNextPacket(ce);
	}
	//TODO: set status
	return res;
}

int findNextOpenSocket(Server server) {
	assert(server != NULL);
	if (!server)
		return -1;

	for (int i = 0; i < MAX_CLIENTS; i++) {
		if (server->clients[i] == NULL)
			return i;
	}
	return -1;
}


// === D'tor sequence ===

void serverCleanup(Server server) {
	if (!server)
		return;;

	WSACleanup();

	if (server->result != NULL)
		freeaddrinfo(server->result);

	if (server->listenSocket != INVALID_SOCKET)
		closesocket(server->listenSocket);
}