//`which tcc` $CFLAGS -run $0 "$@"; exit $?
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// https://github.com/mkicjn/splay
#include "../../splay/src/splay.c"

struct node {
	size_t count;
	struct splay_link word_link;
	struct splay_link count_link;
	char word[];
};

enum splay_dir word_cmp(const struct splay_link *al, const struct splay_link *bl)
{
	struct node *a = SPLAY_CONTAINER(al, struct node, word_link);
	struct node *b = SPLAY_CONTAINER(bl, struct node, word_link);

	int cmp = strcmp(b->word, a->word);
	if (cmp < 0)
		return SPLAY_LEFT;
	if (cmp > 0)
		return SPLAY_RIGHT;
	return SPLAY_HERE;
}

enum splay_dir count_cmp(const struct splay_link *al, const struct splay_link *bl)
{
	struct node *a = SPLAY_CONTAINER(al, struct node, count_link);
	struct node *b = SPLAY_CONTAINER(bl, struct node, count_link);

	if (b->count < a->count)
		return SPLAY_LEFT;
	if (b->count > a->count)
		return SPLAY_RIGHT;

	int cmp = strcmp(a->word, b->word);
	if (cmp < 0)
		return SPLAY_LEFT;
	if (cmp > 0)
		return SPLAY_RIGHT;
	return SPLAY_HERE;
}

#define MAX_WORD_LEN 100
struct node *getnode(FILE *f)
{
	char buf[MAX_WORD_LEN];
	size_t len = 0;

	// Skip invalid characters
	int c = fgetc(f);
	while (c >= 0 && !isalpha(c))
		c = fgetc(f);

	// EOF check
	if (c < 0)
		return NULL;

	// Store valid characters
	do {
		buf[len++] = tolower(c);
		c = fgetc(f);
	} while (c > 0 && isalpha(c) && len < MAX_WORD_LEN);

	// Clone string into new node
	struct node *n = malloc(sizeof(*n) + len + 1);
	n->count = 0;
	memcpy(n->word, buf, len);
	n->word[len] = '\0';

	return n;
}

void show_counts_and_free(struct splay_link *l)
{
	if (l == NULL)
		return;

	struct node *n = SPLAY_CONTAINER(l, struct node, count_link);
	show_counts_and_free(l->child[SPLAY_LEFT]);
	printf("%zu\t%s\n", n->count, n->word);
	show_counts_and_free(l->child[SPLAY_RIGHT]);
	free(n);
}

int main(int argc, char **argv)
{
	struct splay_link *word_root = NULL;
	struct splay_link *count_root = NULL;

	FILE *f = stdin;
	if (argc > 1)
		f = fopen(argv[1], "r");
	if (f == NULL)
		return 1;

	struct node *n = getnode(f);
	while (n != NULL) {
		// Insert/update in word splay
		struct splay_link *conflict = splay_insert(&word_root, word_cmp, &n->word_link);
		if (conflict != NULL) {
			free(n);
			n = SPLAY_CONTAINER(conflict, struct node, word_link);
		}
		// Remove and re-add to count splay
		(void)splay_delete(&count_root, count_cmp, &n->count_link);
		n->count++;
		splay_insert(&count_root, count_cmp, &n->count_link);
		n = getnode(f);
	}

	show_counts_and_free(count_root);
	return 0;
}
