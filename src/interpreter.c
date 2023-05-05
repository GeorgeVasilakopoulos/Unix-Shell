#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "structures/list.h"
#include "alias.h"
#include "lexer.h"
#include "wildchar.h"
#include "interpreter.h"
#include "interface.h"


static int Execute_destroyalias(const char* readbuf){
	char alias[MAXTOKENSIZE];
	const char* endPointer = readbuf + strlen(readbuf);
	const char* ptr = getNextToken(readbuf,alias,endPointer); 	//Skip first word 'destroyalias'
	ptr = getNextToken(ptr,alias,endPointer);					//Copy second word (alias)
	if(skipWhiteSpaces(ptr,endPointer) != endPointer && *skipWhiteSpaces(ptr,endPointer) != ';'){
		printf("destroyalias: too many arguments\n");
		return 1;
	}
	removeAlias(alias);		//Even if it does not exist, still return 0
	return 0;
}


static int Execute_createalias(const char* readbuf){
	char alias[MAXTOKENSIZE];
	const char* endPointer = readbuf + strlen(readbuf);
	const char* ptr = getNextToken(readbuf,alias,endPointer);		//Skip first word (createalias)
	const char* inst = getNextToken(ptr,alias,endPointer);			//Copy second word (alias)
	if(!isValidAlias(alias)){
		printf("createalias: invalid alias name\n");
		return 1;
	}

	//Perhaps replacing another alias
	createAlias(alias, inst);								
	return 0;
}

static int Execute_cd(const char** arguments){
	if(arguments[2]){
		printf("cd: too many arguments\n");
		return 1;
	}
	const char* dir = arguments[1];
	if(!dir)chdir("");
	else chdir(dir);
	return 0;
}

static int Execute_prev(struct listnode** ptr){
	int index;
	char* index_ptr = getDataPointer(*ptr = nextNode(*ptr));

	//If no number was given, execute the previous instruction
	if(!index_ptr)index = 1;else index = atoi(index_ptr); 
	if(index<=0){
		printf("prev: Expected positive number or newline after prev\n");
		return 1;
	}
	const char* previousInstruction = fetchFromHistory(index);
	if(!previousInstruction){
		printf("prev: Instruction could not be retrieved\n");
		return 1;
	}
	interpretInstruction(previousInstruction,0);
	return 0;
}

static void Execute_myHistory(){
	printHistory(); //Defined in interface.c
}


static int Handle_inputRedirection(struct listnode** ptr, const char** currentToken, int* IOfd){
	if(!nextNode(*ptr)){
		printf("Expected input file after \'<\'\n");
		return 1;
	}
	*currentToken = getDataPointer(*ptr = nextNode(*ptr));
	possiblyCloseFile(IOfd[0]); //Possibly close previous input file.

	//Open new file (possibly create one)
	IOfd[0] = open(*currentToken,O_RDONLY|O_CREAT,S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);	
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
	
	//Possibly close previous output file
	possiblyCloseFile(IOfd[1]);  

	//Open new file (possibly create one)
	IOfd[1] = open(*currentToken,O_WRONLY|O_CREAT|O_TRUNC,S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);	
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
	if(pipe(p) < 0){
		printf("Error in opening pipe\n");
		return 1;
	}
	if(IOfd[1] == 1){forkExecute(IOfd[0],p[1],*commandName,arguments,0);}	//If the stdout is 1, redirect to the following instruction
	else forkExecute(IOfd[0],IOfd[1],*commandName,arguments,0);				//Else, write to the previously set output fd
	possiblyCloseFile(p[1]);
	possiblyCloseFile(IOfd[1]);
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
	int return_immediately = 0;
	if(!nextNode(*ptr)){
		return_immediately = 1;
	}
	if(!strcmp(*commandName,"cd"))Execute_cd(arguments);
	else forkExecute(IOfd[0],IOfd[1],*commandName,arguments,1);
	if(return_immediately)return 1;
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
	int return_immediately = 0;
	if(!nextNode(*ptr)){
		return_immediately = 1;
	}
	if(!strcmp(*commandName,"cd"))Execute_cd(arguments);
	else forkExecute(IOfd[0],IOfd[1],*commandName,arguments,0);
	if(return_immediately) return 1;
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

void replaceEnvVariables(List *tokenList){
	for(struct listnode* i=listFront(tokenList); i!=NULL;i = nextNode(i)){
		char* token = getDataPointer(i);
		if(*token == '$'){
			strcpy(token,getenv(token+1));
		}
	}
}





int interpretInstruction(const char* readbuf, int storeInHistory){
	
	#define EXIT(exitCode)						\
	{											\
		destructList(&tokenList);				\
		return exitCode;						\
	}											

	const char* commandName;
	const char* arguments[MAXARGUMENTS]={};
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
		if(storeInHistory)addToHistory(readbuf);
		EXIT(0);
	}
	if(!strcmp(commandName,"createalias")){
		Execute_createalias(readbuf);
		if(storeInHistory)addToHistory(readbuf);
		EXIT(0);
	}
	
	replaceAliasesInList(&tokenList);	//First, replace aliases
	replaceWildTokens(&tokenList);		
	replaceEnvVariables(&tokenList);	
	removeQuotations(&tokenList);		//Remove quotations last

	ptr = listFront(&tokenList);
	commandName = getDataPointer(ptr);

	if(!commandName){	//No command
		EXIT(0);
	}
	if(!strcmp(commandName,"exit")){	//Terminate execution of the shell
		EXIT(1);
	}
	if(!strcmp(commandName,"prev")){
		Execute_prev(&ptr);
		EXIT(0);
	}
	if(!strcmp(commandName,"myHistory")){
		Execute_myHistory();
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
		else if(!strcmp(currentToken,"&;")){
			if(Handle_backgroundExecution(&ptr,&commandName,arguments,&argumentCounter,IOfd)){
				EXIT(0);
			}
		}
		else{
			arguments[argumentCounter++] = currentToken;	//Else, consider this token as argument
		}
		currentToken = getDataPointer(ptr = nextNode(ptr));	//Get next token
	}
	if(!strcmp(commandName,"cd"))Execute_cd(arguments);
	else forkExecute(IOfd[0],IOfd[1],commandName,arguments,1);	//Execute
	if(storeInHistory)addToHistory(readbuf);
	EXIT(0);
}



