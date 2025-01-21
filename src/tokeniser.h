#ifndef TOKENISER_H
#define TOKENISER_H

#include <stddef.h>

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

extern TokenList tokenise(char const*const xml_text);
extern void print_token_list(TokenList const token_list);
extern void free_token_list(TokenList token_list);

#endif //TOKENISER_H
