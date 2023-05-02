#define BUCKETNUMBER 100

struct tableEntry{
	void* data;
	struct tableEntry* next;
};



typedef struct hashtable{
	struct tableEntry*bucket[BUCKETNUMBER];
	int sizeOfItem;
	int itemsCount;
	int(*hashFunction)(void*);
}Hashtable;




void hashInit(Hashtable* table, int sizeOfItem, int (*hashFunction)(void*));
void hashInsert(Hashtable* table, void* data);
const void* hashFind(Hashtable* table, void* data, int (*compare)(void*, void*));
void hashRemove(Hashtable* table, void* data, int (*compare)(void*, void*));
void hashDestruct(Hashtable* table);
