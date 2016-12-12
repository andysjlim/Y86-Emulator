#define AOK "everything is fine, no detected errors during execution \n"
#define HLT "halt instruction has been encountered!! Program ended normally \n"
#define INS "Invalid instruction. Program stopped \n"
#define ADR "Invalid addresss has been encountered! Program stopped \n"

#define EAX 0
#define ECX 1
#define EDX 2
#define EBX 3
#define ESP 4
#define EBP 5
#define ESI 6
#define EDI 7


char * y86_in_string();
typedef struct computer{
	int registers[9];

	int counter;
	int size;


	bool HLTfound;
	bool OF;
	bool ZF;
	bool SF;
} computer;

