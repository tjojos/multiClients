#include "Server.h"
#include <stdio.h>

int main() {
	printf("IM FINEEE\n");

	Server server = serverCreate(NULL, NULL);

	char buffer[512];

	while (serverGetNextPacket(server, buffer, 512, NULL) == SUCCESS) {
		printf(buffer);
		printf("\n");
	}

	serverDestroy(server);

	printf("-R Geller\n");
	return 0;
}