#pragma once
#include "Libs.h"

typedef struct clientEntry_t ClientEntry;

ClientEntry* CECreate(SOCKET clientSocket);

void CEDestroy(ClientEntry* ce);

Result CEPrint(ClientEntry* ce);

Result CEGetNextPacket(ClientEntry* ce);

Result CEGetLastStatus(ClientEntry* ce);

SOCKET CEGetSocket(ClientEntry* ce);

void CEIncrementCounter(ClientEntry* ce);

int CEGetCounter(ClientEntry* ce);