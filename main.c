#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#define cls fputs("\033[H\033[J",stdout) // sets cursor to 0,0 and clears screen from curser to the end of the screen
#define null 0
#define backspace 0x8

int generate(int passwordLength, time_t random);
int settings(int *passwordLength_ptr);
int reset();
int reset_settings();
int password_options(char* password_ptr);
int reset_password_options(char* password_ptr);

int main() {
	
	int input; // variable for user input;

	// password settings
	unsigned short passwordLength;


	// Initialize standart values

	time_t random;
	time(&random);	// get seconds since 00:00, Jan 1 1970 UTC

	passwordLength = 10 + (random % 60); // get current second and add 10 (10 to 69)
	



	reset();	// cls and draw the menue

	while (1) {

		input = _getch();
		if (input == EOF) return -1; // there was an error while inputting, so lets just exit the progamm
		if (input >= 'a' && input <= 'z') input -= 0x20; // make it uppercase


		switch (input)
		{
		case 'E':
			return 0;

		case 'G':
			generate(passwordLength,random);

			if (reset() == EOF) return -2;	// idk how to fix the potential error from puts, so just close the progamm.
			break;

		case 'S':;
			int ret = settings(&passwordLength);	// get to the change setting menue
			if (ret != 0) return ret;	 // if we got a error, return the error

			if (reset() == EOF) return -2;	// idk how to fix the potential error from puts, so just close the progamm.
			break;
		default:
			break;
		}


	} 
}

int generate(int passwordLength, time_t random) {
	char *password_ptr = null; // we will save a pointer to the password here 

	password_ptr = malloc(sizeof(char)*(passwordLength + 1));

	if (password_ptr == null) {
		puts("out of memory!");
		return -3;
	}

	char utf8_lenghts_data[4] =		  { 0b00000000,0b11000000,0b11100000,0b11110000 }; //data for the first byte to show the length of the char
	char utf8_lenghts_data_maskI[4] = { 0b01111111,0b00011111,0b00001111,0b00000111 }; // mask of the data that shows the lenght of th char in the first byte. (Inverted)

#define utf8_following	      0b10000000
#define utf8_following_maskI  0b00111111


	srand(random);

	int bytes = 0;

	for (int i = 0; i < passwordLength; i++) {

		if (bytes == 0) {
			int freeBytes = passwordLength - i;
			if (freeBytes > 4) freeBytes = 4; // we use this for UTF8 lenghts, and max is 4

			bytes = rand() % freeBytes + 1;

			password_ptr[i] = utf8_lenghts_data[bytes] + (((char)rand()) & utf8_lenghts_data_maskI[bytes]);

			if (password_ptr[i] < 0x20) // if we are in the range of controll charatcers
				password_ptr[i] += 0x20;	// dont let them be in the password.
			
			bytes--;
		}
		else {
			password_ptr[i] = utf8_following + (((char)rand()) & utf8_following_maskI);

			bytes--;
		}
	}

	password_ptr[passwordLength + 1] = '\0';

	fputs("password: ", stdout);
	puts(password_ptr);

	int ret = password_options(password_ptr);
	if (ret != 0) return ret;

	free(password_ptr);
	return 0;
}

int settings(int *passwordLength_ptr) {

	int input;
	reset_settings();


	while (1) {

		input = _getch();
		if (input == EOF) return -1; // there was an error while inputting, so lets just exit the progamm
		if (input >= 'a' && input <= 'z') input -= 0x20; // make it uppercase



		switch (input)
		{
		case 'D':
			return 0;

		case 'L':
			
			puts("Password lenght: ");

			// for number input
			int num = 0;
			int chars[2];
			int i = 0;

			chars[0] = 0;
			chars[1] = 0;

			while (input != '\n' && input != '\r') {
				input = _getch();
				if (input == EOF) return -1; // there was an error while inputting, so lets just exit the progamm

				if (input == backspace && !i <= 0) {
					chars[i] = 0;	// remove the last number
					i--;

					// deleate one char from the console
					fputc(backspace, stdout);
					fputc(' ', stdout);
					fputc(backspace, stdout);
				}

				if (input >= '0' && input <= '9') {
					if (i >= 2) continue; // only alow 2 numbers to be inputed

					input -= '0'; // make it a number

					chars[i] = input;
					i++;

					fputc(input + '0', stdout);
				}
			}
			
			i--; // convert from [next free char] to [current char]
			for (int offset = 1; i >= 0; i--, offset *= 10) {
				num += chars[i] * offset;
			}
			*passwordLength_ptr = num;

			if (reset_settings() == EOF) return -2;	// idk how to fix the potential error from puts, so just close the progamm.
			break;
		default:
			break;
		}


	}

	return 0;
}

int reset() {
	cls;
	return puts("Menue\n----------\nPassword [S]ettings\n[G]enerate Password\n[E]xit Progamm\n");
}

int reset_settings() {
	cls;
	return puts("Menue\n----------\nPassword [S]ettings\n[G]enerate Password\n[E]xit Progamm\n\n----------\n\nset Password [L]enght\n[D]one\n");
}

int password_options(char* password_ptr) {

	int input;

	if (reset_password_options(password_ptr) == EOF) return -2;	// idk how to fix the potential error from puts, so just close the progamm.

	while (1) {

		input = _getch();
		if (input == EOF) return -1; // there was an error while inputting, so lets just exit the progamm
		if (input >= 'a' && input <= 'z') input -= 0x20; // make it uppercase


		switch (input)
		{
		case 'D':
			return 0;

		case 'S':

			if (reset_password_options(password_ptr) == EOF) return -2;	// idk how to fix the potential error from puts, so just close the progamm.
			break;
		case 'C':;

			if (reset_password_options(password_ptr) == EOF) return -2;	// idk how to fix the potential error from puts, so just close the progamm.
			break;
		default:
			break;
		}


	}

	return 0;
}

int reset_password_options(char* password_ptr) {
	cls;
	fputs("Menue\n----------\nPassword [S]ettings\n[G]enerate Password\n[E]xit Progamm\n\nPassword: ",stdout);
	puts(password_ptr);
	puts("\n----------\n[S]ave password in file\n[C]opy password to clipboard\n[D]one\n");
	return 0;
}