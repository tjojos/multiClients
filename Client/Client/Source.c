#include "Client.h"
#include <stdio.h>

int main() {
	printf("Gum will be prefection\n");

	Client client = clientCreate(NULL, NULL);

	if (client == NULL)
		printf("Error");
	else
		printf("client online");
	
	ClientResult res = clientSend(client, "yay", 4);
	
	clientDestroy(client);

	printf("C. Bing\n");
	return 0;
}