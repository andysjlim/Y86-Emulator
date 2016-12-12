#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "hex_conversion.h"

/*
int hexa_to_deci(char * hexN){
	int decimal = 0;
	int size = strlen(hexN);
	int i;
	for(i = size-1; i >=0; i--){
		if(isdigit(hexN[i])){
			decimal += (hexN[i] - '0') * pow(16, (size-i-1));
		}
		else{
			switch(hexN[i]){
				case 'A':
				case 'a':{
					decimal += 10 * pow(16, (size-i-1));
					break;
				}
				case 'B':
				case 'b':{
					decimal += 11 * pow(16, (size-i-1));
					break;
				}
				case 'C':
				case 'c':{
					decimal += 12 * pow(16, (size-i-1));
					break;
				}
				case 'D':
				case 'd':{
					decimal += 13 * pow(16, (size-i-1));
					break;
				}
				case 'E':
				case 'e':{
					decimal += 14 * pow(16, (size-i-1));
					break;
				}
				case 'F':
				case 'f':{
					decimal += 15 * pow(16, (size-i-1));
					break;
				}
				default:{
					break;
				}
			}
		}
	}
	printf("%i\t", decimal);
	unsigned char x = (unsigned char)decimal;
	printf("%x", x);
	return decimal;
}
*/

char * single_digit_hex_to_bin(char hex_dig){
	switch(hex_dig){
		case '0':
			return "0000";
			break;
		case '1':
			return "0001";
			break;
		case '2':
			return "0010";
			break;
		case '3':
			return "0011";
			break;
		case '4':
			return "0100";
			break;
		case '5':
			return "0101";
			break;
		case '6':
			return "0110";
			break;
		case '7':
			return "0111";
			break;
		case '8':
			return "1000";
			break;
		case '9':
			return "1001";
			break;
		case 'A':
		case 'a':
			return "1010";
			break;
		case 'B':
		case 'b':
			return "1011";
			break;
		case 'C':
		case 'c':
			return "1100";
			break;
		case 'D':
		case 'd':
			return "1101";
			break;
		case 'E':
		case 'e':
			return "1110";
			break;
		case 'F':
		case 'f':
			return "1111";
			break;
		default:
			fprintf(stderr, "weird");
			return "";
			break;
	}
}

char * hexa_to_bin(char * hex){
	char * bin = malloc(strlen(hex) * 4+1);
	int i = 0;
	bin[strlen(hex)*4] = '\0';

	for(i = 0; i < strlen(hex); i++){
		bin = strcat(bin, single_digit_hex_to_bin(hex[i]));
	}
	/*printf("%s\n", bin);*/
	return bin;
}

int bin_to_dec(char * bin){
	int i = 0;
	int power = (int)strlen(bin) - 1;
	int value = 0;

	for(i = 0; i < (int)strlen(bin); i++){
		if(bin[i] == '1'){
			value += (int)pow(2, power);
		}

		power--;
	}

	/*printf("%i", value);*/
	return value;
}

int hexa_to_deci(char * hex){
	char * bin = hexa_to_bin(hex);
	return bin_to_dec(bin);
}
/*
int main(int argc, char*argv[]){
	return hexa_to_deci(argv[1]);
}*/
