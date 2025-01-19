#ifndef ERROR_H
#define ERROR_H

#include <stddef.h>
#include <regex.h>

/* error handlers */
extern _Noreturn int handle_wrong_argc_num(int const num);
extern _Noreturn int handle_regcomp_err(int const errcode, regex_t const *preg);
extern _Noreturn int handle_malloc_err(char const*const varname);
extern _Noreturn int handle_realloc_err(char const*const varname);
extern _Noreturn int handle_fopen_err(char const*const filename);
extern _Noreturn int handle_fread_error(void);
extern _Noreturn int handle_fclose_error(char const*const filevarname);
extern _Noreturn int handle_leaf_node_append(void);
extern _Noreturn int handle_mismatch_tag_error(char const*restrict const tagname1, char const*restrict const tagname2);

#endif //ERROR_H
