#include "FManager.h"
#include <stdio.h>

int main() {
	printf("How you doin..?\n");

	FManager fm = FMCreate("file_exaple.txt", 64);
	
	printf(FMGetNextPacket(fm));
	printf("\n");
	printf(FMGetNextPacket(fm));
	printf("\n");
	printf(FMGetNextPacket(fm));
	printf("\n");
	printf(FMGetNextPacket(fm));
	printf("\n");
	printf(FMGetNextPacket(fm));
	printf("\n");
	printf(FMGetNextPacket(fm));
	printf("\n");


	FMDestroy(fm);

	printf("-joey tribbiani\n");
	return 0;
}