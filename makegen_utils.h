#ifndef _MAKEGEN_H_
#define _MAKEGEN_H_

#define QUOTE_POS 9
#define MAX_LENGTH 255
#define MAX_TAB_LENGTH 255
#define GENERATED_NAME "Makefile"
#define TOP_TEXT "# made using makegen by julien.r\n"
#define FLAGS "CFLAGS=-std=c17 -pedantic -Wvla -Werror -Wall -D_DEFAULT_SOURCE -g"

void* checkNull(void*);

void checkNullExit(void*);

void checkArgs(int);

bool containsFile(char**, char*, int);

void addFile(char***, char*, int*);

/* 
 * Pre : filename : the c source file to read from
		 nbFiles  : 
 * Post: nbFiles  : the number of header files found
 */
void getNbFiles(char*, int*);

/* 
 * Pre : filename : the c source file to read from
 		 nbFiles  : the number of header files
 * Res : returns an array containing the header files names
 */
char** getHeaderFiles(char*, int*);

void addFlags(FILE*);

char* addAllRule(FILE*, int, char**);

#endif