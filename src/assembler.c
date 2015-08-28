/*
 * Brandon Jacobsen <brandj94>
 * Compiler Assignment
*/

 #include "assembler.h"

 #define OP_MAP_SIZE   28
 #define REG_MAP_SIZE  26

 	//Maps the registers to their individual register numbers
 	RegMapping registers[REG_MAP_SIZE] = {
 		{"$zero", "00000"},
 		{"$a0",   "00100"},
 		{"$a1",   "00101"},
 		{"$a2",   "00110"},
 		{"$a3",   "00111"},
 		{"$at",   "00001"},
 		{"$s0",	  "10000"},
 		{"$s1",   "10001"},
 		{"$s2",   "10010"},
 		{"$s3",   "10011"},
 		{"$s4",	  "10100"},
 		{"$s5",   "10101"},
 		{"$s6",   "10110"},
 		{"$s7",   "10111"},
 		{"$t0",   "01000"},
 		{"$t1",   "01001"},
 		{"$t2",   "01010"},
 		{"$t3",   "01011"},
 		{"$t4",   "01100"},
 		{"$t5",   "01101"},
 		{"$t6",   "01110"},
 		{"$t7",   "01111"},
 		{"$t8",   "11000"},
 		{"$t9",   "11001"},
 		{"$v0",   "00010"},
 		{"$v1",   "00011"}
 	};

 	//Maps the opcode mnemonics to their individual opcode
 	OpcodeMapping opcodes[OP_MAP_SIZE] = {
 		//Basic arithmetic/logical instructions
 		{"addiu", "001001", "itype"},
 		{"addi",  "001000", "itype"},
 		{"add",   "100000", "rtype", "000000"},
 		{"andi",  "001100", "itype"},
 		{"and",   "100100", "rtype", "000000"},
 		{"mul",   "000010", "rtype", "011100"},
 		{"nor",   "100111", "rtype", "000000"},
 		{"ori",   "001101", "itype"},
 		{"or",    "100101", "rtype", "000000"},
 		{"sll",   "000000", "shift", "000000"},
 		{"slti",  "001010", "itype"},
 		{"slt",   "101010", "rtype", "000000"},
 		{"sra",   "000011", "shift", "000000"},
 		{"sub",   "100010", "rtype", "000000"},
 		//Basic load & store ops
 		{"lw",    "100011", "itype2"},
 		{"sw",    "101011", "itype2"},
 		{"lui",   "001111", "tptype"},
 		{"blez",  "000110", "tptype"},
 		{"bltz",  "000001", "tptype"},
 		//Branch Types
 		{"beq",   "000100", "jtype"},
 		{"bne",   "000101", "jtype"},
 		//Pseudo
 		{"ble","", "pseudo","", (const char * const []){"slt", "beq"}, 2},
 		{"blt","", "pseudo","", (const char * const []){"slt", "bne"}, 2},
 		{"la", "", "pseudo","", (const char * const []){"addi"}, 1},
 		{"li", "", "pseudo","", (const char * const []){"addiu"}, 1},
 		{"nop",   "00000000000000000000000000000000"},
 		{"j"},
 		{"syscall", "00000000000000000000000000001100"}
 	};

 	int symbolTableSize = 50;
 	SymbolTable symbols[50];
 	int symbolCounter = 8192;
 	int symbolPos = 0;
 	char hex[11];
 	char bin[33];
	char reg1[20];
	char reg2[20];
 	char reg3[20];
 	FILE *INPUT;
 	FILE *ENDFILE;

 int main(int argc, char *argv[])
 {
 	if (argc < 2 || argc > 4)
 	{
 		printf("Incorrect number of arguments");
 		exit(-1);
 	}
 	else if (argc == 3)
 	{
 		INPUT = fopen(argv[1], "r");
 		ENDFILE = fopen(argv[2], "w");
 		if (INPUT == 0)
 		{
 			printf("Not a correct file");
 			exit(-1);
 		}
 	}
 	else if (argc == 4)
 	{
 		if (strcmp(argv[1], "-symbols") != 0)
 		{
 			printf("Not a proper flag");
 			exit(-1);
 		}
 		INPUT = fopen(argv[2], "r");
 		ENDFILE = fopen(argv[3], "w");
 	}
 	FILE *DATA = fopen("data.txt", "w"); //Data File
 	FILE *TEXT = fopen("text.txt", "w"); //Text File
 	FILE *OUTPUT = DATA;

 	char line[256];

 	while(fgets(line, sizeof(line), INPUT) != NULL)
 	{
 		int writeTo = lineContains(line, ".data");
 		if (writeTo != -1)
 		{
 			OUTPUT = DATA;
 		}

 		writeTo = lineContains(line, ".text");
 		if (writeTo != -1)
 		{
 			OUTPUT = TEXT;
 		}

 		int found = lineContains(line, "#");
 		if (found != -1)
 		{
 			char strippedLine[found+1];
 			strncpy(strippedLine, line, found);
 			strippedLine[found] = '\0';
 			if (found != 0)
 			{
 				fprintf(OUTPUT, "%s\n", strippedLine);
 			}
 		}
 		else
 		{
 			if (line[0] != '\n')
 			{
 				fprintf(OUTPUT, "%s", line);
 			}
 		}
 	}

 	/////////////////PASS 2////////////////////////

 	/////////////////DATA SECTION//////////////////////////

 	rewind(DATA);
 	DATA = fopen("data.txt", "r");


 	while(fgets(line, sizeof(line), DATA) != NULL)
 	{
 		//Parses any labels and puts them into the Symbol Table
 		int found = lineContains(line, ":");
 		if (found != -1)
 		{
 			char strippedLine[found+1];
 			strncpy(strippedLine, line, found);
 			strippedLine[found] = '\0';

 			symbols[symbolPos].symbolName = malloc(sizeof(strippedLine));
 			strcpy(symbols[symbolPos].symbolName, strippedLine);

 			symbolToHex(hex);
 			symbols[symbolPos].symbolAddress = malloc(sizeof(hex));
 			strcpy(symbols[symbolPos].symbolAddress, hex);

 			symbols[symbolPos].symbolAddressInt = symbolCounter;

 			symbolCounter+=4;
 		}

 		//If we have found a word in the assembly file
 		found = lineContains(line, ".word");
 		if (found != -1)
 		{
 			//Strip down it's parameters
 			char stripped[20];
 			findEndRemoveSpaces(line, stripped, found+5);

 			//If it's an array, get it's two parts
 			found = lineContains(stripped, ":");
 			if (found != -1)
 			{
 				int num;
 				char leftArray[found+1];
 				strncpy(leftArray, stripped, found);
 				leftArray[found] = '\0';

 				char rightArray[10];
 				findEndRemoveSpaces(stripped, rightArray, found+1);

 				num = atoi(rightArray);
 				char arrayBin[(num*32) + num];

 				int i;
 				for (i = 0; i < num+1; i++)
 				{
 					char temp[32];
 					convertToBinary(atoi(leftArray), temp, 32, 0);
 					strcat(arrayBin, temp);
 					if (i  < num)
 					{
 						arrayBin[(33*i)] = '\n';
 						arrayBin[(33*i)+1] = '\0';
 					}
 				}

 				symbols[symbolPos].symbolValue = malloc(sizeof(arrayBin));
 				strcpy(symbols[symbolPos].symbolValue, arrayBin);
 				 symbolCounter += 4 * (num-1);
 			}
 			else
 			{
 				int num = atoi(stripped);
 				convertToBinary(num, bin, 32, 0);
 				bin[33] = '\0';

 				symbols[symbolPos].symbolValue = malloc(sizeof(bin));
 				strcpy(symbols[symbolPos].symbolValue, bin);

 				bin[0] = '\0';
 			}
 		}

 		symbolPos++;
 	}

 	fclose(DATA);


 	///////////TEXT SECTION/////////////////
 	symbolCounter = 0;
 	rewind(TEXT);
 	TEXT = fopen("text.txt", "r");

 	while(fgets(line, sizeof(line), TEXT) != NULL)
 	{
 		int found = lineContains(line, ":");
		if (found != -1)
		{
			char strippedLine[found+1];
 			strncpy(strippedLine, line, found);
 			strippedLine[found] = '\0';

 			symbols[symbolPos].symbolName = malloc(sizeof(strippedLine));
 			strcpy(symbols[symbolPos].symbolName, strippedLine);

 			symbolToHex(hex);
 			symbols[symbolPos].symbolAddress = malloc(sizeof(hex));
 			strcpy(symbols[symbolPos].symbolAddress, hex);

 			symbols[symbolPos].symbolAddressInt = symbolCounter;

 			symbolPos++;
		}

		int i;
		int processed = 0;
		for (i = 0; i < OP_MAP_SIZE; i++)
		{
			if (!processed)
			{
				int found = lineContains(line, opcodes[i].opName);
				if (found != -1)
				{
					if (opcodes[i].opType == "pseudo")
					{
						symbolCounter += opcodes[i].pseudoSize * 4;
					}
					else
					{
						symbolCounter += 4;
					}
					processed = 1;
				}
			}
		}
 	}

 	rewind(TEXT);
 	symbolCounter = 0;

 	while(fgets(line, sizeof(line), TEXT) != NULL)
	{
		int i, found;
		int processed = 0;
		for (i = 0; i < OP_MAP_SIZE; i++)
		{
			found = lineContains(line, opcodes[i].opName);
			if (found != -1)
			{
				char registerLine[50];
				char bin[33];
				findEndRemoveSpaces(line, registerLine, found+5);
				if (!processed)
				{
					if (opcodes[i].opType == "pseudo")
					{
						int j;
						for (j = 0; j < opcodes[i].pseudoSize; j++)
						{
							int k;
							for (k = 0; k < OP_MAP_SIZE; k++)
							{
								if (opcodes[k].opName == opcodes[i].opPseudos[j])
								{
									if (opcodes[k].opType == "itype")
									{
										processTwoParams(registerLine, reg1, reg2);
										constructIType2Binary(opcodes[k].opName, reg1, reg2, NULL);
									}
									else if (opcodes[k].opType == "rtype")
									{
										processRType(registerLine, reg1, reg2, reg3);		
										if (opcodes[i].opName == "ble")
										{			
											constructRTypeBinary(opcodes[k].opName, "$at", reg2, reg1);
										}
										else if (opcodes[i].opName == "blt")
										{
											constructRTypeBinary(opcodes[k].opName, "$at", reg1, reg2);
										}
									}
									else if (opcodes[k].opType == "jtype")
									{
										processRType(registerLine, reg1, reg2, reg3);
										constructJTypeBinary(opcodes[k].opName, "$at", "$zero", reg3, symbolCounter);
									}
								}
							}
							symbolCounter += 4;
						}
					}
					else
					{
						analyzeLine(i, registerLine);
						symbolCounter += 4;
					}
					processed = 1;
				}
			}
		}
	}

	int i;
	for (i = 0; i < symbolPos; i++)
	{
		if (symbols[i].symbolValue != NULL)
		{
			fprintf(ENDFILE, "%s", symbols[i].symbolValue);
		}
	}

	fprintf(ENDFILE, "\n");

	if (argc == 4)
	{
		fclose(ENDFILE);
		ENDFILE = fopen(argv[3], "w");

		fprintf(ENDFILE, "Address\t\t\t  Symbol\n");
		fprintf(ENDFILE, "---------------------------\n");
		int i;
		for (i = 1; i < symbolPos; i++)
		{
			fprintf(ENDFILE, "%s\t\t\t%s\n", symbols[i].symbolAddress, symbols[i].symbolName);
		}
	}
	
 	fclose(INPUT);
 	fclose(TEXT);
 	fclose(ENDFILE);
 	remove("data.txt");
 	remove("text.txt");
 }







int countElements(char *str)
{
	fprintf(ENDFILE, "%s", str);
	int i;
	int count = 0;
	for (i = 0; i < strlen(str); i++)
	{
		if (str[i] == ',')
		{
			count++;
		}
	}

	return count;
}

void parseArray(char *str, char *newString)
{	
	int i;
	int j = 0;

	for (i = 0; str[i] != ','; i++)
	{
		newString[j] = str[i];
		j++;
	}
	newString[i] = '\0';
	
	j = 0;
	for (i += 1; str[i] != '\0'; i++)
	{
		str[j] = str[i];
		j++;
	}
	str[j] = '\0';
}

void analyzeLine(int i, char *registerLine)
{
	if (opcodes[i].opType == "itype2")
	{
		reg3[0] = '\0';
		processIType2(registerLine, reg1, reg2, reg3);
		//removeNewLines(reg2);
		//fprintf(ENDFILE, "%s  %s  %s\n", reg1, reg2, reg3);
		if (reg3[0] == '\0')
		{
			//fprintf(ENDFILE, "%s\n", "NULL");
			processTwoParams(registerLine, reg1, reg2);
			constructIType2Binary(opcodes[i].opName, reg1, reg2, "$zero");
		}
		else
		{
			processIType2(registerLine, reg1, reg2, reg3);
			constructIType2Binary(opcodes[i].opName, reg1, reg2, reg3);
		}
	}

	if (opcodes[i].opType == "rtype")
	{
		processRType(registerLine, reg1, reg2, reg3);
		constructRTypeBinary(opcodes[i].opName, reg1, reg2, reg3);
	}

	if (opcodes[i].opType == "shift")
	{
		processRType(registerLine, reg1, reg2, reg3);
		constructShiftBinary(opcodes[i].opName, reg2, reg1, reg3);
	}

	if (opcodes[i].opType == "itype")
	{
		processRType(registerLine, reg1, reg2, reg3);
		constructIType2Binary(opcodes[i].opName, reg1, reg3, reg2);
	}

	if (opcodes[i].opType == "jtype")
	{
		processRType(registerLine, reg1, reg2, reg3);
		constructJTypeBinary(opcodes[i].opName, reg1, reg2, reg3, symbolCounter);
	}

	if (opcodes[i].opType == "tptype")
	{
		processTwoParams(registerLine, reg1, reg2);

		if (opcodes[i].opName == "lui")
		{
			constructIType2Binary(opcodes[i].opName, reg1, reg2, NULL);
		}
		else
		{
			constructJTypeBinary(opcodes[i].opName, reg1, NULL, reg2, symbolCounter);
		}
	}

	if (opcodes[i].opName == "syscall")
	{
		fprintf(ENDFILE, "%s\n", opcodes[i].opCode);
	}

	if (opcodes[i].opName == "nop")
	{
		fprintf(ENDFILE, "%s\n", opcodes[i].opCode);
	}

	if (opcodes[i].opName == "j")
	{
		constructJumpBinary(registerLine);
	}
}

int lineContains(char *line, char *search)
{
	char *found = strstr(line, search);
	if (found)
	{
		return found - line;
	}

	return -1;
}

void symbolToHex(char* hex)
{
	sprintf(hex, "0x%08x", symbolCounter);
}

void findEndRemoveSpaces(char *line, char *newString, int pos)
{
	int i;
	int j = 0;
	for (i = pos; line[i] != '\0'; i++)
	{
		if (line[i] != ' ')
		{
			newString[j] = line[i];
			j++;
			newString[j] = '\0';
		}
	}
}

//-----------POSSIBLE ERROR DUE TO NULL-TERMINATOR------------------//
//If a user is to pass an immediate, we need to convert it to binary
void convertToBinary(int num, char *str, int length, int mode)
{
	if (mode == 0)
	{
		str[0] = '\0';
	}
	int i;
	int converted;
	for (i = length-1; i >= 0; i--)
	{
		converted = num >> i;

		if (converted & 1)
		{
			strcat(str, "1");
		}
		else
		{
			strcat(str, "0");
		}
	}
}

void processRType(char *line, char *reg1, char *reg2, char *reg3)
{
	int found = lineContains(line, ",");
	if (found != -1)
	{
		char newString[50];
		findEndRemoveSpaces(line, newString, found+1);
		strcpy(reg2, newString);

		int found2 = lineContains(newString, ",");
		if (found2 != -1)
		{
			char strippedLine[found2+1];
			strncpy(strippedLine, newString, found2);
			strippedLine[found2] = '\0';
			strcpy(reg2, strippedLine);
			findEndRemoveSpaces(newString, newString, found2+1);
			removeNewLines(newString);
			strcpy(reg3, newString);
		}

		char strippedLine[found+1];
		strncpy(strippedLine, line, found);
		strippedLine[found] = '\0';
		strcpy(reg1, strippedLine);
	}
}

void processIType2(char *line, char *reg1, char *reg2, char *reg3)
{
	int found = lineContains(line, ",");
	if (found != -1)
	{
		char newString[50];
		findEndRemoveSpaces(line, newString, found+1);
		strcpy(reg2, newString);

		int found2 = lineContains(newString, "(");
		if (found2 != -1)
		{
			char strippedLine[found2+1];
			strncpy(strippedLine, newString, found2);
			strippedLine[found2] = '\0';
			strcpy(reg2, strippedLine);
			findEndRemoveSpaces(newString, newString, found2+1);
			removeNewLines(newString);
			newString[strlen(newString)-1] = '\0';
			strcpy(reg3, newString);
		}

		char strippedLine[found+1];
		strncpy(strippedLine, line, found);
		strippedLine[found] = '\0';
		strcpy(reg1, strippedLine);
	}
}

void processTwoParams(char *line, char *reg1, char *reg2)
{
	int found = lineContains(line, ",");
	if (found != -1)
	{
		char newString[50];
		findEndRemoveSpaces(line, newString, found+1);
		removeNewLines(newString);
		strcpy(reg2, newString);

		char strippedLine[found+1];
		strncpy(strippedLine, line, found);
		strippedLine[found] = '\0';
		strcpy(reg1, strippedLine);
	}
}

void removeNewLines(char *line)
{
	int i;
	for (i = 0; line[i] != '\0'; i++)
	{
		if (line[i] == '\n')
		{
			line[i] = '\0';
		}
	}
}

void constructJumpBinary(char *reg1)
{
	fprintf(ENDFILE, "%s", "00001\0");
	removeNewLines(reg1);

	int i = 1;
	int found = 0;
	while (i < symbolTableSize && !found)
	{
		if (strcmp(symbols[i].symbolName, reg1) == 0)
		{
			char str[28];
			int num = (symbols[i].symbolAddressInt / 4);
			convertToBinary(num, str, 27, 0);
			str[27] = '\0';
			fprintf(ENDFILE, "%s\n", str);
			found = 1;
		}
		i += 1;
	}
}

void constructRTypeBinary(char *opName, char *reg1, char *reg2, char *reg3)
{
	int i;
	char defaultCode[6] = "00000\0";

	for (i = 0; i < OP_MAP_SIZE; i++)
	{
		if (strcmp(opcodes[i].opName, opName) == 0)
		{
			fprintf(ENDFILE, "%s", opcodes[i].opSpecial);
		}
	}

	for (i = 0; i < REG_MAP_SIZE; i++)
	{
		if (strcmp(registers[i].regName, reg2) == 0)
		{
			fprintf(ENDFILE, "%s", registers[i].regNumber);
		}
	}

	for (i = 0; i < REG_MAP_SIZE; i++)
	{
		if (strcmp(registers[i].regName, reg3) == 0)
		{
			fprintf(ENDFILE, "%s", registers[i].regNumber);
		}
	}

	for (i = 0; i < REG_MAP_SIZE; i++)
	{
		if (strcmp(registers[i].regName, reg1) == 0)
		{
			fprintf(ENDFILE, "%s", registers[i].regNumber);
		}
	}

	fprintf(ENDFILE, "%s", defaultCode);

	for (i = 0; i < OP_MAP_SIZE; i++)
	{
		if (strcmp(opcodes[i].opName, opName) == 0)
		{
			fprintf(ENDFILE, "%s\n", opcodes[i].opCode);
		}
	}
}

void constructJTypeBinary(char *opName, char *reg1, char *reg2, char *reg3, int pc)
{
	int i;

	for (i = 0; i < OP_MAP_SIZE; i++)
	{
		if (strcmp(opcodes[i].opName, opName) == 0)
		{
			fprintf(ENDFILE, "%s", opcodes[i].opCode);
		}
	}

	for (i = 0; i < REG_MAP_SIZE; i++)
	{
		if (strcmp(registers[i].regName, reg1) == 0)
		{
			fprintf(ENDFILE, "%s", registers[i].regNumber);
		}
	}

	if (reg2 == NULL)
	{
		fprintf(ENDFILE, "%s", "00000\0");
	}
	else
	{
		for (i = 0; i < REG_MAP_SIZE; i++)
		{
			if (strcmp(registers[i].regName, reg2) == 0)
			{
				fprintf(ENDFILE, "%s", registers[i].regNumber);
			}
		}
	}

	i = 1;
	int found = 0;
	while (i < symbolTableSize && !found)
	{
		if (strcmp(symbols[i].symbolName, reg3) == 0)
		{
			char str[17];
			int num = (symbols[i].symbolAddressInt - (pc + 4)) / 4;
			convertToBinary(num, str, 16, 0);
			str[16] = '\0';
			fprintf(ENDFILE, "%s\n", str);
			found = 1;
		}
		i += 1;
	}
}

void constructPseudoBinary(char *opName, char *reg1, char *reg2, char *reg3)
{
	int i;
	char opCode[7];
	char reg1Code[6];
	char reg2Code[6];
	char reg3Code[17];

	for (i = 0; i < OP_MAP_SIZE; i++)
	{
		if (strcmp(opcodes[i].opName, opName) == 0)
		{
			strncpy(opCode, opcodes[i].opCode, 6);
			opCode[6] = '\0';
			fprintf(ENDFILE, "%s", opCode);
		}
	}

	for (i = 0; i < REG_MAP_SIZE; i++)
	{
		if (strcmp(registers[i].regName, reg1) == 0)
		{
			strncpy(reg1Code, registers[i].regNumber, 5);
			reg1Code[5] = '\0';
			fprintf(ENDFILE, "%s", reg1Code);
		}
	}

	for (i = 0; i < REG_MAP_SIZE; i++)
	{
		if (strcmp(registers[i].regName, reg2) == 0)
		{
			strncpy(reg2Code, registers[i].regNumber, 5);
			reg2Code[5] = '\0';
			fprintf(ENDFILE, "%s", reg2Code);
		}
	}

	for (i = 0; i < symbolPos; i++)
	{
		if (symbols[i].symbolName != NULL)
		{
			if (strcmp(symbols[i].symbolName, reg3) == 0)
			{
				int x;
				sscanf(symbols[i].symbolAddress, "%x", &x);
				convertToBinary(x, reg3Code, 16, 0);
				reg3Code[16] = '\0';
				fprintf(ENDFILE, "%s\n", reg3Code);
			}
		}
	}
}

void constructIType2Binary(char *opName, char *reg1, char *reg2, char *reg3)
{
	int i;
	char opCode[7];
	char reg1Code[6];
	char reg3Code[6];
	char regOffset[17];

	for (i = 0; i < OP_MAP_SIZE; i++)
	{
		if (strcmp(opcodes[i].opName, opName) == 0)
		{
			strncpy(opCode, opcodes[i].opCode, 6);
			opCode[6] = '\0';
			fprintf(ENDFILE, "%s", opCode);
		}
	}

	if (reg3 == NULL)
	{
		fprintf(ENDFILE, "%s", "00000\0");
	}
	else
	{
		for (i = 0; i < REG_MAP_SIZE; i++)
		{
			if (strcmp(registers[i].regName, reg3) == 0)
			{
				strncpy(reg3Code, registers[i].regNumber, 5);
				reg3Code[5] = '\0';
				fprintf(ENDFILE, "%s", reg3Code);
			}
		}
	}

	for (i = 0; i < REG_MAP_SIZE; i++)
	{
		if (strcmp(registers[i].regName, reg1) == 0)
		{
			strncpy(reg1Code, registers[i].regNumber, 5);
			reg1Code[5] = '\0';
			fprintf(ENDFILE, "%s", reg1Code);
		}
	}

	int found = 0;
	for (i = 0; i < symbolPos; i++)
	{
		if (symbols[i].symbolName != NULL)
		{
			if (strcmp(symbols[i].symbolName, reg2) == 0)
			{
				convertToBinary(symbols[i].symbolAddressInt, regOffset, 16, 0);
				regOffset[17] = '\0';
				fprintf(ENDFILE, "%s\n", regOffset);
				found = 1;
			}
		}
	}

	if (!found)
	{
		int num = atoi(reg2);
		convertToBinary(num, regOffset, 16, 0);
		regOffset[17] = '\0';

		fprintf(ENDFILE, "%s\n", regOffset);
	}
}

void constructShiftBinary(char *opName, char *reg1, char *reg2, char *reg3)
{
	int i;
	char defaultCode[6] = "00000\0";

	for (i = 0; i < OP_MAP_SIZE; i++)
	{
		if (strcmp(opcodes[i].opName, opName) == 0)
		{
			fprintf(ENDFILE, "%s", opcodes[i].opSpecial);
		}
	}

	fprintf(ENDFILE, "%s", defaultCode);

	for (i = 0; i < REG_MAP_SIZE; i++)
	{
		if (strcmp(registers[i].regName, reg1) == 0)
		{
			fprintf(ENDFILE, "%s", registers[i].regNumber);
		}
	}

	for (i = 0; i < REG_MAP_SIZE; i++)
	{
		if (strcmp(registers[i].regName, reg2) == 0)
		{
			fprintf(ENDFILE, "%s", registers[i].regNumber);
		}
	}

	char shiftAmount[6];
	int num = atoi(reg3);
	convertToBinary(num, shiftAmount, 5, 0);
	shiftAmount[5] = '\0';

	fprintf(ENDFILE, "%s", shiftAmount);

	for (i = 0; i < OP_MAP_SIZE; i++)
	{
		if (strcmp(opcodes[i].opName, opName) == 0)
		{
			fprintf(ENDFILE, "%s\n", opcodes[i].opCode);
		}
	}
}

void convertTo16Bit(char *line, char *converted)
{
	int i;
	for (i = 0; i < 16; i++)
	{
		converted[i] = line[i+16];
	}
}