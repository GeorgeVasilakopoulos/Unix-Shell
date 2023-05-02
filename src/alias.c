#include <stdlib.h>
#include <string.h>
#include "structures/hashtable.h"
#include "structures/list.h"
#include "alias.h"
#include "lexer.h"
#include "interface.h"

static Hashtable aliasTable;


//Alias-String pair
typedef struct aliasMap{
	char alias[MAXBUFSIZE];
	char mappedString[MAXBUFSIZE];
}aliasMapping;

//String hash function. Sum up ascii values. Result will be taken mod with n. of buckets.
static int hash(void* data){
	int sum = 0;
	for(int i=0; i < strlen(((aliasMapping*)data)->alias); i++){
		sum += ((aliasMapping*)data)->alias[i];
	}
	return sum;
}

//If two aliasMappings are about the same 'alias', 
static int compare(void* data1, void* data2){
	struct aliasMap* ptr1 = (struct aliasMap*)data1;
	struct aliasMap* ptr2 = (struct aliasMap*)data2;
	if(!strcmp(ptr1->alias,ptr2->alias))return 1;
	return 0;
}



void aliasInit(){
	hashInit(&aliasTable,sizeof(aliasMapping),&hash);
}



void createAlias(const char* alias, const char* instruction){
	removeAlias(alias); //Possibly remove any previous record.
	aliasMapping data;	//Create pair structure
	strcpy(data.alias,alias);
	strcpy(data.mappedString,instruction);
	hashInsert(&aliasTable,(void*)&data);	
}


const char* findAlias(const char* alias){
	if(!alias)return NULL;
	aliasMapping data;
	strcpy(data.alias,alias);
	aliasMapping* pair = (aliasMapping*) hashFind(&aliasTable,&data,&compare);
	if(pair == NULL)return NULL;	//alias not found
	return pair->mappedString;		
	
}

void replaceAliasesInList(List* tokenList){
	List result;
	listInit(&result,sizeof(char)*MAXBUFSIZE);
	const char* alias;
	for(struct listnode* i = listFront(tokenList); i != NULL; i = nextNode(i)){
		if((alias = findAlias(getDataPointer(i)))){	//If the token is a registered alias
			List tempList;
			listInit(&tempList,sizeof(char)*MAXBUFSIZE);	
			createTokenList(alias,&tempList);		//Transform aliased instruction into token list
			replaceAliasesInList(&tempList);		//Recursively replace aliases in list
			listCat(&result,&tempList,&result);		//Concatenate results
			destructList(&tempList);
		}
		else listAppend(&result,getDataPointer(i));	//If it is not, simply add it to the result
	}
	listCopy(tokenList,&result);		//replace given list with the result.
	destructList(&result);
}






void removeAlias(const char* alias){
	aliasMapping data;
	strcpy(data.alias,alias);
	hashRemove(&aliasTable,&data,&compare);
}


void destructAlias(){
	hashDestruct(&aliasTable);
}



