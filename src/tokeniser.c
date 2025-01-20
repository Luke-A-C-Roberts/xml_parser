#include <regex.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "tokeniser.h"
#include "errhandle.h"

static void regcomp_handled(regex_t *restrict const regexp, char const*restrict const pattern) {
	int regerrcode = regcomp(regexp, pattern, REG_EXTENDED);
	if (regerrcode) handle_regcomp_err(regerrcode, regexp);
}

static char *copy_regmatch_data(char *const text_at_pos, regmatch_t const regmatch, size_t const start_offset, size_t const end_offset) {
	ptrdiff_t const regmatch_sz = (regmatch.rm_eo - end_offset) - (regmatch.rm_so + start_offset);
	char *tagstr = malloc(regmatch_sz + 1);
	tagstr[regmatch_sz] = '\0';
	(void)strncpy(tagstr, &text_at_pos[regmatch.rm_so + start_offset], regmatch_sz);
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
		(void)strcpy(_text, text);
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

extern TokenList tokenise(char const*const xml_text) {
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

extern void free_token_list(TokenList token_list) {
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

