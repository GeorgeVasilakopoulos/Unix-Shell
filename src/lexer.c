#include <stdio.h>
#include <string.h>
#include "structures/list.h"
#include "lexer.h"

const char* skipWhiteSpaces(const char* readbuf, const char* readbufSize){
	while(readbuf != readbufSize && (*readbuf == ' ' || *readbuf == '\t' || *readbuf == '\n'))
		readbuf++;
	return readbuf;
}

int isAlpharethmetic(const char c){
	return ((c>='a' && c<='z')||(c>='A' && c<='Z')||(c>='0' && c<='9')||(c=='/')||(c=='.')||(c=='-'));
}

int isSpecialCharacter(const char c){
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

int isWildCharacter(const char c){
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
		readbuf++;
		while(readbuf != readbufSize && *readbuf != '"'){
			*writebuf++ = *readbuf++;
		}
		if(readbuf != readbufSize)readbuf++;
	}
	else if(*readbuf == '\''){
		readbuf++;
		while(readbuf != readbufSize && *readbuf != '\''){
			*writebuf++ = *readbuf++;
		}
		if(readbuf != readbufSize)readbuf++;
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




void createTokenList(const char* readbuf, List* tokenList){
	listInit(tokenList,sizeof(char)*100);
	const char* ptr = readbuf;
	int length = strlen(readbuf);
	char buffer[100];
	while(skipWhiteSpaces(ptr,length + readbuf) != length + readbuf){
		ptr = getNextToken(ptr,buffer, length + readbuf);
		listAppend(tokenList,buffer);
	}
}

















