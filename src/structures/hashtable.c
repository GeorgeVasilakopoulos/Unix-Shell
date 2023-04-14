#include <string.h>
// #include <stdio.h>
#include <stdlib.h>
#include "hashtable.h"


	
void hashInit(Hashtable* table, int sizeOfItem, int (*hashFunction)(void*)){
	table->sizeOfItem = sizeOfItem;
	table->itemsCount = 0;
	table->hashFunction = hashFunction;
	memset(table->bucket,0,sizeof(struct tableEntry*)*BUCKETNUMBER);
}

void hashInsert(Hashtable* table, void* data){
	table->itemsCount++;
	int bucketID = table->hashFunction(data) % BUCKETNUMBER;
	// printf("Inserting in bucket %d\n",bucketID);
	struct tableEntry* entry = malloc(sizeof(struct tableEntry));
	entry->next = NULL;
	entry->data = malloc(table->sizeOfItem);
	memcpy(entry->data,data,table->sizeOfItem);
	if(table->bucket[bucketID] == NULL){
		table->bucket[bucketID] = entry;
		return;
	}
	struct tableEntry* j = table->bucket[bucketID];
	while(j->next != NULL)j = j->next;
	j->next = entry;
}

const void* hashFind(Hashtable* table, void* data, int (*compare)(void*, void*)){
	int bucketID = table->hashFunction(data) % BUCKETNUMBER;
	struct tableEntry* entry = table->bucket[bucketID];
	// printf("Lookming in bucket %d, %d\n",bucketID, entry==NULL);
	while(entry!=NULL){
		if(compare(data,entry->data))return entry->data;
		entry = entry->next;
		// printf("Found");
	}
	return NULL;
}


void hashRemove(Hashtable* table, void* data, int (*compare)(void*, void*)){
	int bucketID = table->hashFunction(data) % BUCKETNUMBER;
	struct tableEntry* entry = table->bucket[bucketID];
	if(entry==NULL)return;
	if(compare(data,entry->data)){
		table->bucket[bucketID] = entry->next;
		free(entry->data);
		free(entry);
		return;
	}

	// printf("Lookming in bucket %d, %d\n",bucketID, entry==NULL);
	while(entry->next!=NULL){
		if(compare(data,entry->data)){
			free(entry->next->data);
			struct tableEntry* temp = entry->next; 
			entry->next = entry->next->next;
			free(temp);
			return;
		}
		entry = entry->next;
	}
}


void hashDestruct(Hashtable* table){
	struct tableEntry* ptr;
	for(int i=0;i<BUCKETNUMBER;i++){
		ptr = table->bucket[i];
		struct tableEntry* temp;
		while(ptr){
			free(ptr->data);
			temp = ptr;
			ptr = ptr->next;
			free(temp); 
		}
	}

}




