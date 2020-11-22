#include "hashtable.h"
#include "utils.h"
#include "tema1.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX 256
#define DEF 10
#define STDIN 0
#define STDOUT 1
#define MEM_ERROR 12

int  main(int argc, char **argv) {
	char buf[MAX];

	defines = createTable(DEF);
	if (!defines)
		return MEM_ERROR;

	rc = parseCommand(argc, argv);
	if (rc == MEM_ERROR) {
		destroy(defines);
		return MEM_ERROR;
	} else if (rc == -1) {
		destroy(defines);
		return 0;
	}

	openFiles();

	rc = parseFile(buf, in);
	if(rc) {
		catastrophe();
		return MEM_ERROR;
	}
	
	destroy(defines);
	closeFiles();
	
	return 0;
}

/* Parsarea argumentelor din linia de comanda */
int parseCommand(int argc, char **argv) {
	char *define = NULL;
	int i, a;

	for (i = 1; i < argc; ++i) {
		if (argv[i][1] == 'D') {
			/* -D <define> */
			if (strlen(argv[i]) == 2) {
				define = argv[++i];
				rc = getDefine(define, "=");

				if (rc)
					return MEM_ERROR;

				continue;	
			} else {
				/* -D<define> */
				define = argv[i] + 2;
				rc = getDefine(define, "=");

				if (rc)
					return MEM_ERROR;
			}
		} else if (argv[i][1] == 'I') {
			/* -I <dir> */
			if (strlen(argv[i]) == 2) {
				if (!gotDir) {
					gotDir = 1;
					
					dir = malloc(MAX);
					if (!dir)
						return MEM_ERROR;

					strcpy(dir, argv[++i]);

					continue;
				}
			} else {
				/* -I<dir> */
				if (!gotDir) {
					gotDir = 1;

					dir = malloc(MAX);
					if (!dir)
						return MEM_ERROR;

					strcpy(dir, argv[i] + 2);
				}
			}
		} else if (argv[i][1] == 'o') {
			/* -o <file> */
			if (strlen(argv[i]) == 2) {
				output = argv[++i];
				gotOut = 1;
				++i;
			} else {
				/* -o<file> */
				output = argv[i] + 2;
				gotOut = 1;
			}
		} else if (strchr(argv[i], '.') && !strstr(argv[i], ".dir")) {
			/* Numar fisierele primite in linia de comanda,
			   pentru a asigura corectitudinea in argumentelor */
			++cntFiles;
		} else if (strlen(argv[i]) == 2 && argv[i][1] == 'X') {
			DIE(1, "Bad param\n\r");
		}
	}

	if (cntFiles > 2) {
		/* Prea multe fisiere primite */
		DIE(1, "Bad param\n\r");
	} else if (cntFiles) {
		/* Daca exista fisiere de input / output,
		   parcurgem argumentele si le retim numele fisierelor */
		a = 1;

		/* Fisierul de input */
		while (cntFiles && (a < argc || !gotIn)) {
			if (strchr(argv[a], '.') && !strstr(argv[a], ".dir")) {
				input = argv[a];
				gotIn = 1;
				--cntFiles;	

				break;
			}

			++a;
		}

		++a;

		/* Fisierul de output */
		while (cntFiles && (a < argc || !gotOut)) {
			if (strchr(argv[a], '.')  && !strstr(argv[a], ".dir")) {
				output = argv[a];
				gotOut = 1;
				--cntFiles;

				break;
			}

			++a;
		}
	}

	return 0;
}

void openFiles() {
	/* Daca avem fisier de input, il deschidem.
	   Altfel, citim de la stdin */
	if (input) {
		in = fopen(input, "r");
		DIE(in == NULL, "open in");
	} else {
		fdIn = STDIN;
		in = stdin;
	}

	/* Daca avem fisier de output, il deschidem.
	   Altfel, afisam la stdout */
	if (output) {
		out = fopen(output, "w");
		DIE(out == NULL, "open out");
	} else {
		fdOut = STDOUT;
		out = stdout;
	}
}

int parseFile(char *buf, FILE *inLocal) {
	while (fgets(buf, MAX, inLocal)) {
		if (!strcmp(buf, "\n"))
			continue;
		
		if (strstr(buf, "#include ")) {
			// printf("%s\n", buf);
			rc = includeFunc(strchr(buf, '"') + 1);
			if (rc) 
				return MEM_ERROR;

			continue;
		}

		if (strstr(buf, "#define ")) {
			// printf("%s\n", buf);
			if (strchr(buf, '"') || !strchr(buf, '\\')) {
				/* Define pe o singura linie */
				rc = getDefine(strchr(buf, ' ') + 1, " \n");
				if (rc)
					return MEM_ERROR;
			} else {
				/* Define pe mai multe linii */
				rc = parseMultiLines(buf);
				if (rc)
					return MEM_ERROR;
			}

			continue;
		}

		if (strstr(buf, "#undef ")) {
			undefinedFunc(strchr(buf, ' ') + 1, "\n");
			continue;
		}

		if (strstr(buf, "#if ")) {
			ifFunc(buf, inLocal);
			continue;
		}

		/* Daca se ajunge la else,
		   se trateaza cazul ca fiind o conditie neindeplinita.
		   -> Situatia de pe ramura false a if-ului */
		if (strstr(buf, "#else")) {
			strcpy(buf, " 0\n");
			ifFunc(buf, inLocal);

			continue;
		}

		/* Alt if */
		if (strstr(buf, "#elif"))
			ifFunc(buf, inLocal);

		if (strstr(buf, "#endif"))
			continue;

		if (strstr(buf, "#ifndef ")) {
			char *eval = strtok(strchr(buf, ' ') + 1, "\n");

			if (!findKey(defines, eval)) {
				strcpy(buf, " 1\n");
			} else {
				strcpy(buf, " 0\n");
			}
			
			ifFunc(buf, inLocal);
			continue;
		}

		if (strstr(buf, "#ifdef ")) {
			char *eval = strtok(strchr(buf, ' ') + 1, "\n");

			if (!findKey(defines, eval)) {
				strcpy(buf, " 0\n");
			} else {
				strcpy(buf, " 1\n");				
			}
		
			ifFunc(buf, inLocal);
			continue;
		}

		if (strstr(buf, "printf")) {
			rc = printfFunc(buf);
			if (rc)
				return MEM_ERROR;
		} else {
			findDef(buf);
		}

		fputs(buf, out);
	}

	return 0;
}

int includeFunc(char *buf) {
	char *current = strtok(buf, "\"");

	include = malloc(strlen("_test/inputs") +  strlen(current) + 2);
	if (!include)
		return MEM_ERROR;

	strcpy(include, "_test/inputs");
	strcat(include, "/");
	strcat(include, current);

	/* Deschid fisierul aflat in directorul in care
	   se afla si input-ul */
	included = fopen(include, "r");

	if (!included) {
		/* Deschid fisierul din directorul primit in linia de comanda */
		if (dir) {
			strcat(dir, "/");
			strcat(dir, current);

			included = fopen(dir, "r");

			if (!included) {
				if (dir)
					free(dir);
				if (include)
					free(include);

				catastrophe();
				DIE(included == NULL, "Open included file");
			}
		} else {
			if (dir)
				free(dir);
			if (include)
				free(include);

			catastrophe();
			DIE(included == NULL, "Open included file");
		}
	}

	rc = parseFile(buf, included);

	if (include)
		free(include);
	if (dir)
		free(dir);

	fclose(included);

	return rc;
}

/* Parcurg hash-ul.
   Inlocuiesc fiecare variabila din buffer cu valoarea ei */
void findDef(char *buf) {
	HashNode **table = getTable(defines);
	HashNode *d;
	char *name;
	char *value;
	int i;

	for (i = 0; i < DEF; ++i) {
		d = table[i];

		while (d) {
			name = getKey(d);

			if (name) {
				value = getValue(defines, name);
				replace(buf, name, value);
			}

			d = getNext(d);
		}
	}
}

/* Functie care inlocuieste toate aparitiile unui cuvant intr-un sir */
void replace(char *buf, char *name, char *value) {
	char *pos, temp[MAX];
	int index = 0;
	int lenName = strlen(name);

	strcpy(temp, buf);

	/* cautam pozitia la care se afla name */
	pos = strstr(buf, name);
	if (pos) {
		/* Daca name este in buf, retinem textul pana la aparitia acestuia,
		   concatenam value si completam cu sirul de dupa name */
		index = pos - buf;
		buf[index] = '\0';
		strcat(buf, value);
		strcat(buf, temp + index + lenName);
	}
}

/* Parsare #define */
int getDefine(char *define, char *delim) {
	char *name = strtok(define, delim);
	char *value = strtok(NULL, "\n");

	if (!value) {
		value = "";
	} else {
		rc = imbricatedDef(value);
		while (rc) {
			if (rc == MEM_ERROR)
				return MEM_ERROR;

			rc = imbricatedDef(value);
		}
	}

	rc = putKey(defines, name, value);
	if (rc)
		return MEM_ERROR;

	return 0;
}

int imbricatedDef(char *imbricated) {
	char *copy;
	char *token;
	int cnt = 0;

	copy = malloc(strlen(imbricated) + 1);
	if (!copy)
		return MEM_ERROR;

	strcpy(copy, imbricated);

	if (strchr(copy, ' ')) {
		token = strtok(copy, " \0");
		while (token) {
			if (findKey(defines, token)) {
				replace(imbricated, token, getValue(defines, token));
				++cnt;
			}
			
			token = strtok(NULL, " \0");
		}
	}

	free(copy);

	return cnt;
}

/* Elimin variabila din hash */
void undefinedFunc(char *define, char *delim) {
	char *name = strtok(define, delim);
	removeKey(defines, name);
}

int parseMultiLines(char *buf) {
	char multilines[MAX] = "";

	replace(buf, "        ", "");
	replace(buf, "\\", "");
	replace(buf, "\n", "");
	strcpy(multilines, buf);

	while (fgets(buf, MAX, in) && strcmp(buf, "\n")) {
		replace(buf, "        ", "");
		replace(buf, "\\", " ");
		replace(buf, "\n", "");
		strcat(multilines, buf);
	}

	rc = getDefine(strchr(multilines, ' ') + 1, " ");
	if (rc)
		return MEM_ERROR;

	return 0;
}

void ifFunc(char *buf, FILE *inLocal) {
	char *eval = strtok(strchr(buf, ' ') + 1, "\n");

	if (findKey(defines, eval)) {
		strcpy(eval, getValue(defines, eval));
	}

	/* Daca nu e indeplinita conditia,
	   se ignora liniile citite, pana se ajunge la else sau endif */
	if (!strcmp(eval, "0")) {
		fgets(buf, MAX, inLocal);

		while (!strstr(buf, "#else") && !strstr(buf, "#elif")
									 && !strstr(buf, "#endif")) {
			fgets(buf, MAX, inLocal);
			// printf("hello: %s", buf);						
		}

		if (strstr(buf, "#elif")) {
			ifFunc(buf, inLocal);
		}

		/* Daca se ajunge la else cand conditia a fost evaluata la false,
		   inderpretez else-ul ca un if evaluat la true */
		if (strstr(buf, "#else")) {
			strcpy(buf, " 1\n");
			ifFunc(buf, inLocal);
		}
	}
}

int printfFunc(char *buf) {
	char temp[MAX] = "";
	char *token = calloc(MAX, sizeof (char));

	if (!token)
		return MEM_ERROR;

	/* Separ argumentele functiei */
	if (strchr(buf, ',')) {
		strcpy(token, strtok(buf, ","));
		memmove(temp, token, strlen(token));

		/* Trimit cea de-a doua parte la replace */
		strcpy(token, strtok(NULL, "\0"));
		memset(buf, 0, MAX);
		memmove(buf, token, strlen(token));

		findDef(buf);

		/* Rebuild */
		strcat(temp, ",");
		strcat(temp, buf);
		strcpy(buf, temp);
	}
	
	free(token);

	return 0;
}

void closeFiles() {
	if (in != stdin) {
		rc = fclose(in);
		DIE(rc < 0, "fdIn");
	}

	if (out != stdout) {
		rc = fclose(out);
		DIE(rc < 0, "fdOut");
	}
}

void catastrophe() {
	destroy(defines);
	closeFiles();
}
