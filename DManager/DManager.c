#include "DManager.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <Windows.h>

#define MAX_AVERAGES 5
#define MAX_DIGITS_IN_INT 10
#define MAX_INTS_IN_PACKET 256


struct dManager_t{
	double averages[MAX_AVERAGES];
	int firstIndex;
	int counter;
};

// === Private declarations ===

bool isDigit(char d);
int decryptData(char* buffer, int* pDecrypted);

// === End of private declarations ===



DManager DMCreate(){
	DManager dm = (DManager)malloc(sizeof(*dm));
	if(!dm)
		return NULL;

	dm->firstIndex = 0;
	dm->counter = 0;
	
	return dm;
}

void DMDestroy(DManager dm){
	if (!dm)
		return;
		
	free(dm);
}

Result DMsetNextPacket(DManager dm, char* buffer){
	if(!dm || !buffer ||strlen(buffer) == 0 )
		return ERR_NULL_PTR;

	int size;
	int heartbeats[MAX_INTS_IN_PACKET];
	ZeroMemory(heartbeats, MAX_INTS_IN_PACKET);
	int sum = 0;
	size = decryptData(buffer, heartbeats);
	for (int i = 0; i < size; i++)
		sum += heartbeats[i];

	if (dm->counter < MAX_AVERAGES)
		dm->averages[dm->counter++] = (double)(sum) / size;
	else {
		dm->averages[dm->firstIndex] = (double)(sum) / size;
		dm->firstIndex = (dm->firstIndex + 1) % MAX_AVERAGES;
	}
	
	return SUCCESS;
}

int decryptData(char* buffer, int* pDecrypted){
	if(buffer == NULL || pDecrypted == NULL)
		return -1;
		
	int decryptedIndex = 0;
	char tempNumber[MAX_DIGITS_IN_INT];
	ZeroMemory(tempNumber, MAX_DIGITS_IN_INT);
	int tempIndex = 0;
	
	for(unsigned int i = 0 ; i < strlen(buffer) ; i++){
		if(tempIndex > 0 && !isDigit(buffer[i])){
			pDecrypted[decryptedIndex++] = atoi(tempNumber);
			tempIndex = 0;
			ZeroMemory(tempNumber, MAX_DIGITS_IN_INT);
			continue;
		}
		if(isDigit(buffer[i])){
			tempNumber[tempIndex++] = buffer[i];
		}	
	}
	if(tempIndex > 0)
		pDecrypted[decryptedIndex++] = atoi(tempNumber);
	
	return decryptedIndex;
}

bool isDigit(char d){
	return d >= '0' && d <= '9';
}

void DMPrintAverages(DManager dm){
	if(!dm)
		return;
	
	for (int i = 0; i < dm->counter; i++)
		printf("%.2f ", dm->averages[(dm->firstIndex+i)%MAX_AVERAGES]);
}