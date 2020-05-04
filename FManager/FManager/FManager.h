#pragma once
#include "Libs.h"

typedef struct fManager_t* FManager;

FManager FMCreate(char* path, unsigned int maxBuffer);

char* FMGetNextPacket(FManager fm, int * size);

void FMDestroy(FManager fm);