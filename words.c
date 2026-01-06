//$(which tcc) $CFLAGS -run $0 "$@"; exit $?
//
// Faster wordlist generation from large texts
// (WIP? Might want to add stats/sorting later)
//

#include <stdio.h>
#include <string.h>

// FIXME: Reference this code in a smarter way
#include "/home/mark/Projects/splay/src/splay.h"
#include "/home/mark/Projects/splay/src/splay.c"

#define MAX_WORD_LEN 64
#define MAX_WORDS (size_t)10e6
#define MAX_LINE_LEN 200

struct word_node {
	char word[MAX_WORD_LEN];
	struct splay_link link;
};

enum splay_dir word_nav(const struct splay_link *arg, const struct splay_link *tree_node)
{
	struct word_node *a = SPLAY_CONTAINER(arg, struct word_node, link);
	struct word_node *b = SPLAY_CONTAINER(tree_node, struct word_node, link);

	int cmp = strcmp(a->word, b->word);
	if (cmp < 0)
		return SPLAY_LEFT;
	else if (cmp > 0)
		return SPLAY_RIGHT;
	else
		return SPLAY_HERE;
}

struct word_node node_buf[MAX_WORDS];
struct word_node *node_buf_next = node_buf;

void show_words(struct splay_link *link)
{
	if (link == NULL)
		return;
	struct word_node *node = SPLAY_CONTAINER(link, struct word_node, link);

	show_words(link->child[SPLAY_LEFT]);
	printf("%s\n", node->word);
	show_words(link->child[SPLAY_RIGHT]);
}

int main(int argc, char **argv)
{
	struct splay_link *root = NULL;
	char line_buf[MAX_LINE_LEN];
	while (fgets(line_buf, sizeof(line_buf), stdin)) {
		const char *delim = ",.;:()[]{}<>\"+-\'.!? \n";
		const char *word = strtok(line_buf, delim);
		while (word) {
			struct word_node *n = (node_buf_next++);
			strncpy(n->word, word, sizeof(n->word));
			if (splay_insert(&root, word_nav, &n->link))
				node_buf_next--;
			word = strtok(NULL, delim);
		}
	}
	show_words(root);
}
