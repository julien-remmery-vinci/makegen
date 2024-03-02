#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "makegen_utils.h"

void* checkNull(void* v) {
	if(v == NULL) {
		perror("Error");
		return NULL;
	}
	return v;
}

void checkNullExit(void* v) {
	if(v == NULL) {
		perror("Error");
		exit(1);
	}
}

void checkArgs(int nbArgs){
	if(nbArgs == 1){
		printf("Error: please provide a file in arguments\n");
		exit(1);
	}
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
	checkNull((*t)[*size]);
	strcpy((*t)[*size], toAdd);
	(*size)++;
}

void getNbFiles(char* filename, int* nbFiles){
	*nbFiles = 0;
	//Open c source file (read mode)
	FILE* fptr = fopen(filename, "r");
	if(fptr != NULL) { 
		//Get number of files
		char buff[MAX_LENGTH+1];
		while(fgets(buff, MAX_LENGTH, fptr))
			if(strncmp(buff, "#include \"", 10) == 0) (*nbFiles)++;
			else if(strncmp(buff, "#d", 2) == 0) break;
		//Close file
		fclose(fptr);
	}
}

char** getHeaderFiles(char* filename, int* nbFiles){
	getNbFiles(filename, nbFiles);
	if(*nbFiles == 0) {
		char** requiredFiles = (char**) malloc(1*sizeof(char*));
		checkNull(requiredFiles);
		requiredFiles[0] = (char*) malloc((strlen(filename))*sizeof(char));
		checkNull(requiredFiles[0]);
		addFile(&requiredFiles, filename, nbFiles);
		return requiredFiles;
	}
	char** requiredFiles = (char**) malloc((*nbFiles)*sizeof(char*));
	checkNull(requiredFiles);

	//Open c source file (read mode)
	FILE* fptr = fopen(filename, "r");
	if(fptr != NULL) {
		//Get files names
		char buff[MAX_LENGTH+1];
		int filesFound = 0;
		while(fgets(buff, MAX_LENGTH, fptr) && filesFound < *nbFiles)
			if(strncmp(buff, "#include \"", 10) == 0){
					char name[MAX_LENGTH];
					strncpy(name, buff+QUOTE_POS+1, MAX_LENGTH);
					int size = strlen(name)-4;
					name[size] = '\0';
					addFile(&requiredFiles, name, &filesFound);
			}
		//Close file
		fclose(fptr);
		return requiredFiles;
	}
	return NULL;
}

void addFlags(FILE* f) {
	fprintf(f, "%s\n\n", FLAGS);
}

char* addAllRule(FILE* f, int n, char** t) {
	char* text = NULL;
	for (int i = 0; i < n; ++i) {
		int size = strlen(t[i])-2;
		text = realloc(text, size+1*sizeof(char));
		checkNullExit(text);
		char* filename = (char*)malloc(size*sizeof(char));
		checkNullExit(filename);
		strncpy(filename, t[i], size);
		strcat(text, filename);
		free(filename);
		strcat(text, " ");
	}
	fprintf(f, "all: %s\n\n", text);
	return text;
}