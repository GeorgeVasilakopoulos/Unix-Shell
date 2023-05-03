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

const char* skipWhiteSpaces(const char* readbuf, const char* readbufSize){
	while(readbuf != readbufSize && isWhiteSpace(*readbuf))
		readbuf++;
	return readbuf;
}

int isAlpharethmetic(char c){
	return ((c>='a' && c<='z')||(c>='A' && c<='Z')||(c>='0' && c<='9')||(c=='/')||(c=='.')||(c=='-')||(c=='=')||(c=='$')||(c=='_'));
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
		if(!((*str>='a' && *str<='z')||(*str>='A' && *str<='Z')||(*str>='0' && *str<='9')||(*str == '_')))return 0;
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
	else if(isAlpharethmetic(*readbuf)||isWildCharacter(*readbuf)){
		while(readbuf != readbufSize && (isAlpharethmetic(*readbuf)||isWildCharacter(*readbuf))){
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


//0 for OK, 1 for syntax error (missing closing " or ')
int removeQuotations(List* tokenList){
	for(struct listnode* i = listFront(tokenList); i!=NULL; i = nextNode(i)){
		char* token = getDataPointer(i);
		if(rmQuotesFromString(token))return 1;	
	}
	return 0;
}















