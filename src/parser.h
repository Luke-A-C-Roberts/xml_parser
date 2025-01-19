#ifndef PARSER_H
#define PARSER_H

#include <stddef.h>
#include "tokeniser.h"

/* Parsing */
typedef enum {ROOT, BRANCH, LEAF} NodeLabel;

typedef struct Node Node;
struct Node {
	NodeLabel label;
	char *content; // only a reference to a string stored in TokenList (Node isn't responsible for freeing it).
	Node **sub; // dynamic array
	size_t sub_sz;
	size_t sub_cap;
};

typedef struct NodeItem NodeItem;
struct NodeItem {
	Node *node;
	NodeItem *next;
};

typedef struct {NodeItem *start; size_t size;} NodeList;

extern void push_nodelist(NodeList *restrict const list, Node const*restrict const node);
extern void free_nodelist(NodeList const list);
extern Node *parse(TokenList tokenlist);
extern void free_nodetree(Node *const node);

#endif //PARSER_H
