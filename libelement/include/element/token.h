#if !defined(ELEMENT_TOKEN_H)
#define ELEMENT_TOKEN_H

#include "element/common.h"

#if defined(__cplusplus)
extern "C" {
#endif

/// <summary>
///
/// </summary>
typedef enum
{
    ELEMENT_TOK_NONE = 0,
    ELEMENT_TOK_NUMBER,
    ELEMENT_TOK_IDENTIFIER,
    ELEMENT_TOK_UNDERSCORE,
    ELEMENT_TOK_DOT,
    ELEMENT_TOK_BRACKETL,
    ELEMENT_TOK_BRACKETR,
    ELEMENT_TOK_COLON,
    ELEMENT_TOK_COMMA,
    ELEMENT_TOK_BRACEL,
    ELEMENT_TOK_BRACER,
    ELEMENT_TOK_EQUALS,
    ELEMENT_TOK_EOF,
} element_token_type;

/// <summary>
///
/// </summary>
typedef struct
{
    element_token_type type;
    int pre_pos; //todo: unsure
    int pre_len;
    int tok_pos; //the position in the input string where the token starts
    int tok_len;
    int post_pos; //todo: unsure
    int post_len;

    //for debug/logging
    int line;
    int line_start_position; //the position at which the line starts in the input string
    int character;           //the position in the line where the token starts (starting from 1, not 0)
    const char* file_name;
} element_token;

/// <summary>
///
/// </summary>
typedef struct element_tokeniser_ctx element_tokeniser_ctx;

/// <summary>
///
/// </summary>
/// <param name="tokeniser"></param>
/// <returns></returns>
element_result element_tokeniser_create(
    element_tokeniser_ctx** tokeniser);

/// <summary>
///
/// </summary>
/// <param name="tokeniser"></param>
void element_tokeniser_delete(
    element_tokeniser_ctx** tokeniser);

/// <summary>
///
/// </summary>
/// <param name="tokeniser"></param>
/// <param name="input"></param>
/// <param name="filename"></param>
/// <returns></returns>
element_result element_tokeniser_run(
    element_tokeniser_ctx* tokeniser, 
    const char* input, 
    const char* filename);

/// <summary>
///
/// </summary>
/// <param name="tokeniser"></param>
/// <returns></returns>
element_result element_tokeniser_clear(
    element_tokeniser_ctx* tokeniser);

/// <summary>
///
/// </summary>
/// <param name="tokeniser"></param>
/// <param name="log_callback"></param>
/// <param name="user_data"></param>
/// <returns></returns>
element_result element_tokeniser_set_log_callback(
    element_tokeniser_ctx* tokeniser, 
    void (*log_callback)(const element_log_message*, void*), 
    void* user_data);

/// <summary>
///
/// </summary>
/// <param name="tokeniser"></param>
/// <param name="filename"></param>
/// <returns></returns>
element_result element_tokeniser_get_filename(
    const element_tokeniser_ctx* tokeniser,
    const char** filename);

/// <summary>
///
/// </summary>
/// <param name="tokeniser"></param>
/// <param name="input"></param>
/// <returns></returns>
element_result element_tokeniser_get_input(
    const element_tokeniser_ctx* tokeniser, 
    const char** input);

/// <summary>
///
/// </summary>
/// <param name="tokeniser"></param>
/// <param name="count"></param>
/// <returns></returns>
element_result element_tokeniser_get_token_count(
    const element_tokeniser_ctx* tokeniser, 
    size_t* count);

/// <summary>
///
/// </summary>
/// <param name="tokeniser"></param>
/// <param name="index"></param>
/// <param name="token"></param>
/// <param name="msg"></param>
/// <returns></returns>
element_result element_tokeniser_get_token(
    const element_tokeniser_ctx* tokeniser, 
    size_t index, 
    const element_token** token, 
    const char* msg);

/// <summary>
/// token_to_mark will output "HERE" beside it, pass null if unwanted
/// </summary>
/// <param name="tokeniser"></param>
/// <param name="token_to_mark"></param>
/// <param name="output_buffer"></param>
/// <param name="output_buffer_size"></param>
/// <returns></returns>
element_result element_tokeniser_to_string(
    const element_tokeniser_ctx* tokeniser, 
    const element_token* token_to_mark, 
    char* output_buffer,
    int output_buffer_size);

#if defined(__cplusplus)
}
#endif
#endif