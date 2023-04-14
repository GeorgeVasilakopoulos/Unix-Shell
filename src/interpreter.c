#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "structures/list.h"
#include "alias.h"
#include "lexer.h"

#define MAXHISTORY 1

List instructionHistory;

void printtoken(void* data){
	printf("Token is %s\n", (char*)data);
}

void possiblyCloseFile(int filedes){
	if(filedes != 1 && filedes != 0)close(filedes);
}

void forkExecute(int inputfd, int outputfd, const char* commandName, const char* arguments[], int waitForChild){
	pid_t pid = fork();
	if(!pid){
		dup2(inputfd,0);
		dup2(outputfd,1);
		int statusCode = execvp(commandName,(char * const*)arguments);
		printf("Status code %d\n",statusCode);
		exit(statusCode);
	}
	// printf("waiting for %s\n", commandName);
	possiblyCloseFile(inputfd);
	possiblyCloseFile(outputfd);
	if(waitForChild){
		while(wait(NULL) != pid);
		// printf("Exited!!\n");
	}
}


void addToHistory(const char* inst){
	// printf("Adding %s to histor\n",inst);
	char* copyOfInst = malloc(sizeof(char)*(strlen(inst)+1));
	strcpy(copyOfInst,inst);
	if(listSize(&instructionHistory)>MAXHISTORY){
		free(*(char**)getDataPointer(listEnd(&instructionHistory)));
		listRemove(&instructionHistory,listEnd(&instructionHistory));
	}
	listPrepend(&instructionHistory,&copyOfInst);
}


static void freePointerFromAddress(void** p){free(*p);}

void clearHistory(){
	visitList(&instructionHistory,(void (*)(void*))&freePointerFromAddress);
	destructList(&instructionHistory);
}


void replaceAliasesInList(List* tokenList){
	List result;
	listInit(&result,sizeof(char)*50);
	const char* alias;
	for(struct listnode* i = listFront(tokenList); i != NULL; i = nextNode(i)){
		if(alias = findAlias(getDataPointer(i))){
			List tempList;
			listInit(&tempList,sizeof(char)*50);
			createTokenList(alias,&tempList);
			replaceAliasesInList(&tempList);
			listCat(&result,&tempList,&result);
			destructList(&tempList);
		}
		else listAppend(&result,getDataPointer(i));
	}
	listCopy(tokenList,&result);
	destructList(&result);
}




int interpretInstruction(const char* readbuf){
	int length = strlen(readbuf);
	const char* commandName;
	const char* arguments[100]={};
	int argumentCounter = 1;
	int IOfd[2] = {0,1};
	List tokenList;
	listInit(&tokenList,sizeof(char)*50);
	createTokenList(readbuf,&tokenList);
	struct listnode* ptr = listFront(&tokenList);
	commandName = getDataPointer(ptr);
	if(!commandName){destructList(&tokenList);return 0;}

	if(!strcmp(commandName,"destroyalias")){
		char alias[100];
		getNextToken(getNextToken(readbuf,alias,readbuf + strlen(readbuf)),alias, readbuf + strlen(readbuf));
		removeAlias(alias);
		destructList(&tokenList);addToHistory(readbuf);return 0;
	}
	if(!strcmp(commandName,"createalias")){
		char alias[100];
		const char* inst = getNextToken(getNextToken(readbuf,alias,readbuf + strlen(readbuf)),alias, readbuf + strlen(readbuf));
		createAlias((const char*)alias, inst);
		destructList(&tokenList);addToHistory(readbuf);return 0;
	}
	
	replaceAliasesInList(&tokenList);
	// visitList(&tokenList,&printtoken);
	ptr = listFront(&tokenList);
	commandName = getDataPointer(ptr);
	// printf("Command name is %s\n",commandName);
	if(!commandName){destructList(&tokenList);return 0;}
	if(!strcmp(commandName,"cd")){
		char* dir = getDataPointer(ptr = nextNode(ptr));
		if(nextNode(ptr)){
			printf("cd: too many arguments");
			destructList(&tokenList);return 0;
		}
		if(!dir)chdir("");
		else chdir(dir);
		destructList(&tokenList);addToHistory(readbuf);return 0;
	}
	if(!strcmp(commandName,"exit")){
		destructList(&tokenList);
		return 1;
	}
	if(!strcmp(commandName,"prev")){
		char* index_ptr = getDataPointer(ptr = nextNode(ptr));
		int index;
		if(!index_ptr)index = 1;else index = atoi(index_ptr);
		if(index<=0){
			printf("prev: Expected positive number or newline after prev\n");
			destructList(&tokenList);return 0;
		}

		char** inst = getDataPointer(getNodeWithIndex(&instructionHistory,index-1));
		if(!inst){
			printf("prev: Instruction could not be retrieved\n");
			destructList(&tokenList);return 0;
		}

		destructList(&tokenList);
		char answer='\0';
		// write(0,*inst,strlen(*inst)-1);
		// char buff[100];
		// scanf("%s\n",buff);
		// printf("%s\n",buff);
		while(answer != 'y' && answer != 'n'){
			printf("prev: \"%s\". y/n?\n",*inst);
			getchar();answer = getchar();getchar();
		}

		if(answer == 'y')interpretInstruction(*inst);
		


		return 0;
	}


	arguments[0] = commandName;
	const char* currentToken = getDataPointer(ptr = nextNode(ptr));
	while(currentToken){
		if(!strcmp(currentToken,"<")){
			if(!nextNode(ptr)){
				printf("Expected input file after \'<\'\n");
				destructList(&tokenList);return 0;
			}
			currentToken = getDataPointer(ptr = nextNode(ptr));
			possiblyCloseFile(IOfd[0]);
			IOfd[0] = open(currentToken,O_RDONLY|O_CREAT,S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
			if(IOfd[0]==-1){
				printf("Error in opening file \"%s\"\n",currentToken);
				destructList(&tokenList);return 0;
			}

		}
		else if(!strcmp(currentToken,">")){
			if(!nextNode(ptr)){
				printf("Expected output file after \'>\'\n");
				destructList(&tokenList);return 0;
			}
			currentToken = getDataPointer(ptr = nextNode(ptr));
			possiblyCloseFile(IOfd[1]);
			IOfd[1] = open(currentToken,O_WRONLY|O_CREAT|O_TRUNC,S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
			if(IOfd[1]==-1){
				printf("Error in opening file \"%s\"\n",currentToken);
				destructList(&tokenList);return 0;
			}
			// printf("File opened successfully\n");
		}
		else if(!strcmp(currentToken,"|")){
			if(!nextNode(ptr)){
				printf("Expected instruction after pipe \'|\'\n");
				destructList(&tokenList);return 0;
			}	
			int p[2];
			pipe(p);
			if(IOfd[1] == 1){forkExecute(IOfd[0],p[1],commandName,arguments,0);}	//If the stdout is 1, redirect to the following instruction
			else forkExecute(IOfd[0],IOfd[1],commandName,arguments,0);
			possiblyCloseFile(p[1]);
			IOfd[0] = p[0];
			IOfd[1] = 1;
			for(int i=0;i<argumentCounter;i++){
				arguments[i] = NULL;
			}
			argumentCounter = 1;
			commandName = getDataPointer(ptr = nextNode(ptr));
			arguments[0] = commandName;
		}
		else if(!strcmp(currentToken,";")){
			if(!nextNode(ptr)){
				printf("Expected instruction after \';\'\n");
				destructList(&tokenList);return 0;
			}
			forkExecute(IOfd[0],IOfd[1],commandName,arguments,1);
			for(int i=0;i<argumentCounter;i++){
				arguments[i] = NULL;
			}
			IOfd[0] = 0;
			IOfd[1] = 1;
			argumentCounter = 1;
			commandName = getDataPointer(ptr = nextNode(ptr));
			arguments[0] = commandName;
		}
		else if(!strcmp(currentToken,"&")){
			if(!nextNode(ptr)){
				printf("Expected instruction after \'&\'\n");
				destructList(&tokenList);return 0;
			}
			forkExecute(IOfd[0],IOfd[1],commandName,arguments,0);
			for(int i=0;i<argumentCounter;i++){
				arguments[i] = NULL;
			}
			IOfd[0] = 0;
			IOfd[1] = 1;
			argumentCounter = 1;
			commandName = getDataPointer(ptr = nextNode(ptr));
			arguments[0] = commandName;
		}
		else if(!strcmp(currentToken,">>")){
			if(!nextNode(ptr)){
				printf("Expected output file after \'>>\'\n");
				destructList(&tokenList);return 0;
			}
			currentToken = getDataPointer(ptr = nextNode(ptr));
			possiblyCloseFile(IOfd[1]);
			IOfd[1] = open(currentToken,O_WRONLY|O_CREAT|O_APPEND,S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
			if(IOfd[1]==-1){
				printf("Error in opening file \"%s\"\n",currentToken);
				destructList(&tokenList);return 0;
			}
			// printf("File opened successfully\n");
		}	
		else{
			arguments[argumentCounter++] = currentToken;
		}
		currentToken = getDataPointer(ptr = nextNode(ptr));
	}
	// printf("Exectuting %s\n",commandName);
	forkExecute(IOfd[0],IOfd[1],commandName,arguments,1);
	destructList(&tokenList);
	addToHistory(readbuf);
	return 0;
}



