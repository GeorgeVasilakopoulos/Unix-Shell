
#define MAX_CHARACTERS_PER_LS 100000

int isWildString(const char* readbuf);
void replaceCharInString(char* string, char c, const char* subst);
int replaceWildTokens(List* tokenList);