#include "structures/list.h"
#include "wildchar.h"
#include "lexer.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <regex.h>
#include "interpreter.h"


int isWildString(const char* readbuf){
	while(*readbuf){
		if(*readbuf == '*'||*readbuf == '?') return 1;
		readbuf++;
	}
	return 0;
}


//Replace all instances of 'c' with the substring 'subst'
void replaceCharInString(char* string, char c, const char* subst){
	char buffer[100];
	int i=0;
	while(i<strlen(string)){
		if(c == string[i]){
			string[i] = '\0';
			strcpy(buffer,string); //Copy string right before 'c'
			strcat(buffer,subst);	//Concatenate substring
			strcat(buffer,string+1+i);	//Concatenate the rest after 'c'
			strcpy(string,buffer); //Replace string with result.
			i+=strlen(subst);		//Skip the characters of subst
		}
		else i++;
	}
}

void replaceWildTokens(List* tokenList){
	List dirList;	//List of files+directiories (produced by ls)
	int did_ls=0;	
	listInit(&dirList,sizeof(char)*50);
	for (struct listnode* i = listFront(tokenList); i!=NULL;){ //Iterate tokenList
		char pattern[100];
		pattern[0] = '^';
		strcpy(pattern+1,getDataPointer(i));	//Copy token string to 'pattern'
		if(isWildString(pattern)){

			//'pattern' becomes a valid regex pattern
			replaceCharInString(pattern,'*',"[a-zA-Z0-9_.+-]*");
			replaceCharInString(pattern,'?',"[a-zA-Z0-9_.+-]");
			replaceCharInString(pattern,'.',"\\.");
			strcat(pattern,"$");

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
				did_ls=1;	//Do not execute ls again...
			}

			//For every file/directory in dirList, check if it matches the pattern
			for(struct listnode* j=listFront(&dirList);j!=NULL;j = nextNode(j)){	
				int reg = regexec(&regex, getDataPointer(j), 0, NULL, 0);
				if(!reg){
					listAddBefore(tokenList,i,getDataPointer(j)); //Add the matched string right before the wild token.
				}
			}
			regfree(&regex);	//Deallocate regex 
			struct listnode* temp = i;
			i = nextNode(i);
			listRemove(tokenList,temp);	//Remove the wild token. All matched strings have been inserted
			continue;
		}
		i = nextNode(i);
	}
	destructList(&dirList);
}



