#include <stdio.h>
#include "structures/list.h"
#include "alias.h"
#include<stdlib.h>
static List instructionHistory;






void interpretInstruction(char* readbuf);


int main(){

	aliasInit();
	char* myshell = "in-mysh-now>";
	char buffer[100];



	while(1){
		printf("%s",myshell);
		scanf(" %[^\n]",buffer);
		interpretInstruction(buffer);
	}

	destructAlias();
	return 0;
}


