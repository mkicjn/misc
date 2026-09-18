//`which tcc` $CFLAGS -run $0 "$@"; exit $?
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int kipisi_nimi(char *nimi, int pini)
{
	// https://sona.pona.la/wiki/Phonotactics
	int namako = 0;
	// Onset (open nimi)
	if (pini > 0 && (nimi[pini-1] == 'm' || nimi[pini-1] == 'n')) {
		// No adjacent nasals (kalama nene li ken ala lon poka kalama nene)
		nimi[pini + namako++] = "jklpstw"[rand() % 7];
	} else if (pini == 0 && rand() % 14 < 5) {
		// Null onset is word initial only (open nimi li ken jo e kalama open)
	} else {
		nimi[pini + namako++] = "jklmnpstw"[rand() % 9];
	}
	// Vowel (kalama open)
	if (pini + namako > 0 && nimi[pini + namako - 1] == 'w') {
		// Prohibit wu/wo (nimi li ala ken jo e wu anu wo)
		nimi[pini + namako++] = "aei"[rand() % 3];
	} else if (pini + namako > 0 && (nimi[pini + namako - 1] == 'j' || nimi[pini + namako - 1] == 't')) {
		// Prohibit ji/ti (nimi li ala ken jo e ji anu ti)
		nimi[pini + namako++] = "aeou"[rand() % 4];
	} else {
		nimi[pini + namako++] = "aeiou"[rand() % 5];
	}
	// Coda (pini)
	if (rand() % 3 == 0) {
		nimi[pini + namako++] = 'n';
	}
	return namako;
}

int nimi_sin(char *nimi, int nanpa_kipisi_nimi)
{
	int pini = 0;
	for (int nanpa = 0; nanpa < nanpa_kipisi_nimi; nanpa++)
		pini += kipisi_nimi(nimi, pini);
	nimi[pini] = '\0';
	return pini;
}

int main()
{
	srand(time(NULL));
	char nimi[80];
	int nanpa_kipisi_nimi = 1 + rand() % 4;
	nimi_sin(nimi, nanpa_kipisi_nimi);
	nimi[0] += 'A' - 'a';
	printf("%s\n", nimi);
	return 0;
}
