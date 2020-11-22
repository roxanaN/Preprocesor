#ifndef TEMA1_H_
#define TEMA1_H_     1

int fdIn, fdOut, rc;
int gotIn = 0, gotOut = 0, gotDir = 0, cntFiles = 0;
char *input = NULL, *output = NULL, *include = NULL, *dir = NULL;
HashTable *defines;
FILE *in, *out, *included;

void openFiles();
void closeFiles();
void catastrophe();
void ifFunc(char *buf, FILE *in);
void findDef(char *buf);
int parseFile(char *buf, FILE *in);
int printfFunc(char *buf);
int includeFunc(char *buf);
int parseMultiLines(char *buf);
int imbricatedDef(char *imbricated);
int parseCommand(int argc, char **argv);
int getDefine(char *define, char *delim);
void undefinedFunc(char *define, char *delim);
void replace(char *buf, char *name, char *value);

#endif