#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#ifdef _WIN32

#define cls system("cls"); // clear the screen

#include<conio.h>;

int getch() {
	return _getch();
}

#else

#include <termios.h>
#include <unistd.h>

int getch() {

	struct termios oldt, newt;
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	mewt.c_lflag &= ~ICANON | ECHO);
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

int generate(int passwordLength, int random, bool bulk);
int settings(unsigned short *passwordLength_ptr);
int reset();
int reset_settings();
int password_options(char* password_ptr);
int reset_password_options(char* password_ptr);
int getNumber(unsigned int length, unsigned short *number);
int writeFile(char* password_ptr, const char* fileMode, bool doNotWrite);

const char newline[] = "\r\n";

int main() {
	
	int input; // variable for user input;

	// password settings
	unsigned short passwordLength;


	// Initialize standart values

	time_t random;
	time(&random);	// get seconds since 00:00, Jan 1 1970 UTC
	unsigned int seed = random;

	passwordLength = 10 + (random % 60); // get current second and add 10 (10 to 69)
	

	// used for checking function retrurns
	int ret;


	reset();	// cls and draw the menue

	while (1) {

		input = getch();
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
			ret = getNumber(2, &passwords);
			if (ret != 0) return ret;

			writeFile(NULL, "w", true); // just create an emptry file

			time(&random);	// get seconds since 00:00, Jan 1 1970 UTC

			for (unsigned short i = 0; i < passwords; i++) {
				seed = random + rand();

				ret = generate(passwordLength, seed, true);
				if (ret != 0) return ret;	 // if we got a error, return the error

				ret = writeFile(newline, "a", false);	// append a new line
				if (ret != 0) return ret;	 // if we got a error, return the error

			}

			int ret = puts("\npasswords saved.\npress any key...");
			if (ret == EOF) return ret_inputError;
			getch();

			if (reset() == EOF) return ret_outputError;	// idk how to fix the potential error from puts, so just close the progamm.
			break;

		case 'S':;
			ret = settings(&passwordLength);	// get to the change setting menue
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

	unsigned char utf8_lenghts_data[4] =		  { 0b00000000,0b11000000,0b11100000,0b11110000 }; //data for the first byte to show the length of the char
	unsigned char utf8_lenghts_data_maskI[4] = { 0b01111111,0b00011111,0b00001111,0b00000111 }; // mask of the data that shows the lenght of th char in the first byte. (Inverted)

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

	for (int i = 0; i < passwordLength; i++) {

		if (bytes == 0) {
			int freeBytes = passwordLength - i;
			if (freeBytes > 4) freeBytes = 3; // we use this for UTF8 lenghts, and max is 4. but we only want to go up to 3, to avoid 'invalid' character

			bytes = rand() % freeBytes + 1; // from 1 to 3

			password_ptr[i] = utf8_lenghts_data[bytes-1] + (((unsigned char)rand()) & utf8_lenghts_data_maskI[bytes-1]);

			if (bytes == 4 && password_ptr[i] >= 0xe0) {	// characters after this might not make much sense.
				password_ptr[i] -= 0x20; // get it below 0xe0
				password_ptr[i] = utf8_lenghts_data[bytes - 1] + (password_ptr[i] & utf8_lenghts_data_maskI[bytes - 1]);	// make sure the formating is still right
			}

			if (password_ptr[i] < 0x20) // if we are in the range of controll charatcers
				password_ptr[i] += 0x20;	// dont let them be in the password.
			
			bytes--;
		}
		else {
			password_ptr[i] = utf8_following + (((unsigned char)rand()) & utf8_following_maskI);

			bytes--;
		}
	}

	password_ptr[passwordLength] = '\0';

	if (bulk == true) {

		writeFile(password_ptr, "a", false);

	}else {

		int ret = password_options(password_ptr);
		if (ret != 0) return ret;
	}

	free(password_ptr);
	return 0;
}

int settings(unsigned short *passwordLength_ptr) {

	if (passwordLength_ptr == NULL) return ret_nullError;

	int input;
	reset_settings();


	while (1) {

		input = getch();
		if (input == EOF) return ret_inputError; // there was an error while inputting, so lets just exit the progamm
		if (input >= 'a' && input <= 'z') input -= 0x20; // make it uppercase



		switch (input)
		{
		case 'D':
			return 0;

		case 'L':;
			
			int ret = puts("Password lenght: ");
			if (ret != 0) return ret;

			ret = getNumber(2,passwordLength_ptr);
			if (ret != 0) return ret;
			
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
	if (ret != 0) return ret;
	return puts("\n----------\n\nset Password [L]enght\n[D]one\n");
}

int reset_password_options(char* password_ptr) {
	cls;
	int ret = puts(defaultMenue);
	if (ret != 0) return ret;

	ret = fputs("\nPassword: ", stdout);
	if (ret != 0) return ret;

	ret = puts(password_ptr);
	if (ret != 0) return ret;

	return puts("\n----------\n[S]ave password in pasword.txt (overwrites)\n[D]one\n");
}


int writeFile(char* password_ptr, const char* fileMode, bool doNotWrite) {
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

		if (password_ptr == NULL) return ret_nullError;

		if (fputs(password_ptr, file) != 0) {
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


		input = getch();
		if (input == EOF) return ret_inputError; // there was an error while inputting, so lets just exit the progamm
		if (input >= 'a' && input <= 'z') input -= 0x20; // make it uppercase
		

		switch (input)
		{
		case 'D':
			return 0;

		case 'S':;

			writeFile(password_ptr, "w", false);

			int ret = puts("password saved.\npress any key...");
			if (ret == EOF) return ret_inputError;
			getch();

			if (reset_password_options(password_ptr) == EOF) return ret_outputError;	// idk how to fix the potential error from puts, so just close the progamm.
			break;
		default:
			break;
		}


	}

	return 0;
}


int getNumber(unsigned int length, unsigned short *number) {
	
	if (number == NULL) return ret_nullError;

	int input = 0;
	unsigned short num = 0;
	unsigned int *chars = (int*)malloc(sizeof(int)*length);

	for (int i = 0;i < length;i++)
		chars[i] = 0;



	int i = 0;

	while (input != '\n' && input != '\r') {
		input = getch();
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