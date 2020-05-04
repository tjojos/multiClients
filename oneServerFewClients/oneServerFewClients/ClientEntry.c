#include "ClientEntry.h"
#include "DManager.h"
#include "Result.h"
#include <winsock2.h>
#include <Windows.h>
#include <stdlib.h>

struct clientEntry_t {
	SOCKET clientSocket;

	DManager dm;

	Result status;
	int counter;
};

ClientEntry* CECreate(SOCKET clientSocket) {
	ClientEntry* ce = (ClientEntry*)malloc(sizeof(*ce));
	if (!ce)
		return NULL;

	ce->clientSocket = clientSocket;
	ce->dm = DMCreate();
	assert(ce->dm != NULL);
	if (!ce->dm) {
		free(ce);
		return NULL;
	}

	ce->status = WAIT;
	ce->counter = 0;

	return ce;
}

void CEDestroy(ClientEntry* ce) {
	if (!ce)
		return;

	closesocket(ce->clientSocket);
	DMDestroy(ce->dm);

	free(ce);
}

Result CEPrint(ClientEntry* ce) {
	if (!ce)
		return ERR_NULL_PTR;

	DMPrintAverages(ce->dm);
	return SUCCESS;
}

Result CEGetNextPacket(ClientEntry* ce) {
	if (!ce)
		return  ERR_NULL_PTR;
	
	char buffer[DEFAULT_BUFLEN];
	int bytesReceived;
	ZeroMemory(buffer, DEFAULT_BUFLEN);

	bytesReceived = recv(ce->clientSocket, buffer, DEFAULT_BUFLEN, 0);

	if (bytesReceived == SOCKET_ERROR)
		return ERR_COMMU;

	if (bytesReceived == 0) {
		ce->status = DONE;
		return DONE;
	}

	DMsetNextPacket(ce->dm, buffer);

	ce->status = SUCCESS;
	return SUCCESS;
}

Result CEGetLastStatus(ClientEntry* ce) {
	if (!ce)
		return ERR_NULL_PTR;

	return ce->status;
}

SOCKET CEGetSocket(ClientEntry* ce) {
	if (!ce)
		return INVALID_SOCKET;

	return ce->clientSocket;
}

void CEIncrementCounter(ClientEntry* ce) {
	assert(ce != NULL);
	if (!ce)
		return;

	ce->counter++;
}

int CEGetCounter(ClientEntry* ce) {
	if (!ce)
		return -1;

	return ce->counter;
}