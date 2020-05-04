#include "Libs.h"

#include "DManager.h"
#include "Server.h"
#include "Client.h"
#include "FManager.h"
#include "HeartbeatGenerator.h"

#include <string.h>



DWORD WINAPI launchServer();
DWORD WINAPI launchClient();


int main() {

    DWORD serverTID;
    DWORD clientTID;

    HANDLE serverThread = CreateThread(NULL, 0, launchServer, NULL, 0, &serverTID);
    HANDLE clientThread = CreateThread(NULL, 0, launchClient, NULL, 0, &clientTID);

    if (!serverThread || !clientThread) {
        return ERR_THREAD;
    }

    WaitForMultipleObjects(1, &serverThread, TRUE, INFINITE);
    WaitForMultipleObjects(1, &clientThread, TRUE, INFINITE);

    return 0;
}

DWORD WINAPI launchServer() {
    Server server = serverCreate(NULL, NULL);
    assert(server != NULL);

    int debugCounter = 0;
    serverRun(server);

    do {
        debugCounter++;
        if (debugCounter % 5 == 0)
            printClients(server);
        //serverPrint(server);
        printf("Receiving...\n");
        Sleep(SEC_IN_MILLI);
    } while (serverGetNumberOfActiveClient(server) > 0);

    serverPrint(server);

    serverDestroy(server);
    
    return DONE;
}

DWORD WINAPI launchClient() {
    Client clients[MAX_CLIENTS];
    char path[DEFAULT_BUFLEN];

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (i % 100 == 0)
            printf("Launching the %d client\n", i);

        ZeroMemory(path, DEFAULT_BUFLEN);

        strcpy(path, DATA_NAME);
        itoa(i, (path+strlen(DATA_NAME)), 10);
        strcat(path, DATA_SUFFIX);

        generateHeartbeats(path, 0, 0, 0, 0);

        clients[i] = clientCreate(NULL, NULL, path);
        clientRun(clients[i]);
    }

   

    return DONE;
}
