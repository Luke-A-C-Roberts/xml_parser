#include <errno.h>
#include <regex.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

/* error handlers */
static _Noreturn int handle_regcomp_err(int const errcode, regex_t const *preg) {
	static const size_t errbuf_sz = 64;
	char errbuf[errbuf_sz];
	regerror(errcode, preg, errbuf, errbuf_sz);
	fprintf(stderr, "%s\n", errbuf);
	exit(EXIT_FAILURE);
}

static _Noreturn int handle_malloc_err(char const*const varname) {
	int const errsv = errno;
	(void)fprintf(stderr, "%s malloc retured nullptr:\n", varname);
	(void)fprintf(stderr, "%s\n", strerror(errsv));
	exit(EXIT_FAILURE);
}

static _Noreturn int handle_realloc_err(char const*const varname) {
	int const errsv = errno;
	(void)fprintf(stderr, "%s realloc retured nullptr:\n", varname);
	(void)fprintf(stderr, "%s\n", strerror(errsv));
	exit(EXIT_FAILURE);
}

static _Noreturn int handle_fopen_err(char const*const filename) {
	(void)fprintf(stderr, "File \"%s\" could not be found\n", filename);
	exit(EXIT_FAILURE);
}

static _Noreturn int handle_fread_error(void) {
	(void)fputs("Not all file data could be read\n", stderr);
	exit(EXIT_FAILURE);
}

static _Noreturn int handle_fclose_error(char const*const filevarname) {
	int const errsv = errno;
	(void)fprintf(stderr, "fclose(%s) failed:\n", filevarname);
	(void)fprintf(stderr, "%s\n", strerror(errsv));
	exit(EXIT_FAILURE);
}

static _Noreturn int handle_leaf_node_append(void) {
	(void)fputs("Tried to append a node to a leaf nodes' subnodes\n", stderr);
	exit(EXIT_FAILURE);
}

static _Noreturn int handle_mismatch_tag_error(char const*restrict const tagname1, char const*restrict const tagname2) {
	(void)fprintf(stderr, "start tag \"%s\" mismatched with end tag \"%s\"\n", tagname1, tagname2);
	exit(EXIT_FAILURE);
}

/* I/O */
static char *copy_file_data(char const*const filename) {
	FILE *const file = fopen(filename, "r");
	if (file == (void *)0) handle_fopen_err(filename);

	size_t file_sz = 0UL;
	for (int c = fgetc(file); c != EOF; c = fgetc(file)) file_sz++;

	rewind(file);

	char *const file_data = malloc(file_sz + 1);
	if (file_data == (void *)0) handle_malloc_err("file_data");

	size_t fread_count = fread(file_data, sizeof(char), file_sz, file);
	if (fread_count != file_sz) handle_fread_error();
	file_data[file_sz] = '\0';

	if (fclose(file) == EOF) handle_fclose_error("file");

	return file_data;
}

/* Tokenisation */
typedef enum {START, END} TagType;
typedef struct {TagType type; char *name;} TagTok;
typedef struct {char *name;} StringTok;
typedef union {TagTok tok; StringTok string;} TokenData;
typedef enum {STRING, TAG} TokenLabel;
typedef struct {TokenLabel label; TokenData data;} Token;

typedef struct TokenItem TokenItem;
struct TokenItem {Token token; TokenItem *next;};
typedef struct {TokenItem *start; size_t size;} TokenList;

static void regcomp_handled(regex_t *restrict const regexp, char const*restrict const pattern) {
	int regerrcode = regcomp(regexp, pattern, REG_EXTENDED);
	if (regerrcode) handle_regcomp_err(regerrcode, regexp);
}

static char *copy_regmatch_data(char *const text_at_pos, regmatch_t const regmatch, size_t const start_offset, size_t const end_offset) {
	ptrdiff_t const regmatch_sz = (regmatch.rm_eo - end_offset) - (regmatch.rm_so + start_offset);
	char *tagstr = malloc(regmatch_sz + 1);
	tagstr[regmatch_sz] = '\0';
	strncpy(tagstr, &text_at_pos[regmatch.rm_so + start_offset], regmatch_sz);
	return tagstr;
}

static bool next_token(char const*restrict const text, Token *restrict const out) {
	static char  *_text;
	static size_t _text_sz;
	static size_t _text_pos;
	static regex_t start_tag_regexp;
	static regex_t end_tag_regexp;
	static regex_t string_regexp;

	// if text is not nullptr, copy it into the static variable
	if (text != (void *)0) {
		_text_sz = strlen(text);
		_text = malloc(_text_sz + 1);
		strcpy(_text, text);
		_text_pos = 0UL;

		// compile regex expressions (regex_t vars stored globally)
		regcomp_handled(&string_regexp, "^[^<> \\n\\t]+");
		regcomp_handled(&start_tag_regexp, "^<[^\\/<>]+>");
		regcomp_handled(&end_tag_regexp, "^<\\/[^<>]+>");
	}

	// remove trailing whitespace characters
	char c = _text[_text_pos];
	while (c == ' ' || c == '\n' || c == '\t') {
		_text_pos++;
		if (_text_pos >= _text_sz) goto cleanup;
		c = _text[_text_pos];
	}

	// matching tags and content strings.
	char *const text_at_pos = &_text[_text_pos];
	regmatch_t regmatch = {0};

	if (regexec(&string_regexp, text_at_pos, 1, &regmatch, 0) != REG_NOMATCH) {
		char *tagstr = copy_regmatch_data(text_at_pos, regmatch, 0, 0);
		_text_pos += regmatch.rm_eo;
		*out = (Token) {.label = STRING, .data.string = (StringTok) {.name = tagstr}};
		return true;
	}

	if (regexec(&start_tag_regexp, text_at_pos, 1, &regmatch, 0) != REG_NOMATCH) {
		char *tagstr = copy_regmatch_data(text_at_pos, regmatch, 1, 1);
		_text_pos += regmatch.rm_eo;
		*out = (Token) {.label = TAG, .data.tok = (TagTok) {.type = START, .name = tagstr}};
		return true;
	}

	if (regexec(&end_tag_regexp, text_at_pos, 1, &regmatch, 0) != REG_NOMATCH) {
		char *tagstr = copy_regmatch_data(text_at_pos, regmatch, 2, 1);
		_text_pos += regmatch.rm_eo;
		*out = (Token) {.label = TAG, .data.tok = (TagTok) {.type = END, .name = tagstr}};
		return true;
	}
	
cleanup:
	// text memory freed
	free(_text);
	_text = (void *)0;
	
	// regex expression memory freed
	regfree(&start_tag_regexp);
	regfree(&end_tag_regexp);
	regfree(&string_regexp);

	return false;
}

static TokenList tokenise(char const*const xml_text) {
	TokenList token_list = {.start = (void *)0, .size = 0UL};
	// TokenItem *end_item = token_list.start; // i.e. null ptr
	Token next_tok = {0};

	if (next_token(xml_text, &next_tok)) {
		TokenItem *start_item = malloc(sizeof(TokenItem));
		if (start_item == (void *)0) handle_malloc_err("start_item");
		*start_item = (TokenItem) {.token = next_tok, .next = (void *)0};
		TokenItem *end_item = start_item;
		token_list.start = start_item;
		token_list.size++;

		while (next_token((void *)0, &next_tok)) {
			TokenItem *next_item = malloc(sizeof(TokenItem));
			if (next_item == (void *)0) handle_malloc_err("next_item");
			*next_item = (TokenItem) {.token = next_tok, .next = (void *)0};
			end_item->next = next_item;
			end_item = next_item;
			token_list.size++;
		}
	}

	return token_list;
}

static void free_token_list(TokenList token_list) {
	TokenItem *current_item = token_list.start;
	while (current_item != (void *)0) {
		Token current = current_item->token;
		if (current.label == STRING) free(current.data.string.name);
		else free(current.data.tok.name);
    	TokenItem *next_item = current_item->next;
    	free(current_item);
    	current_item = next_item;
    }
}

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

// Typical stack functions
static void push_nodelist(NodeList *restrict const list, Node const*restrict const node) {
	NodeItem *const old_start_item = list->start;
	NodeItem *const new_start_item = malloc(sizeof(NodeItem));
	if (new_start_item == (void *)0) handle_malloc_err("new_start_item");
	*new_start_item = (NodeItem) {
		.node = (Node *)node,  // removes constness
		.next = old_start_item
	};
	list->size++;
	list->start = new_start_item;
}

static Node *pop_nodelist(NodeList *const list) {
	if (list->size == 0) return (void *)0;
	NodeItem *const old_start_item = list->start;
	Node *const node = old_start_item->node;
	NodeItem *const new_start_item = old_start_item->next;
	free(old_start_item);
	list->start = new_start_item;
	list->size--;
	return node;
}

static inline unsigned long long most_sig_bit(size_t const size) {
	return 1 << ((8 * size) - 1);
}

static void append_to_node_children(Node *restrict const node, Node *restrict const subnode) {
	if (node->label == LEAF) handle_leaf_node_append();
	if (node->sub_cap < most_sig_bit(sizeof(size_t)) && node->sub_sz + 1 >= node->sub_cap) {
		node->sub_cap *= 2;
		node->sub = realloc(node->sub, node->sub_cap * sizeof(Node *));
		if (node->sub == (void *)0) handle_realloc_err("node->sub");
	}
	node->sub[node->sub_sz++] = subnode;
}

// takes the top Node of the list and makes it a subnode of the one below it.
static void under_nodelist(NodeList *const list) {
	Node *const node = pop_nodelist(list);
	if (node == (void *)0) return;
	append_to_node_children(list->start->node, node);
}

static void free_nodelist(NodeList const list) {
	NodeItem *current_item = list.start;
	while (current_item != (void *)0) {
    	NodeItem *const next_item = current_item->next;
    	free(current_item);
    	current_item = next_item;
    }
}

static Node *parse(TokenList tokenlist) {
	Node *const start_node = malloc(sizeof(Node));
	*start_node = (Node) {
		.label = ROOT,
		.content = (void *)0,
		.sub = malloc(sizeof(Node*)),
		.sub_sz = 0,
		.sub_cap = 1,
	};
	if (start_node->sub == (void *)0) handle_malloc_err("start_node->sub");

	NodeItem *const start_node_item = malloc(sizeof(NodeItem));
	*start_node_item = (NodeItem) {
		.node = start_node,
		.next = (void *)0
	};
	if (start_node_item == (void *)0) handle_malloc_err("start_node_item");
	
	NodeList list = (NodeList) {
		.size = 1,
		.start = start_node_item
	};

	while (tokenlist.size > 0) {
		TokenItem *start_item = tokenlist.start;
		tokenlist.start = start_item->next;
		tokenlist.size--;
		Token start = start_item->token;

		if (start.label == STRING) {
			Node *const string_node = malloc(sizeof(Node));
			if (string_node == (void *)0) handle_malloc_err("string_node");
			*string_node = (Node) {
				.label = LEAF,
				.content = start.data.string.name,
				.sub = (void *)0,
				.sub_sz = 0,
				.sub_cap = 0,
			};
			push_nodelist(&list, string_node);
			under_nodelist(&list);
			continue;
		}

		// Must be a tag
		if(start.data.tok.type == START) {
			Node *const start_node = malloc(sizeof(Node));
			if (start_node == (void *)0) handle_malloc_err("start_node");
			*start_node = (Node) {
				.label = BRANCH,
				.content = start.data.tok.name,
				.sub = malloc(sizeof(Node*)),
				.sub_sz = 0,
				.sub_cap = 1,
			};
			if (start_node->sub == (void *)0) handle_malloc_err("start_node->sub");
			push_nodelist(&list, start_node);
			continue;
		}

		// Must be an END tag
		char const*const start_tag = start.data.tok.name;
		char const*const end_tag = list.start->node->content;
		if (strcmp(start_tag, end_tag) != 0)
			handle_mismatch_tag_error(start_tag, end_tag);
		else
			under_nodelist(&list);
	}

	// only the start needs freeing since pop automatically frees all other node items
	free(start_node_item);
	
	return start_node;
}

static void free_nodetree(Node *const node) {
	for (size_t i = 0; i < node->sub_sz; ++i)
		free_nodetree(node->sub[i]);

	free(node);
}

static void print_nodetree(Node const*const node, size_t const indent) {
	for (size_t i = 0; i < indent; ++i)
		(void)printf("  ");

	(void)printf(
		"Node{c=\"%s\",l=%s,s=%p,sz=%zu,cap=%zu} @ %p\n",
		node->content,
		node->label == ROOT? "ROOT" : (node->label == BRANCH? "BRANCH" : "LEAF"),
		(void *)node->sub,
		node->sub_sz,
		node->sub_cap,
		(void *)node
	);
	for (size_t i = 0; i < node->sub_sz; ++i)
		print_nodetree(node->sub[i], indent + 1);
}

static inline NodeList empty_node_list(void) {
	return (NodeList) {.start = (void *)0, .size = 0};
}

static void find_tags_rec(Node const*restrict const node, NodeList *const list, char const*restrict const tag) {
	if (node->label == LEAF) return;
	for (size_t i = 0; i < node->sub_sz; ++i) {
		find_tags_rec(node->sub[i], list, tag);
	}
	if (node->label == ROOT) return;

	if (strcmp(node->content, tag) == 0) {
		push_nodelist(list, node);
	}
}

static inline NodeList find_tags(Node const*restrict const node, char const*restrict const tag) {
    NodeList search_list = empty_node_list();
    find_tags_rec(node, &search_list, tag);
    return search_list;
}

static Node const *find_tag(Node const*restrict const node, char const*restrict const tag) {
	if (node->label == LEAF) return (void *)0;
	for (size_t i = 0; i < node->sub_sz; ++i) {
		Node const*const found = find_tag(node->sub[i], tag);
		if (found != (void *)0) return found;
	}
	if (node->label == ROOT) return (void *)0;

	if (strcmp(node->content, tag) == 0) {
		return node;
	}

	return (void *)0;
}

int main(int const argc, char const*const argv[argc]) {
	/* arg handling */
	if (argc <= 1) {
		(void)fputs("No file path provided\n", stderr);
		return EXIT_FAILURE;
	}

	char *const xml_text = copy_file_data(argv[1]);
	TokenList token_list = tokenise(xml_text);
	free(xml_text);

    Node const*const root = parse(token_list);
    puts("Whole XML Tree:");
    print_nodetree(root, 0);
    puts("");

    NodeList person_list = find_tags(root, "person");
	NodeItem const *person_item = person_list.start;

	while (person_item != (void *)0) {
		char const*const firstname = find_tag(person_item->node, "firstname")->sub[0]->content;
		char const*const lastname = find_tag(person_item->node, "lastname")->sub[0]->content;
		printf("%s %s\n", firstname, lastname);
		person_item = person_item->next;
    }

    free_nodelist(person_list);

    free_nodetree((Node *const)root);
	free_token_list(token_list);
	return EXIT_SUCCESS;
}
