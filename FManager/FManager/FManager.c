#include "FManager.h"

#define DEFAULT_MAX_LINE 4096

struct fManager_t{
	FILE* file;
	char buffer[DEFAULT_MAX_LINE];
};

FManager FMCreate(char* path, unsigned int maxBuffer) {
	if (!path || strlen(path) == 0)
		return NULL;

	FManager fm = malloc(sizeof(*fm));
	if (!fm)
		return NULL;

	fm->file = fopen(path, "r");
	if (!fm->file) {
		free(fm);
		return NULL;
	}


	ZeroMemory(fm->buffer, DEFAULT_BUFLEN);

	return fm;
}

char* FMGetNextPacket(FManager fm, int* size) {
	if (!fm)
		return NULL;
	
	ZeroMemory(fm->buffer, DEFAULT_BUFLEN);
	int i = 0;
	char c = EOF;
	

	while (i < DEFAULT_BUFLEN && fscanf(fm->file, "%c", &c)  >0 && c != '\0' && c != EOF && c != '\n') {
		fm->buffer[i++] = c;
	}
	fm->buffer[i] = '\0';

	if (size)
		*size = i;

	return fm->buffer;
}

void FMDestroy(FManager fm) {
	if (!fm)
		return;

	fclose(fm->file);
	free(fm);
}