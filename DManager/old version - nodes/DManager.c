#include "DManager.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <Windows.h>

#define MAX_AVERAGES 5
#define MAX_PACKETS 5
#define MAX_DIGITS_IN_INT 10


struct dmPacketNode_t{
	char* data;
	int *decrypted;
	int decryptedSize;
	double average;
	
	struct dmPacketNode_t* next;
};

typedef struct dmPacketNode_t* DMPacketNode;

struct dManager_t{
	DMPacketNode head; //dummy node

	double averages[MAX_AVERAGES];
	int firstIndex;
	int counter;
};

// === Private declarations ===

bool isDigit(char d);
int getDecryptedSize(char* buffer);
int decryptData(char* buffer, int** pDecrypted);
void cleanupPacket(DMPacketNode packet);
double getPacketAverage(DMPacketNode packet);
DMPacketNode packetCreate();

// === End of private declarations ===



DManager DMCreate(){
	DManager dm = (DManager)malloc(sizeof(*dm));
	if(!dm)
		return NULL;
	
	dm->head = packetCreate();
	if(!dm->head){
		free(dm);
		return NULL;
	}

	dm->firstIndex = 0;
	dm->counter = 0;
	
	return dm;
}

void DMDestroy(DManager dm){
	if (!dm)
		return;
	
	while(dm->head != NULL){
		
		if(dm->head->data != NULL)
			free(dm->head->data);
		if(	dm->head->decrypted != NULL)
			free(dm->head->decrypted);
		
		DMPacketNode temp = dm->head;
		dm->head = dm->head->next;
		free(temp);
	}
		
	free(dm);
}

DMResult DMsetNextPacket(DManager dm, char* buffer){
	if(!dm || !buffer ||strlen(buffer) == 0 )
		return DM_ERR_NULL_PRT;

	int size;
	int* heartbeats = NULL;
	int sum = 0;
	size = decryptData(buffer, &heartbeats);
	for (int i = 0; i < size; i++)
		sum += heartbeats[i];

	if (dm->counter < MAX_AVERAGES)
		dm->averages[dm->counter++] = (double)(sum) / size;
	else {
		dm->averages[dm->firstIndex] = (double)(sum) / size;
		dm->firstIndex = (dm->firstIndex + 1) % MAX_AVERAGES;
	}
	
	DMPacketNode iter = dm->head;
	while(iter->next != NULL)
		iter = iter->next;
	
	iter->next = packetCreate();
	if(!iter->next)
		return DM_ERR_MEM;
	
	iter->next->data = malloc(sizeof(*(iter->next->data))*(strlen(buffer) + 1));
	if(!iter->next->data){
		cleanupPacket(iter->next);
		iter->next = NULL;
		return DM_ERR_MEM;
	}
	strcpy(iter->next->data, buffer);
	iter->next->decryptedSize = decryptData(buffer, &(iter->next->decrypted));
	
	if(iter->next->decryptedSize == -1){
		cleanupPacket(iter->next);
		iter->next = NULL;
		return DM_ERR_MEM;
	}
	
	iter->next->average = getPacketAverage(iter->next);
	
	return DM_SUCCESS;
}

DMPacketNode packetCreate() {
	DMPacketNode packet = (DMPacketNode)malloc(sizeof(*packet));
	if (!packet)
		return NULL;

	packet->average = 0;
	packet->data = NULL;
	packet->decrypted = NULL;
	packet->decryptedSize = 0;
	packet->next = NULL;

	return packet;
}



void cleanupPacket(DMPacketNode packet){
	if(!packet)
		return;
	
	assert(packet->next == NULL);
	
	if(packet->data)
		free(packet->data);
	
	if(packet->decrypted != NULL)
		free(packet->decrypted);
	
	free(packet);
}

int decryptData(char* buffer, int** pDecrypted){
	if(buffer == NULL || pDecrypted == NULL || *pDecrypted != NULL)
		return -1;
	
	int size = getDecryptedSize(buffer);
	 
	*pDecrypted = (int*)malloc(sizeof(int)*size);
	if(*pDecrypted == NULL)
		return -1;
	
	int decryptedIndex = 0;
	char tempNumber[MAX_DIGITS_IN_INT];
	ZeroMemory(tempNumber, MAX_DIGITS_IN_INT);
	int tempIndex = 0;
	
	for(int i = 0 ; i < strlen(buffer) ; i++){
		if(tempIndex > 0 && !isDigit(buffer[i])){
			(*pDecrypted)[decryptedIndex++] = atoi(tempNumber);
			tempIndex = 0;
			ZeroMemory(tempNumber, MAX_DIGITS_IN_INT);
			continue;
		}
		if(isDigit(buffer[i])){
			tempNumber[tempIndex++] = buffer[i];
		}	
	}
	if(tempIndex > 0)
		(*pDecrypted)[decryptedIndex++] = atoi(tempNumber);
	
	assert(decryptedIndex == size);
	return size;
}

int getDecryptedSize(char* buffer){
	if(!buffer)
		return -1;
	
	int counter = 0;
	bool inNumber = false;
	
	for(int i = 0; i < strlen(buffer); i++){
		if(inNumber && !isDigit(buffer[i])){
			counter++;
			inNumber = false;
			continue;
		}
		if(isDigit(buffer[i]))
			inNumber = true;
	}
	if (inNumber)
		counter++;

	return counter;
}

bool isDigit(char d){
	return d >= '0' && d <= '9';
}

double getPacketAverage(DMPacketNode packet){
	if(!packet || packet->decryptedSize == 0)
		return -1;
	
	double sum = 0;
	for(int i = 0 ; i < packet->decryptedSize; i++)
		sum += packet->decrypted[i];
	
	return sum/(packet->decryptedSize);
}

void DMPrintAvarages(DManager dm){
	if(!dm)
		return;
	
	printf("Printing avarages: ");
	for (int i = 0; i < dm->counter; i++)
		printf("%.2f ", dm->averages[(dm->firstIndex+i)%MAX_AVERAGES]);
	printf("\n");

	
	DMPacketNode iter = dm->head->next;
	while(iter != NULL){
		printf("%.2f ", iter->average);
		iter = iter->next;
	}
	printf("\n");
}