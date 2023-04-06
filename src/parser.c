#include <stdio.h>
#include <string.h>

char* skipWhiteSpaces(char* readbuf, char* readbufSize){
	while(readbuf != readbufSize && (*readbuf == ' ' || *readbuf == '\t' || *readbuf == '\n'))
		readbuf++;
	return readbuf;
}



char* getNextToken(char* readbuf, char* writebuf, char* readbufSize){
	readbuf = skipWhiteSpaces(readbuf,readbufSize);
	while(readbuf != readbufSize && *readbuf != ' ' && *readbuf != '\n'){
		*writebuf++ = *readbuf++;
	}
	//if(readbuf == readbufSize)return NULL;
	*writebuf = '\0';
	return readbuf;
}








