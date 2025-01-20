#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "errhandle.h"

/* error handlers */
extern _Noreturn int handle_wrong_argc_num(int const num) {
	(void)fprintf(stderr, "incorrect number of args: %d provided, 1 exepcted!", num - 1);
	exit(EXIT_FAILURE);
}

extern _Noreturn int handle_regcomp_err(int const errcode, regex_t const *preg) {
	static const size_t errbuf_sz = 64;
	char errbuf[errbuf_sz];
	(void)regerror(errcode, preg, errbuf, errbuf_sz);
	(void)fprintf(stderr, "%s\n", errbuf);
	exit(EXIT_FAILURE);
}

extern _Noreturn int handle_malloc_err(char const*const varname) {
	int const errsv = errno;
	(void)fprintf(stderr, "%s malloc retured nullptr:\n", varname);
	(void)fprintf(stderr, "%s\n", strerror(errsv));
	exit(EXIT_FAILURE);
}

extern _Noreturn int handle_realloc_err(char const*const varname) {
	int const errsv = errno;
	(void)fprintf(stderr, "%s realloc retured nullptr:\n", varname);
	(void)fprintf(stderr, "%s\n", strerror(errsv));
	exit(EXIT_FAILURE);
}

extern _Noreturn int handle_fopen_err(char const*const filename) {
	(void)fprintf(stderr, "File \"%s\" could not be found\n", filename);
	exit(EXIT_FAILURE);
}

extern _Noreturn int handle_fread_error(void) {
	(void)fputs("Not all file data could be read\n", stderr);
	exit(EXIT_FAILURE);
}

extern _Noreturn int handle_fclose_error(char const*const filevarname) {
	int const errsv = errno;
	(void)fprintf(stderr, "fclose(%s) failed:\n", filevarname);
	(void)fprintf(stderr, "%s\n", strerror(errsv));
	exit(EXIT_FAILURE);
}

extern _Noreturn int handle_leaf_node_append(void) {
	(void)fputs("Tried to append a node to a leaf nodes' subnodes\n", stderr);
	exit(EXIT_FAILURE);
}

extern _Noreturn int handle_mismatch_tag_error(char const*restrict const tagname1, char const*restrict const tagname2) {
	(void)fprintf(stderr, "start tag \"%s\" mismatched with end tag \"%s\"\n", tagname1, tagname2);
	exit(EXIT_FAILURE);
}

