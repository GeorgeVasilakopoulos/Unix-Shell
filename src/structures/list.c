// #include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"
 

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
	if(list->itemsCount == 1) list->end = node;
	else list->head->previous = node;
	list->head = node;
}


void listAppend(List* list, void* data){
	list->itemsCount++;
	struct listnode* node = malloc(sizeof(struct listnode));
	node->data = malloc(list->sizeOfItem);
	memcpy(node->data,data,list->sizeOfItem);
	node->next = NULL;
	node->previous = list->end;
	if(list->itemsCount == 1) list->head = node;
	else list->end->next = node;
	list->end = node;
}

void listAddBefore(List* list, struct listnode* existingNode, void* data){
	if(existingNode == NULL || existingNode == list->head){
		listPrepend(list,data);
		return;
	}

	list->itemsCount++;

	struct listnode* node = malloc(sizeof(struct listnode));
	node->data = malloc(list->sizeOfItem);
	memcpy(node->data, data, list->sizeOfItem);
	node->next = existingNode;
	node->previous = existingNode->previous;

	existingNode->previous->next = node;
	existingNode->previous = node;
}


void listAddAfter(List* list, struct listnode* existingNode, void* data){
	if(existingNode == NULL || existingNode == list->end){
		listAppend(list,data);
		return;
	}

	list->itemsCount++;

	struct listnode* node = malloc(sizeof(struct listnode));
	node->data = malloc(list->sizeOfItem);
	memcpy(node->data, data, list->sizeOfItem);
	node->next = existingNode->next;
	node->previous = existingNode;

	existingNode->next->previous = node;
	existingNode->next = node;
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

void* getDataPointer(struct listnode* node){
	if(node == NULL) return NULL;
	return node->data;
}

int listSize(List* list){
	return list->itemsCount;
}


void listCopy(List* clone, List* original){
	if(clone == original)return;
	destructList(clone);
	clone->sizeOfItem = original->sizeOfItem;
	for(struct listnode* i = listFront(original); i!=NULL;i = nextNode(i)){
		listAppend(clone,i->data);
	}
}


void listCat(List* list1, List* list2, List* result){
	if(result == list1){
		for(struct listnode* i = listFront(list2);i != NULL ; i = nextNode(i)){
			listAppend(result,i->data);
		}
	}
	else if(result == list2){
		for(struct listnode* i = listEnd(list1);i != NULL ; i = previousNode(i)){
			listPrepend(result,i->data);
		}
	}
	else{
		listCopy(result,list1);
		for(struct listnode* i = listFront(list1);i != NULL ; i = nextNode(i)){
			listAppend(result,i->data);
		}
	}
}


void destructList(List* list){
	struct listnode* node = listFront(list);
	while(node != NULL){
		free(node->data);
		struct listnode* temp = node;
		node = node->next;
		free(temp);
	}
	listInit(list,list->sizeOfItem);
}


void visitList(List* list, void (*visit)(void*)){
	struct listnode* node = listFront(list);
	for(int i=0;i<listSize(list) && node; i++){
		visit(getDataPointer(node));
		node = nextNode(node);
	}
}

struct listnode* getNodeWithIndex(List* list, unsigned int i){
	if(i >= list->itemsCount)return NULL;
	struct listnode* ptr = listFront(list);
	while(i--)ptr = ptr->next;
	return ptr;
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



