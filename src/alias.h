void aliasInit();
void createAlias(const char* alias, const char* instruction);
void replaceAliasesInList(List* tokenList);
void removeAlias(const char* alias);
void destructAlias();
const char* findAlias(const char* alias);