#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "filter.h"

#ifdef _WIN32 // also set for 64 bit

#define cls system("cls"); // clear the screen

#include<conio.h>;

int getchFunc() {
	return _getch();
}

#else

#include <termios.h>
#include <unistd.h>

int getchFunc() {

	struct termios oldt, newt;
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);

	int ch = getchar();

	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

	return ch;	// better than nothing		
}

#define cls system("clear"); // clear the screen

#endif //  _win32


#define backspace 0x8

#define true !0
#define false 0
#define bool int

// return codes
#define ret_inputError -1
#define ret_outputError -2
#define ret_outOfMem -3
#define ret_ioError -4
#define ret_nullError -5

typedef struct unicodeChar {
	unsigned int length;
	unsigned char *bytes; // should be only 4 bytes!
} unicodeChar;

int generate(int passwordLength, int random, bool bulk);
int settings();
int reset();
int reset_settings();
int password_options(char* password_ptr);
int reset_password_options(char* password_ptr);
int get_number(unsigned int length, unsigned short *number);
int write_file(char* password_ptr, const char* fileMode, bool doNotWrite);
int generate_unicode_char(int free_bytes, unicodeChar *stu);

const char newline[] = "\r\n";


// password settings
unsigned short passwordLength;
bool checkUnicode = true;

int main() {
	
	int input; // variable for user input;

	// Initialize standart values

	time_t random;
	time(&random);	// get seconds since 00:00, Jan 1 1970 UTC
	unsigned int seed = random;

	passwordLength = 12 + (random % 60); // get current second and add 10 (10 to 69)
	

	// used for checking function retrurns
	int ret;


	// cls and draw the menue
	if (reset() == EOF) return ret_outputError;	// idk how to fix the potential error from puts, so just close the progamm.

	while (1) {

		input = getchFunc();
		if (input == EOF) return ret_inputError; // there was an error while inputting, so lets just exit the progamm
		if (input >= 'a' && input <= 'z') input -= 0x20; // make it uppercase


		switch (input)
		{
		case 'E':
			return 0;

		case 'G':;

			time(&random);	// get seconds since 00:00, Jan 1 1970 UTC
			seed = random + rand();

			ret = generate(passwordLength, seed,false);
			if (ret != 0) return ret;	 // if we got a error, return the error

			if (reset() == EOF) return ret_outputError;	// idk how to fix the potential error from puts, so just close the progamm.
			break;
		case 'B':;

			ret = puts("Amount of passwords:");
			
			unsigned short passwords;
			ret = get_number(2, &passwords);
			if (ret != 0) return ret;

			if (passwords == 0) { // abort if no passwords will be generated
				if (reset() == EOF) return ret_outputError;
				break;
			} 

			write_file(NULL, "w", true); // just create an emptry file (seems to always give an error)

			time(&random);	// get seconds since 00:00, Jan 1 1970 UTC

			for (unsigned short i = 0; i < passwords; i++) {
				seed = random + rand();

				ret = generate(passwordLength, seed, true);
				if (ret != 0) return ret;	 // if we got a error, return the error

				ret = write_file(newline, "a", false);	// append a new line
				if (ret != 0) return ret;	 // if we got a error, return the error

			}

			int ret = puts("\npasswords saved.\npress any key...");
			if (ret == EOF) return ret_inputError;
			getchFunc();

			if (reset() == EOF) return ret_outputError;	// idk how to fix the potential error from puts, so just close the progamm.
			break;

		case 'S':;
			ret = settings();	// get to the change setting menue
			if (ret != 0) return ret;	 // if we got a error, return the error

			if (reset() == EOF) return ret_outputError;	// idk how to fix the potential error from puts, so just close the progamm.
			break;
		default:
			break;
		}


	} 
}

const char defaultMenue[] = "Menu\n----------\nPassword [S]ettings\n[G]enerate password\n[B]ulk generate password\n[E]xit progamm\n";

int generate(int passwordLength, int random, bool bulk) {
	unsigned char *password_ptr = NULL; // we will save a pointer to the password here 

	password_ptr = malloc(sizeof(char)*(passwordLength + 1));

	if (password_ptr == NULL) {
		puts("out of memory!");
		return ret_outOfMem;
	}

	unsigned char utf8_lenghts_data[4] =		{ 0b00000000,0b11000000,0b11100000,0b11110000 }; //data for the first byte to show the length of the char
	unsigned char utf8_lenghts_data_maskI[4] =	{ 0b01111111,0b00011111,0b00001111,0b00000111 }; // mask of the data that shows the lenght of th char in the first byte. (Inverted)

#define utf8_following	      0b10000000
#define utf8_following_maskI  0b00111111

	// probably not nessesary, but its fun and it could actually help a bit in some weird cases.
	srand(random);
	unsigned int seed = passwordLength + rand();
	for (int i = 0, j = 1 + rand() % 3; i < j; i++) {
		seed += rand();
	}
	seed += (int)&seed;

	srand(random);

	int bytes = 0;

	for (int i = 0; i < passwordLength;) {
		
		/*if (bytes == 0) {
			int freeBytes = passwordLength - i;
			if (freeBytes > 4) freeBytes = 3; // we use this for UTF8 lenghts, and max is 4. but we only want to go up to 3, to avoid 'invalid' character

			bytes = rand() % freeBytes + 1; // from 1 to 3

			password_ptr[i] = utf8_lenghts_data[bytes-1] + (((unsigned char)rand()) & utf8_lenghts_data_maskI[bytes-1]);

			if (bytes == 4 && password_ptr[i] >= 0xe0) {	// characters after this might not make much sense.
				password_ptr[i] -= 0x20; // get it below 0xe0
				password_ptr[i] = utf8_lenghts_data[bytes - 1] + (password_ptr[i] & utf8_lenghts_data_maskI[bytes - 1]);	// make sure the formating is still right
			}

			if (password_ptr[i] < 0x20 || password_ptr[i] == 0x7f) // if we are in the range of controll charatcers
				password_ptr[i] += 0x20;	// dont let them be in the password.
			
			bytes--;
		}
		else {
			password_ptr[i] = utf8_following + (((unsigned char)rand()) & utf8_following_maskI);

			bytes--;
		}*/

		int freeBytes = passwordLength - i;

		char *uchar = malloc(4);
		if (uchar == NULL) {
			puts("out of memory!");
			return ret_outOfMem;
		}

		unicodeChar stu;
		stu.length = 0;
		stu.bytes = uchar;

		generate_unicode_char(freeBytes, &stu);	
		

		

		for (int j = 0; j < stu.length; j++) {
			password_ptr[i] = stu.bytes[j];
			i++;
		}
		free(uchar);
	}

	password_ptr[passwordLength] = '\0';

	if (bulk == true) {

		write_file(password_ptr, "a", false);

	}else {

		int ret = password_options(password_ptr);
		if (ret != 0) return ret;
	}

	free(password_ptr);
	return 0;
}

int settings() {

	int input;
	reset_settings();


	while (1) {

		input = getchFunc();
		if (input == EOF) return ret_inputError; // there was an error while inputting, so lets just exit the progamm
		if (input >= 'a' && input <= 'z') input -= 0x20; // make it uppercase



		switch (input)
		{
		case 'D':
			return 0;
		case 'U':

			if (checkUnicode == true)
				checkUnicode = false;
			else
				checkUnicode = true;
			if (reset_settings() == EOF) return ret_outputError;	// idk how to fix the potential error from puts, so just close the progamm.
			break;
		case 'L':;
			
			int ret = printf("Old Length: %d\nPassword lenght: ",passwordLength);
			if (ret < 0) return ret_outputError;

			ret = get_number(2,&passwordLength);
			if (ret != 0) return ret;

			if (passwordLength == 0) passwordLength = 1; // bugfix
			
			if (reset_settings() == EOF) return ret_outputError;	// idk how to fix the potential error from puts, so just close the progamm.
			break;
		default:
			break;
		}


	}

	return 0;
}

int reset() {
	cls;
	return puts(defaultMenue);
}

int reset_settings() {
	cls;
	int ret = puts(defaultMenue);
	if (ret == EOF) return ret;

	char boolOut[] = { 'T','r','u','e',' ','\0' };
	if (!checkUnicode)   memcpy(boolOut, &"False",6);

	ret = printf("\n----------\n\nset Password [L]enght (%d)\nToggle valid [U]nicode checking ( %s)\n[D]one\n",passwordLength,boolOut);
	if (ret < 0) return EOF;

	return 0;
}

int reset_password_options(char* password_ptr) {
	cls;
	int ret = puts(defaultMenue);
	if (ret == EOF) return ret;

	ret = fputs("\nPassword: ", stdout);
	if (ret == EOF) return ret;

	ret = puts(password_ptr);
	if (ret == EOF) return ret;

	return puts("\n----------\n[S]ave password in pasword.txt (overwrites)\n[D]one\n");
}


int write_file(char* password_ptr, const char* fileMode, bool doNotWrite) {
	const char *filename = "./password.txt";	// fixed file name, because implementing anything else takes time

	//
	//	we dont check if the file already exists, we dont check if we allowed to modify it, we just try to write the file, and if there is an error
	// then its the problem of the user.
	//

#pragma warning(suppress : 4996)
	FILE* file = fopen(filename, fileMode);

	if (file == NULL) {
		puts("file error!");
		return ret_ioError;
	}

	if (doNotWrite == false) {

		if (password_ptr == NULL) {
			puts("file write error!");
			return ret_nullError;
		}
		if (fputs(password_ptr, file) == EOF) {
			puts("file write error!");
			return ret_ioError;
		}

	}

	fclose(file);
}

int password_options(char* password_ptr) {

	if (password_ptr == NULL) return ret_nullError;

	int input;

	if (reset_password_options(password_ptr) == EOF) return ret_outputError;	// idk how to fix the potential error from puts, so just close the progamm.

	while (1) {


		input = getchFunc();
		if (input == EOF) return ret_inputError; // there was an error while inputting, so lets just exit the progamm
		if (input >= 'a' && input <= 'z') input -= 0x20; // make it uppercase
		

		switch (input)
		{
		case 'D':
			return 0;

		case 'S':;

			write_file(password_ptr, "w", false);

			int ret = puts("password saved.\npress any key...");
			if (ret == EOF) return ret_inputError;
			getchFunc();

			if (reset_password_options(password_ptr) == EOF) return ret_outputError;	// idk how to fix the potential error from puts, so just close the progamm.
			break;
		default:
			break;
		}


	}

	return 0;
}


int get_number(unsigned int length, unsigned short *number) {
	
	if (number == NULL) return ret_nullError;

	int input = 0;
	unsigned short num = 0;
	unsigned int *chars = (int*)malloc(sizeof(int)*length);

	for (unsigned int i = 0;i < length;i++)
		chars[i] = 0;



	int i = 0;

	while (input != '\n' && input != '\r') {
		input = getchFunc();
		if (input == EOF) return ret_inputError; // there was an error while inputting, so lets just exit the progamm

		if (input == backspace && i > 0) {
			i--;
			chars[i] = 0;	// remove the last number
			

			// deleate one char from the console
			fputc(backspace, stdout);
			fputc(' ', stdout);
			fputc(backspace, stdout);
		}

		if (input >= '0' && input <= '9') {
			if (i >= length) continue; // only alow length numbers to be inputed

			input -= '0'; // make it a number

			chars[i] = input;
			i++;

			fputc(input + '0', stdout);
		}
	}

	i--; // convert from [next free char] to [current char]
	for (long offset = 1; i >= 0; i--, offset *= 10) {
		num += chars[i] * offset;
	}
	*number = num;

	free(chars);
	return 0;
}

int generate_unicode_char(int free_bytes, unicodeChar *stu) { // generate unicode characters, until a valid character was found

	unsigned char utf8_lenghts_data[4] = { 0b00000000,0b11000000,0b11100000,0b11110000 }; //data for the first byte to show the length of the char
	unsigned char utf8_lenghts_data_maskI[4] = { 0b01111111,0b00011111,0b00001111,0b00000111 }; // mask of the data that shows the lenght of th char in the first byte. (Inverted)

#define utf8_following	      0b10000000
#define utf8_following_maskI  0b00111111

	do {
		int bytes = 0;
		unsigned char output[4] = { 0,0,0,0 };

		int i = 0;

		unsigned int unicodeLen = 0;

		do {
			if (bytes == 0) {
				if (free_bytes > 4) free_bytes = 3; // we use this for UTF8 lenghts, and max is 4. but we only want to go up to 3, to avoid 'invalid' character

				bytes = rand() % free_bytes + 1; // from 1 to 3
				unicodeLen = bytes;

				output[i] = utf8_lenghts_data[bytes - 1] + (((unsigned char)rand()) & utf8_lenghts_data_maskI[bytes - 1]);

				if (bytes == 4 && output[i] >= 0b11110000) {	// characters after this might not make much sense.
					output[i] -= 0b11110000; // set is to its max value
					output[i] = utf8_lenghts_data[bytes - 1] + (output[i] & utf8_lenghts_data_maskI[bytes - 1]);	// make sure the formating is still right
				}

				if (output[i] < 0x20 || output[i] == 0x7f) // if we are in the range of controll charatcers
					output[i] += 0x20;	// dont let them be in the password.

				bytes--;
			}
			else {
				output[i] = utf8_following + (((unsigned char)rand()) & utf8_following_maskI);

				bytes--;
			}

			i++;
		} while (bytes != 0);

		stu->length = unicodeLen;
		for (int i = 0; i < 4; i++)
			stu->bytes[i] = output[i];


		
		// utf8 to unicode
		int mask;
		switch (unicodeLen)
		{
		default:
		case 1:
			mask = 0x7f; break;
		case 2:
			mask = 0x1f; break;
		case 3:
			mask = 0x0F; break;
		case 4:
			mask = 0x07; break;
		}
		unsigned int rawUnicode = 0;


		
		rawUnicode = output[0] & mask;
		
		for (int i = 1; i < unicodeLen; ++i) {
			rawUnicode <<= 6;
			rawUnicode += (output[i] & 0b00111111);
		}

		// workaround, idk why it does not take it from the header file.

		enum FilterTag {
			FilterTag_Controll = 0b0000000000000001, //		           1		// like 0x0
			FilterTag_NewLines = 0b0000000000000010, //		          10		// like 0xa and 0xd aswell as 0xB
			FilterTag_Spaces = 0b0000000000000100, //		         100		// like 0x9 and 0x20
			FilterTag_Deletion = 0b0000000000001000, //		        1000		// like 0x8 and 0x7F
			FilterTag_PageBreak = 0b0000000000010000, //		      1 0000		// like 0xC
			FilterTag_Escape = 0b0000000000100000, //		     10 0000		// like 0x1B
			FilterTag_NonSpaceInvis = 0b0000000001000000, //		    100 0000		// Invisible (Non Space/Tab) Characters
			FilterTag_Unassigned = 0b0000000010000000, //		   1000 0000		// unassinged in unicode
			FilterTag_PossibleNL = 0b0000000100000000, //		 1 0000 0000		// 0xAD (its either invisible, or visible and a line break)
			FilterTag_Modify = 0b0000000100000000, //		10 0000 0000		// Modifies chars or text in a way. example: 0x34F and 0x61C or even 0x83
			FilterTag_IDontKnow = 0b0000000100000000, //	   100 0000 0000		// 0x2061 - WHAT DOES IT DO???????
			FilterTag_Surrogate = 0b0000001000000000, //	  1000 0000 0000		// surrogates (0xd800-0xdFFFF)
		};

		unsigned int keep = FilterTag_Spaces |	FilterTag_NonSpaceInvis	|FilterTag_Modify;
		if (checkUnicode) keep |= FilterTag_Unassigned;

		if (filterUnicode(rawUnicode, 0xFFFFFFFF ^ keep)) { // ^ = bitwise xor
			return stu;
		}
	} while (true);
}