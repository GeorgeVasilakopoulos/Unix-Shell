int interpretInstruction(const char* readbuf);
void addToHistory(const char* inst);
void clearHistory();
void possiblyCloseFile(int filedes);
void forkExecute(int inputfd, int outputfd, const char* commandName, const char* arguments[], int waitForChild);
