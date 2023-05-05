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

static List instructionHistory;


void possiblyCloseFile(int filedes){
	if(filedes != 1 && filedes != 0)close(filedes);
}

	

//Fork the current process and execute the given command. Set waitForChild = 0 for backrgound execution
void forkExecute(int inputfd, int outputfd, const char* commandName, const char* arguments[], int waitForChild){
	pid_t pid = fork();
	if(!pid){			//Child process

		//If this is a background instruction, ignore the signals
		if(!waitForChild){		
			signal(SIGINT,SIG_IGN);
			signal(SIGTSTP,SIG_IGN);
		}

		dup2(inputfd,0);
		dup2(outputfd,1);
		int statusCode = execvp(commandName,(char * const*)arguments);
		printf("%s: Command Not Found\n",commandName);	//If error occured.
		exit(statusCode);
	}
	possiblyCloseFile(inputfd);
	possiblyCloseFile(outputfd);
	if(waitForChild){
		while(waitpid(pid, NULL, WUNTRACED) != pid); //Wait for child process to terminate.
	}

}



void addToHistory(const char* inst){
	char* copyOfInst = malloc(sizeof(char)*(strlen(inst)+1));	//Storing copies!
	strcpy(copyOfInst,inst);

	//Delete oldest instruction.
	if(listSize(&instructionHistory)>MAXHISTORY){	
		free(*(char**)getDataPointer(listEnd(&instructionHistory)));
		listRemove(&instructionHistory,listEnd(&instructionHistory));
	}
	listPrepend(&instructionHistory,&copyOfInst);	//Add instruction to the front.
}

//Print instruction history: Oldest First
void printHistory(){
	int counter=listSize(&instructionHistory);
	for(struct listnode* i = listEnd(&instructionHistory); i !=NULL;i = previousNode(i)){
		printf("%3d: %s\n",counter--,*(char**)getDataPointer(i));
	}
}


//Fetch instruction by index. 1 means previous
const char* fetchFromHistory(int index){
	char** previousInstruction = getDataPointer(getNodeWithIndex(&instructionHistory,index-1));
	if(!previousInstruction)return NULL;	//Unable to fetch instruction
	return *previousInstruction;
}


static void freePointerFromAddress(void** p){free(*p);}

//Delete all instructions in history.
void clearHistory(){
	visitList(&instructionHistory,(void (*)(void*))&freePointerFromAddress);
	destructList(&instructionHistory);
}

void signalHandler(int sigval){
	
	//Simply ignore signals SIGINT and SIGTSTP
	if(sigval == SIGINT){
		signal(SIGINT,signalHandler);
	}
	else if(sigval == SIGTSTP){
		signal(SIGTSTP,signalHandler);
	}

	//When a child process terminates (bg/fg), the parent reaps them
	else if(sigval == SIGCHLD){	
		while(waitpid(-1,NULL,WNOHANG) > 0);
	}
}



int main(){

	char* myshell = "in-mysh-now>";
	char buffer[MAXBUFSIZE];

	aliasInit();
	listInit(&instructionHistory,sizeof(char**));

	signal(SIGINT,signalHandler);
	signal(SIGTSTP,signalHandler);
	signal(SIGCHLD,signalHandler);
	
	printf("%s",myshell);
	while(1){
		scanf(" %[^\n]",buffer);
		if(!strcmp(buffer,"\n"))continue;
		if(interpretInstruction(buffer,1))break;
		printf("%s",myshell);
	}

	clearHistory();
	destructAlias();
	return 0;
}


