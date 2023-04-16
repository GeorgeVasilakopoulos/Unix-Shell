#include <stdio.h>
#include "structures/list.h"
#include "alias.h"
#include "interpreter.h"
#include <string.h>
#include <stdlib.h>

extern List instructionHistory;




int main(){

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


