 #include <stdio.h>
#include <stdlib.h>

#include "io.h"
#include "errhandle.h"

/* I/O */
extern char *copy_file_data(char const*const filename) {
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

extern void print_nodetree(Node const*const node, size_t const indent) {
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
