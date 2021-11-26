#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define RRAND(min, max) ((min)+rand()%((max)-(min)+1))
#define CHOOSE(x) ((x)[rand()%sizeof(x)])
int apply(int a, char op, int b)
{
	switch (op) {
	case '+':
		return a + b;
	case '-':
		return a - b;
	case '*':
		return a * b;
	}
	return 0;
}
int main(int argc, char **argv)
{
	srand(time(NULL));
	char ops[] = {'*'};
	for (;;) {
		char op = CHOOSE(ops);
		int a = RRAND(0, 20), b = RRAND(0, 20), ans;
		printf("%d %c %d = ", a, op, b);
		for (;;) {
			scanf("%d", &ans);
			if (ans == apply(a, op, b))
				break;
			printf("\033[31mIncorrect.\033[m\n");
			printf("%d %c %d = ", a, op, b);
		}
		printf("\033[32mCorrect!\033[m\n");
	}
	return 0;
}
