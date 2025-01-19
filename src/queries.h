#ifndef QUERIES_H
#define QUERIES_H

#include "parser.h"

extern void find_tags_rec(Node const*restrict const node, NodeList *const list, char const*restrict const tag);
extern NodeList find_tags(Node const*restrict const node, char const*restrict const tag);
extern Node const *find_tag(Node const*restrict const node, char const*restrict const tag);

#endif //QUERIES_H
