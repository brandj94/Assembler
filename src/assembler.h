#ifndef ASSEMBLER_H
#define ASSEMBLER_H
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

struct _RegMapping {
	char* regName;
	char* regNumber;
};

struct _OpMapping {
	char* opName;
	char* opCode;
	char* opType;
	char* opSpecial;
	const char* const *opPseudos;
	int pseudoSize;
};

struct _SymbolMapping {
	char* symbolName;
	char* symbolAddress;
	char* symbolValue;
	int symbolAddressInt;
};

typedef struct _RegMapping RegMapping;
typedef struct _OpMapping OpcodeMapping;
typedef struct _SymbolMapping SymbolTable;

/*  Checks to see if the given line contains a specifc substring
	@param *line Pointer to the line we want to search
	@param *search The substring we are searching for

	@return Returns the position of where the substring is
*/
int lineContains(char *line, char *search);

int countElements(char *str);

void parseArray(char *str, char *newString);

void analyzeLine(int i, char *registerLine);

void convertToBinary(int num, char *str, int length, int mode);

void convertTo16Bit(char *line, char *converted);

void findEndRemoveSpaces(char *line, char *newString, int pos);

void symbolToHex(char* hex);

void processRType(char *line, char *reg1, char *reg2, char *reg3);

void processIType2(char *line, char *reg1, char *reg2, char *reg3);

void processTwoParams(char *line, char *reg1, char *reg2);

void constructJumpBinary(char *reg1);

void constructRTypeBinary(char *opName, char *reg1, char *reg2, char *reg3);

void constructIType2Binary(char *opName, char *reg1, char *reg2, char *reg3);

void constructShiftBinary(char *opName, char *reg1, char *reg2, char *reg3);

void constructJTypeBinary(char *opName, char *reg1, char *reg2, char *reg3, int pc);

void constructPseudoBinary(char *opName, char *reg1, char *reg2, char *reg3);

void removeNewLines(char *line);

#endif