/*
 * Seung Jae Lim
 * Prof Brian Russel
 * Computer Arch
 * 
 * y86emul.c
 */

#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "y86emul.h"
#include "hex_conversion.h"

#define DISASSEMBLER1 true
#define DISASSEMBLER2 true

/* Obtained from http:// stackoverflow.com/questions/174531/easiest-way-to-get-files-contents-in-c */
char * y86_in_string(const char * y86){
	char * buffer = 0;
	long length;
	FILE * f = fopen(y86, "rb");
	if(f){
		fseek(f, 0, SEEK_END);
		length = ftell(f);
		fseek(f,0,SEEK_SET);
		buffer = malloc(length);
		if(buffer){
			fread(buffer,1,length,f);
		}
		fclose(f);
	} else {
		printf("No file with that name exist!\n");
	}

	if(buffer){
		return buffer;
	} else {
		printf("There is something wrong with the y86 file. \n");
		return "";
	}
}

/* Rewrote substr() because for some reason, it wasn't working */
char * substr(const char *firstChar, char *secondChar, size_t start, size_t size){
	/*printf("firstChar+start = %s\n", firstChar + start);
	printf("secondChar = %s\n", secondChar);
	printf("size = %zu\n", size);
	printf("I'm still here\n");*/
	strncpy(secondChar, (firstChar + start), size);
	secondChar[size] = '\0';
	return secondChar;
}

char * strdup(const char *s){
	char *d = malloc (strlen(s) + 1);
	if(d == NULL){
		return NULL;
	}
	strcpy(d,s);
	return d;
}

/* Idea found from: http:// stackoverflow.com/questions/25327211/how-to-split-hex-byte-of-an-ascii-character */
int the_actual_actual_program(unsigned char * bytes, computer * computer){
	unsigned char i;
	unsigned char j;

	char c0;
	int c1;

	int V;
	unsigned int D; /* Destination */

	computer->HLTfound = false;

	while(!(computer->HLTfound)){
		switch(bytes[computer->counter]){
			case 0x00: /* nop */
				if(DISASSEMBLER1){
					printf("nop\tcounter:%x\n", computer->counter);
				}
				computer->counter++;
				break;
			case 0x10: /* halt */
				if(DISASSEMBLER1){
					printf("halt\tcounter:%x\n", computer->counter);
				}
				printf(HLT);
				printf(AOK);
				computer->HLTfound = true;
				return 0;
				break;
			case 0x20: /* rrmovl ; register to register */
				i = (bytes[computer->counter + 1] & 0xf0) >> 4;
				j = (bytes[computer->counter + 1] & 0x0f);

				if(DISASSEMBLER1){
					printf("rrmovl\tcounter:%x\trA:%x\trB:%x\n",computer->counter, i,j);
				}

				computer->registers[j] = computer->registers[i];

				computer->counter += 2;
				break;
			case 0x30: /* irmovl ; immediate to register */
				i = (bytes[computer->counter + 1] & 0xf0) >> 4;
				j = (bytes[computer->counter + 1] & 0x0f);
				V = bytes[computer->counter + 2] + (bytes[computer->counter + 3] << 8)
					+ (bytes[computer->counter + 4] << 16) + (bytes[computer->counter + 5] << 24);

				if(DISASSEMBLER1){
					printf("irmovl\tcounter:%x", computer->counter);
					printf("\tF:%x\trB:%x\tV:%x\n", i,j,V);
				}

				computer->registers[j] = V;

				computer->counter += 6;

				break;
			case 0x40: /* rmmovl ; register to memeory */
				i = (bytes[computer->counter + 1] & 0xf0) >> 4;
				j = (bytes[computer->counter + 1] & 0x0f);
				D = bytes[computer->counter + 2] + (bytes[computer->counter + 3] << 8) 
					+ (bytes[computer->counter + 4] << 16) + (bytes[computer->counter + 5] << 24);

				bytes[computer->registers[j] + D + 0] = (computer->registers[i] & 0x000000ff);
				bytes[computer->registers[j] + D + 1] = (computer->registers[i] & 0x0000ff00) >> 8;
				bytes[computer->registers[j] + D + 2] = (computer->registers[i] & 0x00ff0000) >> 16;
				bytes[computer->registers[j] + D + 3] = (computer->registers[i] & 0xff000000) >> 24;

				if(DISASSEMBLER1){
					printf("rmmvol\tcounter:%x", computer->counter);
					printf("\trA:%x\trB:%x\tD:%x\n", i, j, D);
				}

				computer->counter += 6;
				break;
			case 0x50: /* mrmovl ; memory to register */
				i = (bytes[computer->counter + 1] & 0xf0) >> 4;
				j = (bytes[computer->counter + 1] & 0x0f);
				D = bytes[computer->counter + 2] + (bytes[computer->counter + 3] << 8)
					+ (bytes[computer->counter + 4] << 16) + (bytes[computer->counter + 5] << 24);

				computer->registers[i] = (bytes[computer->registers[j] + D + 0]) 
										+ (bytes[computer->registers[j] + D + 1] << 8)
										+ (bytes[computer->registers[j] + D + 2] << 16)
										+ (bytes[computer->registers[j] + D + 3] << 24);

				if(DISASSEMBLER1){
					printf("mrmovl\tcounter:%x", computer->counter);
					printf("\trA:%x\trB:%x\tD:%x\n",i,j,D);
				}

				computer->counter += 6;
				break;
			case 0x60: /* addl */
				i = (bytes[computer->counter + 1] & 0xf0) >> 4;
				j = (bytes[computer->counter + 1] & 0x0f);
				computer->OF = false;
				computer->SF = false;
				computer->ZF = false;

				V = computer->registers[i] + computer->registers[j];

				if((computer->registers[i] > 0 && computer->registers[j] > 0 && V < 0) || 
					(computer->registers[i] < 0 && computer->registers[j] < 0 && V > 0)) {
					computer->OF = true;
				}

				if(V == 0){
					computer->ZF = true;
				}

				if(V < 0){
					computer->SF = true;
				}

				if(DISASSEMBLER1){
					printf("addl\tcounter:%x", computer->counter);
					printf("\trA:%x\trB:%x", i, j);
					printf("\t(%i = %i + %i (%i))\n", V, computer->registers[i], computer->registers[j], computer->OF);
				}

				computer->registers[j] = V;
				computer->counter += 2;

				break;
			case 0x61: /* subl */
				i = (bytes[computer->counter + 1] & 0xf0) >> 4;
				j = (bytes[computer->counter + 1] & 0x0f);
				computer->OF = false;
				computer->SF = false;
				computer->ZF = false;

				V = computer->registers[j] - computer->registers[i];

				if((computer->registers[i] < 0 && computer->registers[j] > 0 && V < 0) || 
					(computer->registers[i] > 0 && computer->registers[j] < 0 && V > 0)) {
					computer->OF = true;
				}
				if(V == 0){
					computer->ZF = true;
				}
				if(V < 0){
					computer->SF = true;
				}

				if(DISASSEMBLER1){
					printf("subl\tcounter:%x", computer->counter);
					printf("\trA:%x\trB:%x", i, j);
					printf("\t(%i = %i - %i (%i))\n", V, computer->registers[j], computer->registers[i], computer->OF);
				}

				computer->registers[j] = V;
				computer->counter += 2;

				break;
			case 0x62: /* andl */
				i = (bytes[computer->counter + 1] & 0xf0) >> 4;
				j = (bytes[computer->counter + 1] & 0x0f);
				computer->SF = false;
				computer->ZF = false;
				V = computer->registers[i] & computer->registers[j];
				computer->registers[j] = V;
				if(DISASSEMBLER1){
					printf("andl\tcounter:%x", computer->counter);
					printf("\trA:%x\trB:%x\tV:%x\n", i, j, V);
				}

				if(V == 0){
					computer->ZF = true;
				}

				if(V < 0){
					computer->SF = true;
				}

				computer->counter += 2;
				break;
			case 0x63: /* xorl */ 
				i = (bytes[computer->counter + 1] & 0xf0) >> 4;
				j = (bytes[computer->counter + 1] & 0x0f);
				computer->SF = false;
				computer->ZF = false;

				if(DISASSEMBLER1){
					printf("xorl\tcounter:%x", computer->counter);
					printf("\trA:%x\trB:%x\n", i, j);
				}

				V = computer->registers[i] ^ computer->registers[j];
				if(V == 0){
					computer->ZF = true;
				}
				if(V < 0){
					computer->SF = true;
				}
				computer->registers[j] = V;
				computer->counter += 2;
				break;
			case 0x64: /* mull */
				i = (bytes[computer->counter + 1] & 0xf0) >> 4;
				j = (bytes[computer->counter + 1] & 0x0f);
				computer->OF = false;
				computer->SF = false;
				computer->ZF = false;

				V = computer->registers[i] * computer->registers[j];

				if((computer->registers[i] > 0 && computer->registers[j] > 0 && V < 0) ||
					(computer->registers[i] < 0 && computer->registers[j] < 0 && V < 0) ||
					(computer->registers[i] < 0 && computer->registers[j] > 0 && V > 0) ||
					(computer->registers[i] > 0 && computer->registers[j] < 0 && V > 0)) {
					computer->OF = true;
				}

				if(V == 0){
					computer->ZF = true;
				}

				if(V < 0){
					computer->SF = true;
				}

				if(DISASSEMBLER1){
					printf("mull\tcounter:%x", computer->counter);
					printf("\trA:%x\trB:%x", i, j);
					printf("\t(%i = %i * %i (%i))\n", V, computer->registers[i], computer->registers[j], computer->OF);
				}

				computer->registers[j] = V;
				computer->counter += 2;

				break;
			case 0x65: /* cmpl */
				i = (bytes[computer->counter + 1] & 0xf0) >> 4;
				j = (bytes[computer->counter + 1] & 0x0f);
				computer->SF = false;
				computer->ZF = false;
				computer->OF = true;

				V = computer->registers[j] - computer->registers[i];

				if(V == 0){
					computer->ZF = true;
				}
				else if(V < 0){
					computer->SF = true;
				}
				else if((V > 0 && computer->registers[i] > 0 && computer->registers[j] < 0) 
					|| (V < 0 && computer->registers[i] < 0 && computer->registers[j] > 0)){
					computer->OF = true;
				}
				if(DISASSEMBLER1){
					printf("cmpl\tcounter:%x\t", computer->counter);
					printf("rA:%x\trB:%x\n", i, j);
				}
				/*computer->registers[j] = V;*/
				computer->counter += 2;
				break;
			case 0x70: /* jmp */
				D = bytes[computer->counter + 1] 
					+ (bytes[computer->counter + 2] << 8)
					+ (bytes[computer->counter + 3] << 16)
					+ (bytes[computer->counter + 4] << 24);

				if(DISASSEMBLER1){
					printf("jmp\tcounter:%x\tD:%x\n", computer->counter, D);
				}

				computer->counter = D;

				break;
			case 0x71: /* jle */
				D = bytes[computer->counter + 1] 
					+ (bytes[computer->counter + 2] << 8)
					+ (bytes[computer->counter + 3] << 16)
					+ (bytes[computer->counter + 4] << 24);

				if(DISASSEMBLER1){
					printf("jle\tcounter:%x\tD:%x\n", computer->counter, D);
				}

				if((computer->SF ^ computer->OF) || computer->ZF){
					computer->counter = D;
				}
				else{
					computer->counter += 5;
				}
				break;
			case 0x72: /* jl */
				D = bytes[computer->counter + 1] 
					+ (bytes[computer->counter + 2] << 8)
					+ (bytes[computer->counter + 3] << 16)
					+ (bytes[computer->counter + 4] << 24);

				if(DISASSEMBLER1){
					printf("jl\tcounter:%x\tD:%x\n", computer->counter, D);
				}

				if(computer->SF ^ computer->OF){
					computer->counter = D;
				}
				else{
					computer->counter += 5;
				}
				break;
			case 0x73: /* je */
				D = bytes[computer->counter + 1] 
					+ (bytes[computer->counter + 2] << 8)
					+ (bytes[computer->counter + 3] << 16)
					+ (bytes[computer->counter + 4] << 24);

				if(DISASSEMBLER1){
					printf("je\tcounter:%x\tD:%x\n", computer->counter, D);
				}
				if(computer->ZF){
					computer->counter = D;
				}
				else{
					computer->counter += 5;
				}

				break;
			case 0x74: /* jne */
				D = bytes[computer->counter + 1] 
					+ (bytes[computer->counter + 2] << 8)
					+ (bytes[computer->counter + 3] << 16)
					+ (bytes[computer->counter + 4] << 24);

				if(DISASSEMBLER1){
					printf("jne\tcounter:%x\tD:%x\n", computer->counter, D);
				}

				if(!(computer->ZF)){
					computer->counter = D;
				}
				else{
					computer->counter += 5;
				}

				break;
			case 0x75: /* jge */
				D = bytes[computer->counter + 1] 
					+ (bytes[computer->counter + 2] << 8)
					+ (bytes[computer->counter + 3] << 16)
					+ (bytes[computer->counter + 4] << 24);

				if(DISASSEMBLER1){
					printf("jge\tcounter:%x\tD:%x\n", computer->counter, D);
				}

				if(!(computer->SF ^ computer->OF)){
					computer->counter = D;
				}
				else{
					computer->counter += 5;
				}

				break;
			case 0x76: /* jg */
				D = bytes[computer->counter + 1] 
					+ (bytes[computer->counter + 2] << 8)
					+ (bytes[computer->counter + 3] << 16)
					+ (bytes[computer->counter + 4] << 24);

				if(DISASSEMBLER1){
					printf("jg\tcounter:%x\tD:%x\n", computer->counter, D);
				}

				if(!(computer->SF ^ computer->OF) && !computer->ZF){
					computer->counter = D;
				}
				else{
					computer->counter += 5;
				}

				break;
			case 0x80: /* call */
				D = bytes[computer->counter + 1] 
					+ (bytes[computer->counter + 2] << 8)
					+ (bytes[computer->counter + 3] << 16)
					+ (bytes[computer->counter + 4] << 24);

				if(DISASSEMBLER1){
					printf("call\tcounter:%x\tD:%x\tregister[ESP]:%x\t", computer->counter, D, computer->registers[ESP]);
				}
				computer->registers[ESP] -= 4;
				c1 = computer->counter + 5;
				V = 0;

				bytes[computer->registers[ESP] + 0] = (c1 & 0x000000ff);
				bytes[computer->registers[ESP] + 1] = (c1 & 0x0000ff00) >> 8;
				bytes[computer->registers[ESP] + 2] = (c1 & 0x00ff0000) >> 16;
				bytes[computer->registers[ESP] + 3] = (c1 & 0xff000000) >> 24;

				if(DISASSEMBLER1){
					printf("After:%x\n",bytes[computer->registers[ESP]+i]);
				}
				computer->counter = D;

				break;
			case 0x90: /* ret (return) */
				if(DISASSEMBLER1){
					printf("ret\tcounter:%x\tregisters[ESP-4]:%x", computer->counter, bytes[computer->registers[ESP-4]]);
				}
				computer->counter = (bytes[computer->registers[ESP]]) 
									+ (bytes[computer->registers[ESP] + 1] << 8)
									+ (bytes[computer->registers[ESP] + 2] << 16)
									+ (bytes[computer->registers[ESP] + 3] << 24);
				if(DISASSEMBLER1){
					printf("new counter:%x\n", computer->counter);
				}
				computer->registers[ESP] += 4;
				break; 
			case 0xa0: /* pushl */
				i = (bytes[computer->counter + 1] & 0xf0) >> 4;
				j = (bytes[computer->counter + 1] & 0x0f);
				if(DISASSEMBLER1){
					printf("pushl\tcounter:%x\trA:%x\trB:%x\tregisters[ESP]:%x\n",computer->counter, i,j, computer->registers[ESP]);
				}
				computer->registers[ESP] -= 4;

				bytes[computer->registers[ESP] + 0] = computer->registers[i];
				bytes[computer->registers[ESP] + 1] = (computer->registers[i] & 0x0000ff00) >> 8;
				bytes[computer->registers[ESP] + 2] = (computer->registers[i] & 0x00ff0000) >> 16;;
				bytes[computer->registers[ESP] + 3] = (computer->registers[i] & 0xff000000) >> 24;;
				
				computer->counter += 2;
				break;
			case 0xb0: /* popl */
				i = (bytes[computer->counter + 1] & 0xf0) >> 4;
				j = (bytes[computer->counter + 1] & 0x0f);
				if(DISASSEMBLER1){
					printf("popl\tcounter:%x\trA:%x\trB:%x\n", computer->counter, i,j);
				}
				computer->registers[i] = (bytes[computer->registers[ESP]])
										+ (bytes[computer->registers[ESP] + 1] << 8)
										+ (bytes[computer->registers[ESP] + 2] << 16)
										+ (bytes[computer->registers[ESP] + 3] << 24);

				computer->registers[ESP] += 4;
				computer->counter += 2;
				break;
			case 0xc0: /* readb */
				i = (bytes[computer->counter + 1] & 0xf0) >> 4;
				j = (bytes[computer->counter + 1] & 0x0f);
				D = bytes[computer->counter + 2] 
					+ (bytes[computer->counter + 3] << 8)
					+ (bytes[computer->counter + 4] << 16)
					+ (bytes[computer->counter + 5] << 24);

				if(DISASSEMBLER2){
					printf("readb\tcounter:%x\trA:%x\trB:%x\tD:%x\n", computer->counter, i,j,D);
				}

				if(EOF == scanf("%c", &c0)) {
					computer->ZF = true;
				}
				else{
					computer->ZF = false;
				}

				bytes[computer->registers[i] + D] = c0;
				computer->counter += 6;
				break;
			case 0xc1: /* readl */
				i = (bytes[computer->counter + 1] & 0xf0) >> 4;
				j = (bytes[computer->counter + 1] & 0x0f);
				D = bytes[computer->counter + 2] 
					+ (bytes[computer->counter + 3] << 8)
					+ (bytes[computer->counter + 4] << 16)
					+ (bytes[computer->counter + 5] << 24);

				if(EOF == scanf("%i", &c1)){
					computer->ZF = true;
				}
				else{
					computer->ZF = false;
				}

				bytes[computer->registers[i] + D] = c1 & 0x000000ff;
				bytes[computer->registers[i] + D + 1] = (c1 & 0x0000ff00) >> 8;
				bytes[computer->registers[i] + D + 2] = (c1 & 0x00ff0000) >> 16;
				bytes[computer->registers[i] + D + 3] = (c1 & 0xff000000) >> 24;

				int l;
				if(DISASSEMBLER2){
					printf("readl\tcounter:%x\trA:%x\trB:%x\tD:%i\tc1:%i\tbytes[computer->registers[i]] = ", computer->counter, i,j,D,c1);
					for(l = 0; l < 4; l++){
						printf("%i", bytes[computer->registers[i] + D + l]);
					}
					printf("\n");
				}
				computer->counter += 6;
				break;
			case 0xd0: /* writeb */
				i = (bytes[computer->counter + 1] & 0xf0) >> 4;
				j = (bytes[computer->counter + 1] & 0x0f);
				D = bytes[computer->counter + 2] 
					+ (bytes[computer->counter + 3] << 8)
					+ (bytes[computer->counter + 4] << 16)
					+ (bytes[computer->counter + 5] << 24);

				if(DISASSEMBLER2){
					printf("writeb\tcounter:%x\tbytes[%x + %x] - ", computer->counter, computer->registers[i], D);
				}


				printf("%c", (char)bytes[computer->registers[i] + D]);

				if(DISASSEMBLER2){
					printf("\n");
				}
				computer->counter += 6;
				break;
			case 0xd1: /* writel */
				i = (bytes[computer->counter + 1] & 0xf0) >> 4;
				j = (bytes[computer->counter + 1] & 0x0f);
				D = bytes[computer->counter + 2] 
					+ (bytes[computer->counter + 3] << 8)
					+ (bytes[computer->counter + 4] << 16)
					+ (bytes[computer->counter + 5] << 24);
				if(DISASSEMBLER2){
					printf("writel\tcounter:%x\trA:%x\trB:%x\tD:%x\n", computer->counter,i,j,D);
				}
				V = (bytes[computer->registers[i] + D])
					+ (bytes[computer->registers[i] + D + 1] << 8)
					+ (bytes[computer->registers[i] + D + 2] << 16)
					+ (bytes[computer->registers[i] + D + 3] << 24);

				printf("%i", V);

				computer->counter += 6;
				break;
			case 0xe0: /* movsbl */
				i = (bytes[computer->counter + 1] & 0xf0) >> 4;
				j = (bytes[computer->counter + 1] & 0x0f);
				D = bytes[computer->counter + 2]
					+ (bytes[computer->counter + 3] << 8)
					+ (bytes[computer->counter + 4] << 16)
					+ (bytes[computer->counter + 5] << 24);


				V = (bytes[computer->registers[j] + D])
					+ (bytes[computer->registers[j]+ D + 1] << 8)
					+ (bytes[computer->registers[j] + D + 2] << 16)
					+ (bytes[computer->registers[j] + D + 3] << 24);

				if(DISASSEMBLER1){
					printf("movsbl\tcounter:%x\trA:%x\trB:%x\tD:%x\tV:%x\n", computer->counter, i,j,D,V);
				}
				computer->registers[i] = V;
				computer->counter += 6;
				break;
			default: /* weird directive! */
				if(DISASSEMBLER1){
					printf("%x\terr\n", computer->counter);
				}
				fprintf(stderr, INS);
				return 1;
				break;
		}

		i = 0;
		j = 0;
		V = 0;
		D = 0;
	}
	fprintf(stderr, ADR);
	return 1;
}

int the_actual_program(char * y86_file){


	/* Obtained from http:// stackoverflow.com/questions/252782/strdup-what-does-it-do-in-c */
	char * copy = strdup(y86_file); /* Make a copy just so that nothing happens to the actual file */
	char * tokens = strtok(copy, "\t\n");

	unsigned char * bytes;  /* Array representing instructions & values */
	computer * computer;

	int size; /* the size directive */
	char * directives;
	char * hex_address;
	char * the_values;


	int i;	/* Counters */
	int j;

	char * text_argument = malloc(3); /* Instructions for .text directive */
	int long_value; /* Value for .long directive */

	/* Obtained from http:// stackoverflow.com/questions/3889992/how-does-strtok-split-the-string-into-tokens-in-c */
	while (tokens){
		if(strcmp(tokens, ".size") == 0){
			tokens = strtok(NULL, "\t\n");
			size = hexa_to_deci(tokens);
		} 
		else{
			tokens = strtok(NULL, "\t\n");
		}
	}

	/*
	 * The variable tokens is actually still pointing after the size directive... 
	 * We need to start from beginning just in case there are any important directives before the size.
	 */
	free(copy);

	computer = malloc(sizeof *computer);
	computer->counter = -1;
	computer->size = size;

	bytes = malloc((size+1) * sizeof(unsigned char));

	/* Initializing all values in the bytes register to 0 */
	for(i = 0; i < size; i++){
		bytes[i] = 0;
	}

	/* Initializing all the values in the register to be 0 */
	for(i = 0; i < 9; i++){
		computer->registers[i] = 0;
	}

	copy = strdup(y86_file);
	tokens = strtok(copy, "\t\n");

	i = 0;

	while(tokens) {
		/* Skip .size directive since we've already gone through that. */
		if(strcmp(tokens, ".size") == 0){
			tokens = strtok(NULL, "\t\n"); 
			tokens = strtok(NULL, "\t\n");
		} else if(strcmp(tokens, ".string") == 0 || 
					strcmp(tokens, ".long") == 0 || 
					strcmp(tokens, ".bss") == 0 ||
					strcmp(tokens, ".byte") == 0 ||
					strcmp(tokens, ".text") == 0){ /*  all valid directives! */

				/* Copy the values to respective variables  */
				directives = strdup(tokens);
				tokens = strtok(NULL, "\n\t");
				/* printf("directives =%s\n", directives); */
				hex_address = strdup(tokens);
				tokens = strtok(NULL, "\n\t");
				/* printf("hex =%s\n", hex_address); */
				the_values = strdup(tokens);
				tokens = strtok(NULL, "\n\t");
				/* printf("value =%s\n",the_values); */

				i = hexa_to_deci(hex_address); /* All directives have a hex-address */
				if(strcmp(directives, ".string") == 0){/* Storing a string! */
					j = 1; /* Start with 1 because the the_values[0] is the quotation mark (") */
					while(j <= strlen(the_values)) {
						bytes[i] = (unsigned char)the_values[j];
						j++;
						i++;
					}
				} else if(strcmp(directives, ".long") == 0){/* Storing a value in a byte which is a long! */
					long_value = atoi(the_values);
					*(unsigned int*)(bytes+i) = long_value;
				} else if(strcmp(directives, ".bss") == 0){ /* Specifies a chunk of UNITIALIZED stuff */
					j = atoi(the_values);
					while(j > 0){
						bytes[i] = (unsigned char)0;
						i++;
						j--;
					}
				
				} else if(strcmp(directives, ".byte") == 0){ /* Storing values! */
					int deci = hexa_to_deci(the_values);
					bytes[i] = ((unsigned char)deci);
				} else if(strcmp(directives, ".text") == 0){ /* Directives including instructions */
					j = 0;
					if(computer->counter == -1){ /* Should be the default value, since this directive shouldn't happen anywhere else */
						computer->counter = i;
					} else if(computer->counter != -1) {
						printf("It should never get here, but okay. (problem w/ counter)\n");
						fprintf(stderr, INS);
						return 1;
					}

					while (j < strlen(the_values)) { /* All instructions, which comes in pairs! */
						text_argument = substr(the_values, text_argument, j, 2);
						int deci = hexa_to_deci(text_argument);
						bytes[i] = deci;
						i++;
						j += 2;
					}

				}

				free(directives); /* Free everything since we're done! */
				free(hex_address);
				free(the_values);
				hex_address = 0;
		} else {
			fprintf(stderr, INS); /* Weird directive! */
			return 1;
		}
	}
	free(copy);
	free(tokens);
	/*for(i = 0; i < 0x18; i++){
		printf("%i\t%x\n", i , bytes[i]);
	}*/
	return the_actual_actual_program(bytes, computer);
	
}

int main(int argc, const char * argv[]){
	if(argc != 2){
		fprintf(stderr, "%s\n", "Incorrect amount of arguments!");
		return 0;
	}
	/*if -h is found!*/
	if(strcmp(argv[1], "-h") == 0){
		fprintf(stderr, "%s\n", "This is how you do this: ./y86emul <y86 instruction file>. Example: ./y86emul prog1.y86");
		return 0;
	}
	char * y86_file = y86_in_string(argv[1]);
	return the_actual_program(y86_file);
}
