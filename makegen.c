#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <string.h>

int main(int argc, char *argv[])
{
	//Check for missing file as argument
	if(argc == 1){
		fprintf(stderr, "Error : Please give me a file\n");
		exit(1);
	}
	//Get filename from args
	char* filename = argv[1];

	//Initialise and open file with write mode
	FILE *fptr;
	fptr = fopen("makefile", "w");
	fputs("# made using makegen by julien.r\n", fptr);

	//Add flags
	fputs("CFLAGS=-std=c17 -pedantic -Wvla -Werror -Wall -D_DEFAULT_SOURCE -g\n\n", fptr);

	//Add all rule
	filename = strtok(filename, ".");
	fprintf(fptr, "all : %s\n\n", filename);

	//Close file
	fclose(fptr);

	//Open c source file with read mode
	strcat(filename, ".c");
	fptr = fopen(filename, "r");

	//Get header files
	char** requiredFiles = (char**) malloc(255*sizeof(char*));
	if(requiredFiles == NULL) exit(1);
	int nbFiles = 0;

	char buff[255];
	while(fgets(buff, 255, fptr)){
		if(buff[0] == '\n') break;
		for (int i = 0; i < strlen(buff); ++i)
		{
			if(buff[i] == '\n'){
				buff[i] = '\0';
				break;
			}
		}
		char s[255];
		char buf[11];
		strncpy(buf, buff, 10);
		buf[strlen(buf)] = '\0';
		if(strcmp(buf, "#include") && buf[9] == '"'){
			char c = buff[10];
			int i = 10;
			int nbLetters = 0;
			while(c != '.'){
				s[nbLetters] = c;
				nbLetters++;
				c = buff[i+1];
				i++;
			}
			s[nbLetters] = '\0';
			requiredFiles[nbFiles] = (char*) malloc(255*sizeof(char));
			if(requiredFiles[nbFiles] == NULL) exit(1);
			strcpy(requiredFiles[nbFiles], s);
			nbFiles++;
		}
	}

	//No header file found
	if(nbFiles == 0){
		fprintf(stderr, "Error : No header file found\n");
		exit(1);
	}

	//Close file
	fclose(fptr);

	//Open makefile file with append mode
	fptr = fopen("makefile", "a");
	filename = strtok(filename, ".");

	//Build the string with needed .o files
	char reqStr[255] = "";
	for (int i = 0; i < nbFiles; i++){
		char s[255];
		sprintf(s, "%s.o ", requiredFiles[i]);
		strcat(reqStr, s);
	}

	//Add edition de liens
	fprintf(fptr, "# Edition de liens\n");

	fprintf(fptr, "%s : %s.o %s\n", filename, filename, reqStr);
	fprintf(fptr, "\tcc $(CFLAGS) -o %s %s.o %s\n\n", filename, filename, reqStr);

	//Add compilation
	fprintf(fptr, "# Compilation\n");

	fprintf(fptr, "%s.o : %s.c\n", filename, filename);
	fprintf(fptr, "\tcc $(CFLAGS) -c %s.c\n\n", filename);

	for (int i = 0; i < nbFiles; ++i)
	{
		fprintf(fptr, "%s.o : %s.c\n", requiredFiles[i], requiredFiles[i]);
		fprintf(fptr, "\tcc $(CFLAGS) -c %s.c\n\n", requiredFiles[i]);
	}

	//Add clean rule
	fprintf(fptr, "clean :\n");
	fprintf(fptr, "\trm *.o\n");
	fprintf(fptr, "\trm %s\n", filename);
}
