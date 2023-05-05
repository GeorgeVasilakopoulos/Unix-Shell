#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <regex.h>
#include "structures/list.h"
#include "lexer.h"
#include "interface.h"
#include "wildchar.h"



int isWildString(const char* readbuf){
	if(isQuotedString(readbuf))return 0;	//Quoted tokens should not be changed
	while(*readbuf){
		if(isWildCharacter(*readbuf)) return 1;
		if(isWhiteSpace(*readbuf)) return 0;
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

int replaceWildTokens(List* tokenList){
	List dirList;	//List of files+directiories (produced by ls)
	listInit(&dirList,sizeof(char)*MAXTOKENSIZE);
	int did_ls=0;	

	//Iterate tokenList
	for (struct listnode* i = listFront(tokenList); i!=NULL;){ 
		if(isWildString(getDataPointer(i))){
			char pattern[MAXBUFSIZE];
			pattern[0] = '^';
			strcpy(pattern+1,getDataPointer(i));	//Copy token string to 'pattern'

			//Transforming 'pattern' into a valid regex pattern
			replaceCharInString(pattern,'*',"[a-zA-Z0-9_.+-]*");
			replaceCharInString(pattern,'?',"[a-zA-Z0-9_.+-]");
			replaceCharInString(pattern,'.',"\\.");
			strcat(pattern,"$");
			regex_t regex;
			int code = regcomp(&regex,pattern,REG_EXTENDED);
			if(code){
				printf("wildchar: error in the comprehension of %s\n",(char*)getDataPointer(i));
				return 1;
			}


			//Execute an ls, store results as tokens in dirList
			if(!did_ls){	
				int p[2];
				if(pipe(p) < 0){
					printf("Error in opening pipe\n");
					return 1;
				}
				const char* arg[2] = {"ls",NULL};

				//Result of ls will be produced in p[1]
				forkExecute(0,p[1],"ls",arg,1);			
				char buffer[MAX_CHARACTERS_PER_LS]={};

				//Copy result to buffer
				read(p[0],buffer,MAX_CHARACTERS_PER_LS);	
				possiblyCloseFile(p[0]);
				possiblyCloseFile(p[1]);

				//Create the token list
				createTokenList(buffer,&dirList);		
				did_ls=1;	//Do not execute ls again...
			}

			//For every file/directory in dirList, check if it matches the pattern
			for(struct listnode* j=listFront(&dirList);j!=NULL;j = nextNode(j)){	
				int match_code = regexec(&regex, getDataPointer(j), 0, NULL, 0);
				if(!match_code){
					listAddBefore(tokenList,i,getDataPointer(j)); //Add the matched string right before the wild token.
				}
			}

			//Deallocate regex
			regfree(&regex);	 
			struct listnode* temp = i;
			i = nextNode(i);
			listRemove(tokenList,temp);	//Remove the wild token. All matched strings have been inserted
			continue;
		}
		i = nextNode(i);
	}
	destructList(&dirList);
	return 0;
}



