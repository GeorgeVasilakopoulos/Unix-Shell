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


const char* skipWhiteSpaces(const char* readbuf, const char* readbufSize){
	while(readbuf != readbufSize && (*readbuf == ' ' || *readbuf == '\t' || *readbuf == '\n'))
		readbuf++;
	return readbuf;
}

int isAlpharethmetic(const char c){
	return ((c>='a' && c<='z')||(c>='A' && c<='Z')||(c>='0' && c<='9')||(c=='/')||(c=='.')||(c=='-'));
}

int isSpecialCharacter(const char c){
	switch(c){
		case '>':
		case '<':
		case '&':
		case '|':
			return 1;
		default:
			return 0;
	}
}



const char* getNextToken(const char* readbuf, char* writebuf, const char* readbufSize){
	readbuf = skipWhiteSpaces(readbuf,readbufSize);
	if(readbuf == readbufSize)return readbuf;
	if(*readbuf == '"'){
		readbuf++;
		while(readbuf != readbufSize && *readbuf != '"'){
			*writebuf++ = *readbuf++;
		}
	}
	else if(*readbuf == '\''){
		readbuf++;
		while(readbuf != readbufSize && *readbuf != '\''){
			*writebuf++ = *readbuf++;
		}
	}
	else if(isAlpharethmetic(*readbuf)){
		while(readbuf != readbufSize && isAlpharethmetic(*readbuf)){
			*writebuf++ = *readbuf++;
		}		
	}
	else{
		while(readbuf != readbufSize && isSpecialCharacter(*readbuf)){
			*writebuf++ = *readbuf++;
		}
	}

	//if(readbuf == readbufSize)return NULL;
	*writebuf = '\0';
	return readbuf;
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
	}
}





void createTokenList(const char* readbuf, List* tokenList){
	listInit(tokenList,sizeof(char)*100);
	const char* ptr = readbuf;
	int length = strlen(readbuf);
	char buffer[100];
	while(skipWhiteSpaces(ptr,length + readbuf) != length + readbuf){
		ptr = getNextToken(ptr,buffer, length + readbuf);
		listAppend(tokenList,buffer);
	}
}


void printList(List* tokenList){
	struct listnode* node = listFront(tokenList);
	printf("List size is %d\n",listSize(tokenList));
	for(int i=0;i<listSize(tokenList) && node; i++){
		printf("Node %d: %s\n",i,(char*)getDataPointer(node));
		node = nextNode(node);
	}
}


void replaceAliasesInList(List* tokenList){
	List result;
	listInit(&result,sizeof(char)*50);
	const char* alias;
	for(struct listnode* i = listFront(tokenList); i != NULL; i = nextNode(i)){
		if(alias = findAlias(getDataPointer(i))){
			printf("found %s\n",alias);
			// exit(1);
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



void interpretInstruction(char* readbuf){
	int length = strlen(readbuf);
	const char* commandName;
	const char* arguments[100]={};
	int argumentCounter = 1;
	int IOfd[2] = {0,1};

	List tokenList;
	listInit(&tokenList,sizeof(char)*50);
	createTokenList(readbuf,&tokenList);
	printList(&tokenList);
	replaceAliasesInList(&tokenList);
	printList(&tokenList);
	struct listnode* ptr = listFront(&tokenList);
	commandName = getDataPointer(ptr);
	if(!commandName){destructList(&tokenList);return;}

	if(!strcmp(commandName,"cd")){
		char* dir = getDataPointer(ptr = nextNode(ptr));
		if(!dir)chdir("");
		else chdir(dir);
		if(nextNode(ptr)){
			printf("cd: too many arguments");
		}
		destructList(&tokenList);return;
	}
	else if(!strcmp(commandName,"createalias")){
		char alias[100];
		const char* inst = getNextToken(getNextToken(readbuf,alias,readbuf + strlen(readbuf)),alias, readbuf + strlen(readbuf));
		createAlias((const char*)alias, inst);
		destructList(&tokenList);return;
	}
	else if(!strcmp(commandName,"destroyalias")){
		char alias[100];
		printf("byebye\n");
		getNextToken(getNextToken(readbuf,alias,readbuf + strlen(readbuf)),alias, readbuf + strlen(readbuf));
		removeAlias(alias);
		destructList(&tokenList);return;
	}


	arguments[0] = commandName;
	const char* currentToken = getDataPointer(ptr = nextNode(ptr));
	while(currentToken){
		if(!strcmp(currentToken,"<")){
			if(!nextNode(ptr)){
				printf("Expected input file after \'<\'\n");
				destructList(&tokenList);return;
			}
			currentToken = getDataPointer(ptr = nextNode(ptr));
			possiblyCloseFile(IOfd[0]);
			IOfd[0] = open(currentToken,O_RDONLY);
			if(IOfd[0]==-1){
				printf("Error in opening file \"%s\"\n",currentToken);
				destructList(&tokenList);return;
			}

		}
		else if(!strcmp(currentToken,">")){
			if(!nextNode(ptr)){
				printf("Expected output file after \'>\'\n");
				destructList(&tokenList);return;
			}
			currentToken = getDataPointer(ptr = nextNode(ptr));
			possiblyCloseFile(IOfd[1]);
			IOfd[1] = open(currentToken,O_RDWR|O_CREAT);
			if(IOfd[1]==-1){
				printf("Error in opening file \"%s\"\n",currentToken);
				destructList(&tokenList);return;
			}
			printf("File opened successfully\n");
		}
		else if(!strcmp(currentToken,"|")){
			if(!nextNode(ptr)){
				printf("Expected instruction after pipe \'|\'\n");
				destructList(&tokenList);return;
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
		else if(!strcmp(currentToken,"&")){
			if(!nextNode(ptr)){
				printf("Expected instruction after \'&\'\n");
				destructList(&tokenList);return;
			}
		}
		else{
			arguments[argumentCounter++] = currentToken;
		}
		currentToken = getDataPointer(ptr = nextNode(ptr));
	}
	printf("Exectuting %s\n",commandName);
	forkExecute(IOfd[0],IOfd[1],commandName,arguments,1);
	destructList(&tokenList);
}





int main(){

	aliasInit();


	char buffer3[] = "createalias haha echo \"This is my favourite part\"";
	interpretInstruction(buffer3);

    char buffer[]= "haha"; 
	interpretInstruction(buffer);

	char buffer2[] = "createalias haha2 haha > myfile.txt";
	interpretInstruction(buffer2);

	interpretInstruction("haha2");

    // printf("%d\n", isSpecialCharacter('>'));
	// execvp("echo", buffer);
	destructAlias();
	return 0;
}









