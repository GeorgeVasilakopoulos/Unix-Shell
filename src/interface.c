#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include "structures/list.h"
#include "alias.h"
#include "interpreter.h"
#include "interface.h"

List instructionHistory;




void possiblyCloseFile(int filedes){
	if(filedes != 1 && filedes != 0)close(filedes);
}


//Fork the current process and execute the given command. Set waitForChild = 0 for parallel (backrgound execution)
void forkExecute(int inputfd, int outputfd, const char* commandName, const char* arguments[], int waitForChild){
	pid_t pid = fork();
	if(!pid){			//Child process
		dup2(inputfd,0);
		dup2(outputfd,1);
		int statusCode = execvp(commandName,(char * const*)arguments);
		printf("%s: Status code %d\n",commandName,statusCode);	//If error occured.
		exit(statusCode);
	}
	possiblyCloseFile(inputfd);
	possiblyCloseFile(outputfd);
	if(waitForChild){
		while(wait(NULL) != pid); //Wait for child process to terminate.
	}
}



void addToHistory(const char* inst){
	char* copyOfInst = malloc(sizeof(char)*(strlen(inst)+1));	//Storing copies!
	strcpy(copyOfInst,inst);
	if(listSize(&instructionHistory)>MAXHISTORY){	//Delete oldest instruction.
		free(*(char**)getDataPointer(listEnd(&instructionHistory)));
		listRemove(&instructionHistory,listEnd(&instructionHistory));
	}
	listPrepend(&instructionHistory,&copyOfInst);	//Add instruction to the front.
}


static void freePointerFromAddress(void** p){free(*p);}

//Delete all instructions in history.
void clearHistory(){
	visitList(&instructionHistory,(void (*)(void*))&freePointerFromAddress);
	destructList(&instructionHistory);
}



int main(){

	char* myshell = "in-mysh-now>";
	char buffer[MAXBUFSIZE];

	aliasInit();
	listInit(&instructionHistory,sizeof(char**));


	while(1){
		printf("%s",myshell);
		scanf(" %[^\n]",buffer);
		if(!strcmp(buffer,"\n"))continue;
		if(interpretInstruction(buffer))break;
	}

	clearHistory();

	destructAlias();
	return 0;
}


