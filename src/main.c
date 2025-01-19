#include <regex.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "errhandle.h"
#include "io.h"
#include "tokeniser.h"
#include "parser.h"
#include "queries.h"

int main(int const argc, char const*const argv[argc]) {
	/* arg handling */
	if (argc <= 1) handle_wrong_argc_num(argc);

	/* file reading */
	char *const xml_text = copy_file_data(argv[1]);

	/* tokenisation of xml */
	TokenList token_list = tokenise(xml_text);
	free(xml_text);

	/* parsing of xml into an AST */
    Node const*const root = parse(token_list);
    (void)puts("Whole XML Tree:");
    print_nodetree(root, 0);
    (void)puts("");

    /* searching for "person" and reporting all finds */
    NodeList person_list = find_tags(root, "person");
	NodeItem const *person_item = person_list.start;

	while (person_item != (void *)0) {
		char const*const firstname = find_tag(person_item->node, "firstname")->sub[0]->content;
		char const*const lastname = find_tag(person_item->node, "lastname")->sub[0]->content;
		printf("%s %s\n", firstname, lastname);
		person_item = person_item->next;
    }

    printf("People: %zu\n", person_list.size);

    free_nodelist(person_list);
    free_nodetree((Node *const)root);
	free_token_list(token_list);

	return EXIT_SUCCESS;
}
