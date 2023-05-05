#include <stdio.h>
#include <string.h>
#include "structures/list.h"
#include "lexer.h"

int isWhiteSpace(char c){
	switch(c){
		case ' ':
		case '\t':
		case '\n':
			return 1;
		default:
			return 0;
	}
}


int isAlpharethmetic(char c){
	return (c>='a' && c<='z')||(c>='A' && c<='Z')||(c>='0' && c<='9');
}


int isValidCharacter(char c){
	return (isAlpharethmetic(c) || (c=='/') || (c=='.') || (c=='-') || (c=='=') || (c=='$') || (c=='_'));
}

int isQuotedString(const char* str){
	char firstChar = *str;
	if(firstChar == '"' || firstChar == '\''){
		const char* i;
		for(i = str+1; *i!=firstChar && *i!='\0';i++);
		if(*i == firstChar)return 1;
		else return 0;
	}
	return 0;
}

int isValidAlias(const char* str){
	while(*str!='\0'){
		if(!(isAlpharethmetic(*str)||(*str == '_')))return 0;
		str++;
	}
	return 1;
}	

int isSpecialCharacter(char c){
	switch(c){
		case '>':
		case '<':
		case '&':
		case '|':
		case ';':
			return 1;
		default:
			return 0;
	}
}

int isWildCharacter(char c){
	switch(c){
	case '*':
	case '?':
		return 1;
	default:
		return 0;
	}
}

//Returns a pointer to the next non white character of the string readbuf
const char* skipWhiteSpaces(const char* readbuf, const char* readbufSize){
	while(readbuf != readbufSize && isWhiteSpace(*readbuf))
		readbuf++;
	return readbuf;
}



//Copies the first token after readbuf into writebuf.
//Returns a pointer immediately after the last copied character.
//readbufSize is a pointer that points to the ending '\0' character of the string
const char* getNextToken(const char* readbuf, char* writebuf, const char* readbufSize){
	readbuf = skipWhiteSpaces(readbuf,readbufSize);
	if(readbuf == readbufSize)return readbuf;
	if(*readbuf == '"'){
		*writebuf++ = *readbuf++;
		while(readbuf != readbufSize && *readbuf != '"'){
			*writebuf++ = *readbuf++;
		}
		if(readbuf != readbufSize)*writebuf++ = *readbuf++;
	}
	else if(*readbuf == '\''){
		*writebuf++ = *readbuf++;
		while(readbuf != readbufSize && *readbuf != '\''){
			*writebuf++ = *readbuf++;
		}
		if(readbuf != readbufSize)*writebuf++ = *readbuf++;
	}
	else if(isValidCharacter(*readbuf)||isWildCharacter(*readbuf)){
		while(readbuf != readbufSize && (isValidCharacter(*readbuf)||isWildCharacter(*readbuf))){
			*writebuf++ = *readbuf++;
		}		
	}
	else if(isSpecialCharacter(*readbuf)){
		while(readbuf != readbufSize && isSpecialCharacter(*readbuf)){
			*writebuf++ = *readbuf++;
		}
	}
	

	*writebuf = '\0';
	return readbuf;
}



int rmQuotesFromString(char* str){
	if(*str == '"'){
		char* i;
		for(i = str+1; *i!='"' && *i!='\0';i++);
		if(*i=='\0')return 1;
		*i='\0';
	}
	else if(*str == '\''){
		char* i;
		for(i = str+1; *i!='\'' && *i!='\0';i++);
		if(*i=='\0')return 1;
		*i='\0';	
	}
	else return 0;
	memmove(str,str+1,strlen(str));
	return 0;
}



//Splits the string stored in readbuf into tokens
void createTokenList(const char* readbuf, List* tokenList){
	listInit(tokenList,sizeof(char)*MAXTOKENSIZE);
	const char* ptr = readbuf;
	int length = strlen(readbuf);
	char buffer[MAXTOKENSIZE];
	while(skipWhiteSpaces(ptr,length + readbuf) != length + readbuf){
		ptr = getNextToken(ptr,buffer, length + readbuf);
		listAppend(tokenList,buffer);
	}
}

//Removes quotations marks from a token list 
//0 for OK, 1 for syntax error (missing closing " or ')
int removeQuotations(List* tokenList){
	for(struct listnode* i = listFront(tokenList); i!=NULL; i = nextNode(i)){
		char* token = getDataPointer(i);
		if(rmQuotesFromString(token))return 1;	
	}
	return 0;
}















