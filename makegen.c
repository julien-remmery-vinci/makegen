#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define QUOTE_POS 9
#define MAX_LENGTH 255
#define MAX_TAB_LENGTH 255
#define GENERATED_NAME "Makefile"
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
		if(buff[0] == '#' && buff[QUOTE_POS] == '"')
			(*nbFiles)++;
	}
	//Close file
	fclose(fptr);
}

/* 
 * Pre : filename : the c source file to read from
 		 nbFiles  : the number of header files
 * Res : returns an array containing the header files names
 */
char** getHeaderFiles(char* filename, int* nbFiles){
	int nbFound; 
	getNbFiles(filename, &nbFound);
	*nbFiles = nbFound;
	if(nbFound == 0) {
		char** requiredFiles = (char**) malloc(1*sizeof(char*));
		if(requiredFiles == NULL) return NULL;
		requiredFiles[0] = (char*) malloc((strlen(filename))*sizeof(char));
		if(requiredFiles[0] == NULL) return NULL;
		strcpy(requiredFiles[0], filename);
		return requiredFiles;
	}
	char** requiredFiles = (char**) malloc(nbFound*sizeof(char*));
	if(requiredFiles == NULL) return NULL;

	//Open c source file (read mode)
	FILE* fptr = fopen(filename, "r");

	//Get files names
	char buff[MAX_LENGTH+1];
	int i = 0;
	int filesFound = 0;
	while(fgets(buff, MAX_LENGTH, fptr) && filesFound < nbFound){
		if(buff[0] == '#') {
			if(buff[QUOTE_POS] == '"'){
				char name[MAX_LENGTH];
				int nbC = 0;
				for (int j = QUOTE_POS+1; buff[j] != '.'; j++, nbC++)
					name[nbC] = buff[j];
				name[nbC] = '\0';
				requiredFiles[i] = (char*) malloc((nbC+1)*sizeof(char));
				if(requiredFiles[i] == NULL) return NULL;
				strcpy(requiredFiles[i], name);
				i++;
				filesFound++;
			}
		}
	}
	*nbFiles = nbFound;
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
	FILE* fptr = fopen(GENERATED_NAME, "w");
	if(fptr == NULL) {
		fprintf(stderr, "Error : Could not create %s file\n", GENERATED_NAME);
		exit(1);
	}

	fputs(TOP_TEXT, fptr);

	//Add flags
	fputs("CFLAGS=-std=c17 -pedantic -Wvla -Werror -Wall -D_DEFAULT_SOURCE -g\n\n", fptr);

	//Add all rule if necessary
	if(nbExec > 1){
		char allRule[MAX_LENGTH];
		sprintf(allRule, "all: ");
		for (int i = 1; i < argc; ++i){
			int size = strlen(argv[i]);
			char* filename = (char*) malloc(size*sizeof(char));
			if(filename == NULL){
				fprintf(stderr, "Error : Malloc error\n");
				exit(1);
			}
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
	bool oFilesToClean;
	for (int i = 1; i <= nbExec; ++i){
		//Get filename from args
		int size = strlen(argv[i]);
		char* filename = (char*) malloc(size*sizeof(char));
		if(filename == NULL){
			fprintf(stderr, "Error : Malloc error\n");
			exit(1);
		}
		sprintf(filename, "%s", argv[i]);

		//Get header files count and names
		int nbFiles;
		char** requiredFiles = getHeaderFiles(filename, &nbFiles);
		if(requiredFiles == NULL){
			fprintf(stderr, "Error : error while getting files\n");
			exit(1);
		}

		//nbFiles = number of header files found in provided file
		if(nbFiles == 0) {
			char c[MAX_LENGTH];
			sprintf(c, "%s", strtok(requiredFiles[0], "."));
			fprintf(fptr, "%s:\n", c);
			fprintf(fptr, "\tcc $(CFLAGS) -o %s %s.c\n\n", c, c);
			strcat(toBeCleaned, requiredFiles[0]);
			strcat(toBeCleaned, " ");
		}
		//Look for header files in files found
		else {
			for (int j = 0; j < nbFiles; ++j) {
				int nbHeaderFiles;
				size = strlen(requiredFiles[j]);
				char* fName = (char*) malloc(size*sizeof(char));
				if(fName == NULL){
					fprintf(stderr, "Error : Malloc error\n");
					exit(1);
				}
				strcpy(fName, requiredFiles[j]);
				strcat(fName, ".c");
				char** requiredHeaderFiles = getHeaderFiles(fName, &nbHeaderFiles);
				if(requiredHeaderFiles == NULL && nbHeaderFiles > 0){
					fprintf(stderr, "Error : error while getting files\n");
					exit(1);
				}
				for (int k = 0; k < nbHeaderFiles; ++k)
				{
					if(!containsFile(requiredFiles, requiredHeaderFiles[k], nbFiles))
						addFile(&requiredFiles, requiredHeaderFiles[k], &nbFiles);
					if(!containsFile(filesToCompile, requiredHeaderFiles[k], nbToCompile))
						addFile(&filesToCompile, requiredHeaderFiles[k], &nbToCompile);
				}
			}
			//Build the string with needed .o files
			char reqStr[MAX_LENGTH];
			sprintf(reqStr, "%s.o", strtok(filename, "."));

			if(!containsFile(filesToCompile, filename, nbToCompile))
				addFile(&filesToCompile, filename, &nbToCompile);

			for (int i = 0; i < nbFiles; i++){
				char s[MAX_LENGTH+1];
				sprintf(s, " %s.o", requiredFiles[i]);
				strcat(reqStr, s);
				if(!containsFile(filesToCompile, requiredFiles[i], nbToCompile))
					addFile(&filesToCompile, requiredFiles[i], &nbToCompile);
			}

			fprintf(fptr, "%s: %s\n", filename, reqStr);
			fprintf(fptr, "\tcc $(CFLAGS) -o %s %s\n\n", filename, reqStr);
			oFilesToClean = true;

			for (int i = 0; i < nbFiles; ++i)
				free(requiredFiles[i]);
			free(requiredFiles);

			strcat(toBeCleaned, filename);
			strcat(toBeCleaned, " ");
		}
	}

	//Add compilation

	for (int i = 0; i < nbToCompile; ++i){
		fprintf(fptr, "%s.o: %s.c\n", filesToCompile[i], filesToCompile[i]);
		fprintf(fptr, "\tcc $(CFLAGS) -c %s.c\n\n", filesToCompile[i]);
	}

	for (int i = 0; i < nbToCompile; ++i)
		free(filesToCompile[i]);
	free(filesToCompile);

	//Add clean rule
	fprintf(fptr, "clean:\n");
	if(oFilesToClean) fprintf(fptr, "\trm *.o\n");
	fprintf(fptr, "\trm %s\n", toBeCleaned);

	//Close file
	fclose(fptr);

	printf("Makegen succeeded\nFile available as '%s'\n", GENERATED_NAME);
}
