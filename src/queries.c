#include <string.h>

#include "queries.h"

static inline NodeList empty_node_list(void) {
	return (NodeList) {.start = (void *)0, .size = 0};
}

extern void find_tags_rec(Node const*restrict const node, NodeList *const list, char const*restrict const tag) {
	if (node->label == LEAF) return;
	for (size_t i = 0; i < node->sub_sz; ++i) {
		find_tags_rec(node->sub[i], list, tag);
	}
	if (node->label == ROOT) return;

	if (strcmp(node->content, tag) == 0) {
		push_nodelist(list, node);
	}
}

extern NodeList find_tags(Node const*restrict const node, char const*restrict const tag) {
    NodeList search_list = empty_node_list();
    find_tags_rec(node, &search_list, tag);
    return search_list;
}

extern Node const *find_tag(Node const*restrict const node, char const*restrict const tag) {
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

