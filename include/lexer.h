
#define MAXTOKENSIZE 500

int isWhiteSpace(char c);
int isAlpharethmetic(char c);
int isValidCharacter(char c);
int isSpecialCharacter(char c);
int isWildCharacter(char c);
int isQuotedString(const char* str);
int isValidAlias(const char* str);
int rmQuotesFromString(char* str);
const char* skipWhiteSpaces(const char* readbuf, const char* readbufSize);
const char* getNextToken(const char* readbuf, char* writebuf, const char* readbufSize);
void createTokenList(const char* readbuf, List* tokenList);
int removeQuotations(List* tokenList);



