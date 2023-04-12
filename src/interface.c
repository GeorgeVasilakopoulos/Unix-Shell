#include <stdio.h>
#include "structures/list.h"
#include "alias.h"
#include <string.h>
#include <stdlib.h>

extern List instructionHistory;




int interpretInstruction(char* readbuf);
void clearHistory();

int main(){

	aliasInit();
	char* myshell = "in-mysh-now>";
	char buffer[100];

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


