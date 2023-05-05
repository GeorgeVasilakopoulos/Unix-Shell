#include <stdlib.h>
#include <string.h>
#include "hashtable.h"


//Implementation of a generic hash table data structure
//In case of collisions, 


//Initialize Structure	
void hashInit(Hashtable* table, size_t sizeOfItem, int (*hashFunction)(void*)){
	table->sizeOfItem = sizeOfItem;
	table->itemsCount = 0;
	table->hashFunction = hashFunction;
	memset(table->bucket,0,sizeof(struct tableEntry*)*BUCKETNUMBER);
}



void hashInsert(Hashtable* table, void* data){
	table->itemsCount++;
	int bucketID = table->hashFunction(data) % BUCKETNUMBER;
	struct tableEntry* entry = malloc(sizeof(struct tableEntry));

	entry->next = table->bucket[bucketID]; //Could be NULL
	entry->data = malloc(table->sizeOfItem);

	memcpy(entry->data,data,table->sizeOfItem);
	table->bucket[bucketID] = entry;
}


//Find element in hash table
//Returns pointer to data if found
//Returns NULL if not found
const void* hashFind(Hashtable* table, void* data, int (*compare)(void*, void*)){
	int bucketID = table->hashFunction(data) % BUCKETNUMBER;
	struct tableEntry* entry = table->bucket[bucketID];
	while(entry!=NULL){
		if(compare(data,entry->data))return entry->data;
		entry = entry->next;
	}
	return NULL;
}

//Remove element from hash table
//Argument 'compare' is a comparator function between items
void hashRemove(Hashtable* table, void* data, int (*compare)(void*, void*)){
	int bucketID = table->hashFunction(data) % BUCKETNUMBER;
	struct tableEntry* entry = table->bucket[bucketID];
	if(entry==NULL)return;
	if(compare(data,entry->data)){
		table->bucket[bucketID] = entry->next;
		free(entry->data);
		free(entry);
		table->itemsCount--;
		return;
	}

	while(entry->next!=NULL){
		if(compare(data,entry->data)){
			free(entry->next->data);
			struct tableEntry* temp = entry->next; 
			entry->next = entry->next->next;
			free(temp);
			table->itemsCount--;
			return;
		}
		entry = entry->next;
	}
}

//Destruct hash table
void hashDestruct(Hashtable* table){
	struct tableEntry* ptr;
	for(int i=0;i<BUCKETNUMBER;i++){
		ptr = table->bucket[i];
		while(ptr){
			free(ptr->data);
			struct tableEntry* temp = ptr;
			ptr = ptr->next;
			free(temp); 
		}
	}

}




