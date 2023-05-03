#define MAXHISTORY 20
#define MAXBUFSIZE 400

void addToHistory(const char* inst);
char* fetchFromHistory(int index);
void printHistory();
void clearHistory();
void possiblyCloseFile(int filedes);
void forkExecute(int inputfd, int outputfd, const char* commandName, const char* arguments[], int waitForChild);