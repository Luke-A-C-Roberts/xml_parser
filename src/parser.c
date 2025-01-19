#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "errhandle.h"

// Typical stack functions
extern void push_nodelist(NodeList *restrict const list, Node const*restrict const node) {
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

extern void free_nodelist(NodeList const list) {
	NodeItem *current_item = list.start;
	while (current_item != (void *)0) {
    	NodeItem *const next_item = current_item->next;
    	free(current_item);
    	current_item = next_item;
    }
}

extern Node *parse(TokenList tokenlist) {
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

extern void free_nodetree(Node *const node) {
	for (size_t i = 0; i < node->sub_sz; ++i)
		free_nodetree(node->sub[i]);

	free(node);
}
