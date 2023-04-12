
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


void listInit(List* list, unsigned int sizeOfItem);
void listPrepend(List* list, void* data);
void listAppend(List* list, void* data);
void listAddBefore(List* list, struct listnode* existingNode, void* data);
void listAddAfter(List* list, struct listnode* existingNode, void* data);
struct listnode* listFront(List* list);
struct listnode* listEnd(List* list);
void listRemove(List* list, struct listnode* node);
struct listnode* nextNode(struct listnode* node);
struct listnode* previousNode(struct listnode* node);
void listCat(List* list1, List* list2, List* result);
void listCopy(List* clone, List* original);
void* getDataPointer(struct listnode* node);
int listSize(List* list);
struct listnode* getNodeWithIndex(List* list, unsigned int i);
void visitList(List* list, void (*visit)(void*));
void destructList(List* list);




