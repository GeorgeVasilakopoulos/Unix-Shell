#include "structures/list.h"
#include "lexer.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <regex.h>

void printtoken(void* data);
void possiblyCloseFile(int filedes);
void forkExecute(int inputfd, int outputfd, const char* commandName, const char* arguments[], int waitForChild);

int isWildString(const char* readbuf){
	while(*readbuf){
		if(*readbuf == '*'||*readbuf == '?') return 1;
		readbuf++;
	}
	return 0;
}

void replaceCharInString(char* string, char c, const char* subst){
	char buffer[100];
	int i=0;
	while(i<strlen(string)){
		if(c == string[i]){
			string[i] = '\0';
			strcpy(buffer,string);
			strcat(buffer,subst);
			strcat(buffer,string+1+i);
			strcpy(string,buffer);
			i+=strlen(subst);
		}
		else i++;
	}
}

void replaceWildTokens(List* tokenList){
	List dirList;
	int did_ls=0;
	listInit(&dirList,sizeof(char)*50);
	for (struct listnode* i = listFront(tokenList); i!=NULL;){
		char pattern[100];
		pattern[0] = '^';
		strcpy(pattern+1,getDataPointer(i));
		if(isWildString(pattern)){
			replaceCharInString(pattern,'*',"[a-zA-Z0-9_.+-]*");
			replaceCharInString(pattern,'?',"[a-zA-Z0-9_.+-]");
			replaceCharInString(pattern,'.',"\.");
			strcat(pattern,"$");
			// printf("pattern %s\n",pattern);

			regex_t regex;
			int reg = regcomp(&regex,pattern,REG_EXTENDED);
			//error check


			if(!did_ls){	//Execute an ls, store results as tokens in dirList
				int p[2];
				pipe(p);
				const char* arg[2] = {"ls",NULL};
				forkExecute(0,p[1],"ls",arg,1);
				char buffer[1000000];
				read(p[0],buffer,100000);
				buffer[strlen(buffer)] = '\0';
				possiblyCloseFile(p[0]);
				possiblyCloseFile(p[1]);
				createTokenList(buffer,&dirList);
				did_ls=1;
			}

			//For every file/directory in dirList, see if it matches the pattern
			for(struct listnode* j=listFront(&dirList);j!=NULL;j = nextNode(j)){	
				int reg = regexec(&regex, getDataPointer(j), 0, NULL, 0);
				if(!reg){
					printf("Match! %s\n",(char*)getDataPointer(j));
					listAddBefore(tokenList,i,getDataPointer(j));
				}
			}
			regfree(&regex);
			struct listnode* temp = i;
			i = nextNode(i);
			listRemove(tokenList,temp);	//Remove the wild token.
			continue;
		}
		i = nextNode(i);
	}
	destructList(&dirList);
}












int main(){
	List mylist;
	listInit(&mylist,sizeof(char)*50);
	createTokenList("*.t?t",&mylist);
	printf("hey");
	visitList(&mylist,&printtoken);
	replaceWildTokens(&mylist);
	visitList(&mylist,&printtoken);
	// char buffer[100];
	// strcpy(buffer,"*.txt");
	// replaceCharInString(buffer,'*',"^[a-zA-Z0-9_.+-]*");
	// printf("%s\n",buffer);

	// regex_t regex;
	// int reg = regcomp(&regex,"^[a-zA-Z0-9_.+-]*\.txt$",REG_EXTENDED);	
	// if(!reg){
	// 	reg = regexec(&regex, "haha.txt", 0, NULL, 0);
	// }

	// if (reg == 0) {
    //     printf("Match!\n");
    // } 
   	// else if (reg == REG_NOMATCH) {
    //     printf("No match.\n");
    // }


	return 0;

}