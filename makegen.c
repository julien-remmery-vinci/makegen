#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "makegen_utils.h"

int main(int argc, char *argv[]){
	//Check for missing files as argument
	checkArgs(argc);

	//Open or create makefile (write mode)
	FILE* f = fopen(GENERATED_NAME, "w");
	checkNullExit(f);

	fputs(TOP_TEXT, f);

	//Add flags
	addFlags(f);

	int nbExec = argc-1;
	char* generatedExec = NULL;
	//Add all rule if necessary
	if(nbExec > 1)
		generatedExec = addAllRule(f, nbExec, argv+1);

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
			fprintf(f, "%s:\n", c);
			fprintf(f, "\tcc $(CFLAGS) -o %s %s.c\n\n", c, c);
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

			fprintf(f, "%s: %s\n", filename, reqStr);
			fprintf(f, "\tcc $(CFLAGS) -o %s %s\n\n", filename, reqStr);
			oFilesToClean = true;

			for (int i = 0; i < nbFiles; ++i)
				free(requiredFiles[i]);
			free(requiredFiles);
		}
	}

	//Add compilation

	for (int i = 0; i < nbToCompile; ++i){
		fprintf(f, "%s.o: %s.c\n", filesToCompile[i], filesToCompile[i]);
		fprintf(f, "\tcc $(CFLAGS) -c %s.c\n\n", filesToCompile[i]);
	}

	for (int i = 0; i < nbToCompile; ++i)
		free(filesToCompile[i]);
	free(filesToCompile);

	//Add clean rule
	fprintf(f, "clean:\n");
	if(oFilesToClean) fprintf(f, "\trm *.o\n");
	fprintf(f, "\trm %s\n", generatedExec);
	free(generatedExec);

	//Close file
	fclose(f);

	printf("Makegen succeeded\nFile available as '%s'\n", GENERATED_NAME);
}
