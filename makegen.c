#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include <stdbool.h>

#define QUOTE_POS 9
#define MAX_LENGTH 255
#define MAX_TAB_LENGTH 255
#define TOP_TEXT "# made using makegen by julien.r\n"

/* 
 * Pre : filename : the c source file to read from
		 nbFiles  : 
 * Post: nbFiles  : the number of header files found
 */
void getNbFiles(char* filename, int* nbFiles){
	*nbFiles = 0;
	//Open c source file (read mode)
	FILE* fptr = fopen(filename, "r");

	//Get number of files
	char buff[MAX_LENGTH+1];
	while(fgets(buff, MAX_LENGTH, fptr)){
		if(buff[0] == '\n') break;
		if(buff[QUOTE_POS] == '"') (*nbFiles)++;
	}
	//Close file
	fclose(fptr);
}

/* 
 * Pre : filename : the c source file to read from
 		 nbFiles  : the number of header files
 * Res : returns an array containing the header files names
 */
char** getHeaderFiles(char* filename, int nbFiles){
	char** requiredFiles = (char**) malloc(nbFiles*sizeof(char*));
	if(requiredFiles == NULL) return NULL;

	//Open c source file (read mode)
	FILE* fptr = fopen(filename, "r");

	//Get files names
	char buff[MAX_LENGTH+1];
	int i = 0;
	while(fgets(buff, MAX_LENGTH, fptr)){
		if(buff[0] == '\n') break;
		if(buff[0] == '#' && buff[QUOTE_POS] == '"'){
			char name[MAX_LENGTH];
			int nbC = 0;
			for (int i = QUOTE_POS+1; buff[i] != '.'; i++, nbC++)
				name[nbC] = buff[i];
			name[nbC] = '\0';
			requiredFiles[i] = (char*) malloc((strlen(name)+1)*sizeof(char));
			if(requiredFiles[i] == NULL) exit(1);
			strcpy(requiredFiles[i], name);
			i++;
		}
	}
	//Close file
	fclose(fptr);
	return requiredFiles;
}

bool containsFile(char** t, char* toCompare, int size){
	for (int i = 0; i < size; ++i){
		if(strcmp(t[i], toCompare) == 0)
			return true;
	}
	return false;
}

void addFile(char*** t, char* toAdd, int* size){
	//*t = realloc(*t, (*size)+1*sizeof(char*));
	(*t)[*size] = (char*) malloc((strlen(toAdd)+1)*sizeof(char));
	strcpy((*t)[*size], toAdd);
	(*size)++;
}

int main(int argc, char *argv[]){
	int nbExec = argc-1;
	//Check for missing files as argument
	if(argc == 0){
		fprintf(stderr, "Error : Please give me a file\n");
		exit(1);
	}

	//Open or create makefile (write mode)
	FILE* fptr = fopen("Makefile", "w");
	fputs(TOP_TEXT, fptr);

	//Add flags
	fputs("CFLAGS=-std=c17 -pedantic -Wvla -Werror -Wall -D_DEFAULT_SOURCE -g\n\n", fptr);

	//Add all rule if necessary
	if(nbExec > 1){
		char allRule[MAX_LENGTH];
		sprintf(allRule, "all : ");
		for (int i = 1; i < argc; ++i){
			char filename[strlen(argv[i])];
			strcpy(filename, argv[i]);
			strcat(allRule, strtok(filename, "."));
			strcat(allRule, " ");
		}
		fprintf(fptr, "%s\n\n", allRule);
	}

	//Keep files to clean
	char toBeCleaned[MAX_LENGTH] = "";

	//Keep files to compile
	int nbToCompile = 0;
	char** filesToCompile = (char**) malloc(MAX_TAB_LENGTH*sizeof(char*));
	if(filesToCompile == NULL){
		fprintf(stderr, "Error : Malloc error\n");
		exit(1);
	}

	//Add linker
	fprintf(fptr, "# Linker\n");

	for (int i = 1; i <= nbExec; ++i){
		//Get filename from args
		char filename[strlen(argv[i])];
		sprintf(filename, "%s", argv[i]);

		//Get header files count and names
		int nbFiles;
		getNbFiles(filename, &nbFiles);
		char** requiredFiles = getHeaderFiles(filename, nbFiles);
		if(requiredFiles == NULL){
			fprintf(stderr, "Error : Malloc error\n");
			exit(1);
		}
		//No header file found
		if(nbFiles == 0){
			fprintf(stderr, "Error : No header file found\n");
			exit(1);
		}

		//Build the string with needed .o files
		char reqStr[MAX_LENGTH];
		sprintf(reqStr, "%s.o", strtok(filename, "."));

		if(!containsFile(filesToCompile, filename, nbToCompile))
			addFile(&filesToCompile, filename, &nbToCompile);

		for (int i = 0; i < nbFiles; i++){
			char s[MAX_LENGTH];
			sprintf(s, " %s.o", requiredFiles[i]);
			strcat(reqStr, s);
			if(!containsFile(filesToCompile, requiredFiles[i], nbToCompile))
				addFile(&filesToCompile, requiredFiles[i], &nbToCompile);
		}

		fprintf(fptr, "%s : %s\n", filename, reqStr);
		fprintf(fptr, "\tcc $(CFLAGS) -o %s %s\n\n", filename, reqStr);

		for (int i = 0; i < nbFiles; ++i)
			free(requiredFiles[i]);
		free(requiredFiles);

		strcat(toBeCleaned, filename);
		strcat(toBeCleaned, " ");

	}

	//Add compilation
	fprintf(fptr, "# Compilation\n");

	for (int i = 0; i < nbToCompile; ++i){
		fprintf(fptr, "%s.o : %s.c\n", filesToCompile[i], filesToCompile[i]);
		fprintf(fptr, "\tcc $(CFLAGS) -c %s.c\n\n", filesToCompile[i]);
	}

	for (int i = 0; i < nbToCompile; ++i)
		free(filesToCompile[i]);
	free(filesToCompile);

	//Add clean rule
	fprintf(fptr, "clean :\n");
	fprintf(fptr, "\trm *.o %s\n", toBeCleaned);

	//Close file
	fclose(fptr);
}
