#pragma once

typedef struct dManager_t* DManager;

typedef enum{
	DM_SUCCESS,
	DM_ERR_NULL_PRT,
	DM_ERR_MEM
}DMResult;

DManager DMCreate();

void DMDestroy(DManager);

DMResult DMsetNextPacket(DManager, char*);

void DMPrintAvarages(DManager dm);