#include<stdio.h>
#include <time.h>

#define cls fputs("\033[H\033[J",stdout) // sets cursor to 0,0 and clears screen from curser to the end of the screen
#define null 0


int main() {
	
	int input; // variable for user input;
	int *password_ptr = null; // we will save a pointer to the password here 

	// password settings
	unsigned short passwordLength;


	// Initialize standart values

	{	//	passwordLength
		time_t random;
		time(&random);

		struct tm *ptm;
		ptm = gmtime(&random);

		passwordLength = 10 + ptm->tm_sec;
	}




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
			generate();

			if (reset() == EOF) return -2;	// idk how to fix the potential error from puts, so just close the progamm.
			break;

		case 'S':
			settings();

			if (reset() == EOF) return -2;	// idk how to fix the potential error from puts, so just close the progamm.
			break;
		default:
			break;
		}


	} 
}

int generate() {
	return 0;
}

int settings() {
	return 0;
}

int reset() {
	cls;
	return puts("Menue\n----------\nPassword [S]ettings\n[G]enerate Password\n[E]xit Progamm\n");
}