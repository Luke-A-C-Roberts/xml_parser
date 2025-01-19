#ifndef IO_H
#define IO_H

#include <stddef.h>
#include "parser.h"

extern char *copy_file_data(char const*const filename);
extern void print_nodetree(Node const*const node, size_t const indent);

#endif //IO_H
