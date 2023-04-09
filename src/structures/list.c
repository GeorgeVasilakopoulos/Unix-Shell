#include <stdio.h>
#include <stdlib.h>
#include <string.h>

 
struct listnode{
	void* data;
	struct listnode* next;
	struct listnode* previous;
};


typedef struct listStruct{
	unsigned int sizeOfItem;
	struct listnode* head;
	struct listnode* end;
	int itemsCount;
}List;



void listInit(List* list, unsigned int sizeOfItem){
	list->itemsCount = 0;
	list->head = NULL;
	list->end = NULL;
	list->sizeOfItem = sizeOfItem; 
}

void listPrepend(List* list, void* data){
	list->itemsCount++;
	struct listnode* node = malloc(sizeof(struct listnode));
	node->data = malloc(list->sizeOfItem);
	memcpy(node->data,data,list->sizeOfItem);
	node->next = list->head;
	node->previous = NULL;
	list->head = node;
	if(list->itemsCount == 1) list->end = node;
}


void listAppend(List* list, void* data){
	list->itemsCount++;
	struct listnode* node = malloc(sizeof(struct listnode));
	node->data = malloc(list->sizeOfItem);
	memcpy(node->data,data,list->sizeOfItem);
	node->next = NULL;
	node->previous = list->end;
	list->end = node;
	if(list->itemsCount == 1) list->head = node;
}



struct listnode* listFront(List* list){
	return list->head;
}

struct listnode* listEnd(List* list){
	return list->end;
}

void listRemove(List* list, struct listnode* node){
	list->itemsCount--;
	free(node->data);
	if(node->previous == NULL){
		list->head = node->next;
	}
	else node->previous->next = node->next;
	if(node->next == NULL){
		list->end = node->previous;
	}
	else node->next->previous = node->previous;
	free(node);
}


struct listnode* nextNode(struct listnode* node){
	if(node == NULL)return NULL;
	return node->next;
}

struct listnode* previousNode(struct listnode* node){
	if(node == NULL)return NULL;
	return node->previous;
}



int listSize(List* list){
	return list->itemsCount;
}






// int main(){
// 	List mylist;
// 	listInit(&mylist,sizeof(int));
// 	for(int i = 0;i<10;i++){
// 		listPrepend(&mylist,&i);
// 	}

// 	struct listnode* node = listFront(&mylist);

// 	printf("%d\n",*(int*)node->data);
// 	node = nextNode(node);
	
// 	printf("%d\n",*(int*)node->data);
// 	node = nextNode(node);
	

// 	printf("%d\n",*(int*)node->data);
// 	node = nextNode(node);



// 	return 0;
// }



