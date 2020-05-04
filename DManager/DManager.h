#pragma once
#include "Libs.h"

typedef struct dManager_t* DManager;


DManager DMCreate();

void DMDestroy(DManager);

Result DMsetNextPacket(DManager, char*);

void DMPrintAverages(DManager dm);