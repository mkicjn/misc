#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include "/home/mark/Projects/test/aterm.h"

/*
|---|
|   |
|   0
| --|--
|   |
| _/ \_
|
--------
*/

void clear_screen(void)
{
	printf(CLS CUP("0","0"));
}

void draw_pole(void)
{
	printf("\
|---|\n\
|   |\n\
|\n\
|\n\
|\n\
|\n\
|\n\
--------\n\
");
}

char word[1024];
void choose_word(void)
{
	printf("Type a word or phrase: ");
	fflush(stdout);
	fgets(word, sizeof(word), stdin);
}

bool hidden[80];
void write_blanks(void)
{
	for (int i = 0; word[i]; i++) {
		if (isalpha(word[i])) {
			putchar('_');
			hidden[i] = true;
		} else {
			putchar(word[i]);
			hidden[i] = false;
		}
	}
}

char guess;
void take_guess(void)
{
	printf(CUP("11","1") EL("2"));
	guess = getchar();
}

bool guess_is_wrong(void)
{
	if (!isalpha(guess))
		return false;
	for (int i = 0; word[i]; i++)
		if (tolower(guess) == tolower(word[i]))
			return false;
	return true;
}

int wrong_guesses = 0;
void draw_next_part(void)
{
	wrong_guesses++;
	switch (wrong_guesses) {
	case 1: // Head
		printf(CUP("3","5") "0");
		break;
	case 2: // Body
		printf(CUP("4","5") "|");
		printf(CUP("5","5") "|");
		break;
	case 3: // Left Arm
		printf(CUP("4","3") "--");
		break;
	case 4: // Right Arm
		printf(CUP("4","6") "--");
		break;
	case 5: // Left Leg
		printf(CUP("6","3") "_/");
		break;
	case 6: // Right Leg
		printf(CUP("6","6") "\\_");
		break;
	default:
		break;
	}
}

void show_guess(void)
{
	if (!isalpha(guess))
		return;
	printf(CUP("10","%d"), tolower(guess)-'a'+1);
	putchar(toupper(guess));
}

void fill_correct_letters(void)
{
	for (int i = 0; word[i]; i++) {
		if (tolower(guess) == tolower(word[i])) {
			printf(CUP("9","%d"), i+1);
			putchar(word[i]);
			hidden[i] = false;
		}
	}
}

bool guesser_wins(void)
{
	for (int i = 0; word[i]; i++) {
		if (hidden[i])
			return false;
	}
	return true;
}

bool hangman_wins(void)
{
	return wrong_guesses >= 6;
}

bool game_over(void)
{
	return guesser_wins() || hangman_wins();
}

void print_win_message(void)
{
	printf(CUP("11","1"));
	if (guesser_wins()) {
		printf("You win! Good for you.\n");
	} else {
		printf("You lose! GAME OVER.\n");
		printf("The word was: %s", word);
	}
}

int main()
{
	choose_word();
	clear_screen();
	draw_pole();
	write_blanks();

	while (!game_over()) {
		take_guess();
		show_guess();
		if (guess_is_wrong()) {
			draw_next_part();
		} else {
			fill_correct_letters();
		}
	}

	print_win_message();

	return 0;
}
