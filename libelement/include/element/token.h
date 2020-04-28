#if !defined(ELEMENT_TOKEN_H)
#define ELEMENT_TOKEN_H

#include "element/common.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef enum
{
    ELEMENT_TOK_NONE = 0,
    ELEMENT_TOK_NUMBER,
    ELEMENT_TOK_IDENTIFIER,
    ELEMENT_TOK_UNDERSCORE,
    ELEMENT_TOK_DOT,
    ELEMENT_TOK_BRACKETL,
    ELEMENT_TOK_BRACKETR,
    ELEMENT_TOK_SEMICOLON,
    ELEMENT_TOK_COLON,
    ELEMENT_TOK_COMMA,
    ELEMENT_TOK_BRACEL,
    ELEMENT_TOK_BRACER,
    ELEMENT_TOK_EQUALS,
} element_token_type;

typedef struct
{
    element_token_type type;
    int pre_pos;
    int pre_len;
    int tok_pos;
    int tok_len;
    int post_pos;
    int post_len;
} element_token;


typedef struct element_tokeniser_ctx element_tokeniser_ctx;

element_result element_tokeniser_create(element_tokeniser_ctx** state);
element_result element_tokeniser_delete(element_tokeniser_ctx* state);

element_result element_tokeniser_get_filename(const element_tokeniser_ctx* state, const char** filename);
element_result element_tokeniser_get_input(const element_tokeniser_ctx* state, const char** input);
element_result element_tokeniser_get_token_count(const element_tokeniser_ctx* state, size_t* count);
element_result element_tokeniser_get_token(const element_tokeniser_ctx* state, const size_t index, const element_token** token);

void element_tokeniser_set_log_callback(element_tokeniser_ctx* state, void (*log_callback)(const element_log_message*));

void element_tokeniser_print(const element_tokeniser_ctx* state);
element_result element_tokeniser_run(element_tokeniser_ctx* state, const char* input, const char* filename);
element_result element_tokeniser_clear(element_tokeniser_ctx* state);

#if defined(__cplusplus)
}
#endif
#endif