#include "DManager.h"
#include <stdio.h>

int main(){
	printf("Hello world!\n");

	DManager dm = DMCreate();
	
	DMsetNextPacket(dm, "10 20 30");
	DMsetNextPacket(dm, "100 200 300");
	DMsetNextPacket(dm, "");
	DMsetNextPacket(dm, "asdg  sdsdg ff 15 fs    ");

	DMPrintAvarages(dm);

	DMDestroy(dm);

	printf("Bye Bye world! -c.19\n");
	return 0;	
}