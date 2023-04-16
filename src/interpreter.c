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
#include "wildchar.h"
#include "interpreter.h"


#define MAXHISTORY 20

List instructionHistory;

void printtoken(void* data){
	printf("Token is %s\n", (char*)data);
}

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
	char* copyOfInst = malloc(sizeof(char)*(strlen(inst)+1));
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


static int Execute_destroyalias(const char* readbuf){
	char alias[100];
	char* ptr = getNextToken(readbuf,alias,readbuf + strlen(readbuf)); 	//Skip first word (destroyalias)
	getNextToken(ptr,alias,readbuf + strlen(readbuf));					//Copy second word (alias)
	removeAlias(alias);
	return 0;
}


static int Execute_createalias(const char* readbuf){
	char alias[100];
	char* ptr = getNextToken(readbuf,alias,readbuf + strlen(readbuf));		//Skip first word (createalias)
	const char* inst = getNextToken(ptr,alias,readbuf + strlen(readbuf));	//Copy second word (alias)
	createAlias(alias, inst);												
	return 0;
}

static int Execute_cd(const char* readbuf, struct listnode** ptr){
	char* dir = getDataPointer(*ptr = nextNode(*ptr));	
	if(nextNode(*ptr)){
		printf("cd: too many arguments");
		return 1;
	}
	if(!dir)chdir("");
	else chdir(dir);
	return 0;
}

static int Execute_prev(struct listnode** ptr){
	char* index_ptr = getDataPointer(*ptr = nextNode(*ptr));
	int index;
	if(!index_ptr)index = 1;else index = atoi(index_ptr); //If no number was given, execute the previous instruction
	if(index<=0){
		printf("prev: Expected positive number or newline after prev\n");
		return 1;
	}

	char** inst = getDataPointer(getNodeWithIndex(&instructionHistory,index-1));
	if(!inst){
		printf("prev: Instruction could not be retrieved\n");
		return 1;
	}

	char answer='\0';
	while(answer != 'y' && answer != 'n'){
		printf("prev: \"%s\". y/n?\n",*inst);	//Ask user whether to execute the instruction
		getchar();answer = getchar();getchar();
	}

	if(answer == 'y')interpretInstruction(*inst);
	return 0;
}


static int Handle_inputRedirection(struct listnode** ptr, const char** currentToken, int* IOfd){
	if(!nextNode(*ptr)){
		printf("Expected input file after \'<\'\n");
		return 1;
	}
	*currentToken = getDataPointer(*ptr = nextNode(*ptr));
	possiblyCloseFile(IOfd[0]); //Possibly close previous input file.
	IOfd[0] = open(*currentToken,O_RDONLY|O_CREAT,S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);	//Open new file
	if(IOfd[0]==-1){
		printf("Error in opening file \"%s\"\n",*currentToken);
		return 1;
	}
	return 0;
}


static int Handle_outputRedirection(struct listnode** ptr, const char** currentToken, int* IOfd){
	if(!nextNode(*ptr)){
		printf("Expected output file after \'>\'\n");
		return 1;
	}
	*currentToken = getDataPointer(*ptr = nextNode(*ptr));
	possiblyCloseFile(IOfd[1]); //Possibly close previous output file 
	IOfd[1] = open(*currentToken,O_WRONLY|O_CREAT|O_TRUNC,S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);	//Open new file
	if(IOfd[1]==-1){
		printf("Error in opening file \"%s\"\n",*currentToken);
		return 1;
	}
	return 0;
}

static int Handle_redirectionEOF(struct listnode** ptr, const char** currentToken, int* IOfd){
	if(!nextNode(*ptr)){
		printf("Expected output file after \'>>\'\n");
		return 1;
	}
	*currentToken = getDataPointer(*ptr = nextNode(*ptr));
	possiblyCloseFile(IOfd[1]);
	IOfd[1] = open(*currentToken,O_WRONLY|O_CREAT|O_APPEND,S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
	if(IOfd[1]==-1){
		printf("Error in opening file \"%s\"\n",*currentToken);
		return 1;
	}
	return 0;
}


static int Handle_pipe(struct listnode**ptr, const char** commandName,  const char** arguments, int* argumentCounter, int* IOfd){
	if(!nextNode(*ptr)){
		printf("Expected instruction after pipe \'|\'\n");
		return 1;
	}	
	int p[2];
	pipe(p);
	if(IOfd[1] == 1){forkExecute(IOfd[0],p[1],*commandName,arguments,0);}	//If the stdout is 1, redirect to the following instruction
	else forkExecute(IOfd[0],IOfd[1],*commandName,arguments,0);
	possiblyCloseFile(p[1]);
	IOfd[0] = p[0];
	IOfd[1] = 1;
	for(int i=0;i<*argumentCounter;i++){
		arguments[i] = NULL;
	}

	//Fetch new command
	*argumentCounter = 1;
	*commandName = getDataPointer(*ptr = nextNode(*ptr));
	arguments[0] = *commandName;
	return 0;
}

static int Handle_semicolon(struct listnode**ptr, const char** commandName,  const char** arguments, int* argumentCounter, int* IOfd){
	if(!nextNode(*ptr)){
		printf("Expected instruction after \';\'\n");
		return 1;
	}
	forkExecute(IOfd[0],IOfd[1],*commandName,arguments,1);
	for(int i=0;i<*argumentCounter;i++){
		arguments[i] = NULL;
	}
	IOfd[0] = 0;
	IOfd[1] = 1;
	*argumentCounter = 1;
	*commandName = getDataPointer(*ptr = nextNode(*ptr));
	arguments[0] = *commandName;
	return 0;
}


static int Handle_backgroundExecution(struct listnode** ptr, const char** commandName, const char** arguments, int* argumentCounter, int* IOfd){
	if(!nextNode(*ptr)){
		printf("Expected instruction after \'&\'\n");
		return 1;
	}
	forkExecute(IOfd[0],IOfd[1],*commandName,arguments,0);
	for(int i=0;i<*argumentCounter;i++){
		arguments[i] = NULL;
	}

	//Fetch new instruction
	IOfd[0] = 0;	//
	IOfd[1] = 1;	//
	*argumentCounter = 1;
	*commandName = getDataPointer(*ptr = nextNode(*ptr));
	arguments[0] = *commandName;
	return 0;
}

int interpretInstruction(const char* readbuf){
	
	#define EXIT(exitCode)						\
	{											\
		destructList(&tokenList);				\
		return exitCode;						\
	}											


	int length = strlen(readbuf);
	const char* commandName;
	const char* arguments[100]={};
	int argumentCounter = 1;
	int IOfd[2] = {0,1};
	List tokenList;
	listInit(&tokenList,sizeof(char)*50);
	createTokenList(readbuf,&tokenList);
	struct listnode* ptr = listFront(&tokenList); //Get first token (corresponds to commandName)
	commandName = getDataPointer(ptr);
	if(!commandName){							//If list is empty (readbuf contains no tokens)
		EXIT(0);
	}
	if(!strcmp(commandName,"destroyalias")){
		Execute_destroyalias(readbuf);
		addToHistory(readbuf);
		EXIT(0);
	}
	if(!strcmp(commandName,"createalias")){
		Execute_createalias(readbuf);
		addToHistory(readbuf);
		EXIT(0);
	}
	
	replaceAliasesInList(&tokenList);	///
	replaceWildTokens(&tokenList);		///

	ptr = listFront(&tokenList);
	commandName = getDataPointer(ptr);

	if(!commandName){
		EXIT(0);
	}
	if(!strcmp(commandName,"cd")){
		if(!Execute_cd(readbuf,&ptr))
			addToHistory(readbuf);
		EXIT(0);
	}
	if(!strcmp(commandName,"exit")){	//Terminate execution of the shell
		EXIT(1);
	}
	if(!strcmp(commandName,"prev")){
		Execute_prev(&ptr);
		EXIT(0);
	}


	arguments[0] = commandName;
	const char* currentToken = getDataPointer(ptr = nextNode(ptr)); //Next token after instrution name
	while(currentToken){
		if(!strcmp(currentToken,"<")){
			if(Handle_inputRedirection(&ptr,&currentToken,IOfd)){
				EXIT(0);
			}
		}
		else if(!strcmp(currentToken,">")){
			if(Handle_outputRedirection(&ptr,&currentToken,IOfd)){
				EXIT(0);
			}	
		}
		else if(!strcmp(currentToken,">>")){
			if(Handle_redirectionEOF(&ptr,&currentToken,IOfd)){
				EXIT(0);
			}
		}	
		else if(!strcmp(currentToken,"|")){
			if(Handle_pipe(&ptr,&commandName,arguments,&argumentCounter,IOfd)){
				EXIT(0);
			}
		}
		else if(!strcmp(currentToken,";")){
			if(Handle_semicolon(&ptr,&commandName,arguments,&argumentCounter,IOfd)){
				EXIT(0);
			}
		}
		else if(!strcmp(currentToken,"&")){
			if(Handle_backgroundExecution(&ptr,&commandName,arguments,&argumentCounter,IOfd)){
				EXIT(0);
			}
		}
		else{
			arguments[argumentCounter++] = currentToken;	//Else, consider this token as argument
		}
		currentToken = getDataPointer(ptr = nextNode(ptr));	//Get next token
	}
	forkExecute(IOfd[0],IOfd[1],commandName,arguments,1);	//Execute
	addToHistory(readbuf);
	EXIT(0);
}



