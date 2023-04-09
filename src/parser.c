#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

char* skipWhiteSpaces(char* readbuf, char* readbufSize){
	while(readbuf != readbufSize && (*readbuf == ' ' || *readbuf == '\t' || *readbuf == '\n'))
		readbuf++;
	return readbuf;
}

int isAlpharethmetic(char c){
	return ((c>='a' && c<='z')||(c>='A' && c<='Z')||(c>='0' && c<='9')||(c=='/')||(c=='.')||(c=='-'));
}

int isSpecialCharacter(char c){
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



char* getNextToken(char* readbuf, char* writebuf, char* readbufSize){
	readbuf = skipWhiteSpaces(readbuf,readbufSize);
	if(readbuf == readbufSize)return readbuf;
	if(*readbuf == '"'){
		while(readbuf != readbufSize && *readbuf != '"'){
			*writebuf++ = *readbuf++;
		}
	}
	else if(*readbuf == '\''){
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


void forkExecute(int inputfd, int outputfd, char* commandName, char** arguments, int waitForChild){
	pid_t pid = fork();
	if(!pid){
		dup2(inputfd,0);
		dup2(outputfd,1);
		int statusCode = execvp(commandName,arguments);
		printf("Status code %d\n",statusCode);
		exit(statusCode);
	}
	printf("waiting for %s\n", commandName);
	possiblyCloseFile(inputfd);
	possiblyCloseFile(outputfd);
	if(waitForChild){
		while(wait(NULL) != pid);
	}
}





void interpretInstruction(char* readbuf){
	int length = strlen(readbuf);
	char commandName[100];
	char* arguments[100]={};
	int argumentCounter = 1;
	int IOfd[2] = {0,1};
	char* ptr = getNextToken(readbuf,commandName,length+readbuf);
	if(!strcmp(commandName,"cd")){
		ptr = getNextToken(ptr,commandName,length+readbuf);
		chdir(commandName);
		if(skipWhiteSpaces(ptr,length + readbuf) != length + readbuf){
			printf("cd: too many arguments");
		}
		return;
	}


	printf("Command name is %s\n",commandName);
	arguments[0] = malloc(sizeof(char)*(strlen(commandName)+1));
	strcpy(arguments[0],commandName);


	char buffer[100]= {};
	while(skipWhiteSpaces(ptr,length + readbuf) != length + readbuf){
		ptr = getNextToken(ptr,buffer,length+readbuf);
		printf("buffer is %s\n", buffer);
		if(!strcmp(buffer,"<")){
			if(skipWhiteSpaces(ptr,length + readbuf) == length + readbuf){
				printf("Expected input file after \'<\'\n");
				return;
			}
			ptr = getNextToken(ptr,buffer,length+readbuf);
			possiblyCloseFile(IOfd[0]);
			IOfd[0] = open(buffer,O_RDONLY);
			if(IOfd[0]==-1){
				printf("Error in opening file \"%s\"\n",buffer);
				return;
			}

		}
		else if(!strcmp(buffer,">")){
			if(skipWhiteSpaces(ptr,length + readbuf) == length + readbuf){
				printf("Expected output file after \'>\'\n");
				return;
			}
			ptr = getNextToken(ptr,buffer,length+readbuf);
			possiblyCloseFile(IOfd[1]);
			IOfd[1] = open(buffer,O_RDWR|O_CREAT);
			if(IOfd[1]==-1){
				printf("Error in opening file \"%s\"\n",buffer);
				return;
			}
			printf("File opened successfully\n");
		}
		else if(!strcmp(buffer,"|")){
			if(skipWhiteSpaces(ptr,length + readbuf) == length + readbuf){
				printf("Expected instruction after pipe \'|\'\n");
				return;
			}	
			int p[2];
			pipe(p);
			if(IOfd[1] == 1){printf("here we are!\n"); forkExecute(IOfd[0],p[1],commandName,arguments,0);}	//If the stdout is 1, redirect to the following instruction
			else forkExecute(IOfd[0],IOfd[1],commandName,arguments,0);
			possiblyCloseFile(p[1]);
			IOfd[0] = p[0];
			IOfd[1] = 1;
			for(int i=0;i<argumentCounter;i++){
				free(arguments[i]);
				arguments[i] = NULL;
			}
			argumentCounter = 1;
			ptr = getNextToken(ptr,commandName,length+readbuf);
			arguments[0] = malloc(sizeof(char)*(strlen(commandName)+1));
			strcpy(arguments[0],commandName);
		}
		else if(!strcmp(buffer,"&")){
			if(skipWhiteSpaces(ptr,length + readbuf) == length + readbuf){
				printf("Expected instruction after \'&\'\n");
				return;
			}
		}
		else{
			arguments[argumentCounter] = malloc(sizeof(char)*(strlen(buffer)+1));
			strcpy(arguments[argumentCounter++],buffer);
		}
	}
	printf("Exectuting %s\n",commandName);
	forkExecute(IOfd[0],IOfd[1],commandName,arguments,1);
}





int main(){


	// char* argument_list[] = {"ls", "-l", NULL};

	// if (fork() == 0) {
    //     // Newly spawned child Process. This will be taken over by "ls -l"
    //     int status_code = execvp("ls", argument_list);

    //     printf("ls -l has taken control of this child process. This won't execute unless it terminates abnormally!\n");

    //     if (status_code == -1) {
    //         printf("Terminated Incorrectly\n");
    //         return 1;
    //     }
    // }
    // else {
    //     // Old Parent process. The C program will come here
    //     printf("This line will be printed\n");
    // }
    // char buffer[]= "ls"; 
	// interpretInstruction(buffer);

	char buffer3[] = "cat test.txt test.txt test.txt | sort";
	interpretInstruction(buffer3);


	// char buffer2[] = "cd mydir"
    // printf("%d\n", isSpecialCharacter('>'));
	// execvp("echo", buffer);

	return 0;
}









