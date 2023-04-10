#include "structures/hashtable.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


static Hashtable aliasTable;

typedef struct aliasMap{
	char alias[100];
	char mappedString[100];
}aliasMapping;


static int hash(void* data){
	int sum = 0;
	for(int i=0; i < strlen(((aliasMapping*)data)->alias); i++){
		sum += ((aliasMapping*)data)->alias[i];
	}
	return sum;
}


static int compare(void* data1, void* data2){
	struct aliasMap* ptr1 = (struct aliasMap*)data1;
	struct aliasMap* ptr2 = (struct aliasMap*)data2;
	if(strcmp(ptr1->alias,ptr2->alias)==0)return 1;
	return 0;
}



void aliasInit(){
	hashInit(&aliasTable,sizeof(aliasMapping),&hash);
}



void createAlias(const char* alias, const char* instruction){
	aliasMapping data;
	strcpy(data.alias,alias);
	strcpy(data.mappedString,instruction);
	hashInsert(&aliasTable,(void*)&data);
}


const char* findAlias(char* alias){
	if(!alias)return NULL;
	aliasMapping data;
	strcpy(data.alias,alias);
	aliasMapping* pair = (aliasMapping*) hashFind(&aliasTable,&data,&compare);
	if(pair == NULL)return NULL;
	return pair->mappedString;
	
}


void removeAlias(char* alias){
	aliasMapping data;
	strcpy(data.alias,alias);
	hashRemove(&aliasTable,&data,&compare);
}


void destructAlias(){
	hashDestruct(&aliasTable);
}



