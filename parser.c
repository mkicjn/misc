//$(which tcc) $CFLAGS -run $0 "$@"; exit $?
#define NO_LEX_MAIN
#include "lexer.c"

// Memory allocation
char zone_mem[(1 << 20)];
size_t zone_next = 0;

void *zalloc(size_t n)
{
	zone_next += n;
	return &zone_mem[zone_next - n];
}

// AST nodes, printing, and creation
struct ast_node {
	enum tok_type type;
	void *data;
	size_t num_child;
	struct ast_node *child[];
};

void print_ast(struct ast_node *node)
{
	if (node == NULL) {
		printf("NULL");
		return;
	}
	if (node->num_child > 1)
		printf("(");
	printf("%s", tok_desc[node->type]);
	if (node->num_child > 1) {
		for (int i = 0; i < node->num_child; i++) {
			printf(" ");
			print_ast(node->child[i]);
		}
		printf(")");
	}
}

struct ast_node *node_create(enum tok_type type, void *data, size_t num_child)
{
	struct ast_node *node = zalloc(sizeof(*node) + num_child * sizeof(node));
	node->type = type;
	node->data = data;
	node->num_child = num_child;
	for (int i = 0; i < num_child; i++)
		node->child[i] = NULL;
	return node;
}

// Shortcuts for converting the current lexer token into a node
void advance(void)
{
	do {
		token_next();
	} while (token_type == TOK_SPACE || token_type == TOK_COMMENT);
}

struct ast_node *absorb(size_t num_child)
{
	struct ast_node *n = node_create(token_type, NULL, num_child);
	advance();
	return n;
}

struct ast_node *absorb_text(void)
{
	char *s = zalloc(token_len+1);
	memcpy(s, token, token_len);
	s[token_len] = '\0';

	struct ast_node *n = absorb(0);
	n->data = s;
	return n;
}


// TODO: Try out Pratt parsing (the code below is not an example of this yet, just messing around a bit for now)

struct ast_node *expr(void);

struct ast_node *prefix(void)
{
	struct ast_node *n = NULL;
	switch (token_type) {
	case TOK_NUMBER:
		return absorb_text();
	case TOK_PLUS:
		return prefix();
	case TOK_LPAREN:
		advance();
		return expr();
	case TOK_MINUS:
		n = absorb(1);
		n->child[0] = expr();
		return n;
	}
	return n;
}

struct ast_node *infix(struct ast_node *left)
{
	struct ast_node *n = NULL;
	switch (token_type) {
	case TOK_EOF:
		return left;
	case TOK_RPAREN:
		advance();
		return left;
	case TOK_PLUS:
	case TOK_MINUS:
	case TOK_STAR:
	case TOK_FSLASH:
	case TOK_CARET:
		n = absorb(2);
		n->child[0] = left;
		n->child[1] = prefix();
		return infix(n);
	}
	return n;
}

struct ast_node *expr(void)
{
	struct ast_node *l = prefix();
	struct ast_node *n = infix(l);
	return n;
}

#ifndef NO_PARSE_MAIN
int main()
{
	token_source = stdin;
	advance();
	for (;;) {
		zone_next = 0;
		struct ast_node *n = expr();
		print_ast(n);
		printf("\n");
		if (token_type == TOK_EOF)
			break;
	}
	return 0;
}
#endif
