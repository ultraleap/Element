#if !defined(ELEMENT_TOKEN_H)
#define ELEMENT_TOKEN_H

#include "element/common.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief token type
 */
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

/**
 * @brief token information
 */
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

/**
 * @brief tokeniser context
 */
typedef struct element_tokeniser_ctx element_tokeniser_ctx;

/**
 * @brief creates a tokeniser context
 *
 * @param[in] tokeniser         tokeniser context
 *
 * @return ELEMENT_OK created an interpreter successfully
 * @return ELEMENT_ERROR_API_OUTPUT_IS_NULL tokeniser pointer is null
 */
element_result element_tokeniser_create(
    element_tokeniser_ctx** tokeniser);

/**
 * @brief deletes a tokeniser context
 *
 * @param[in,out] tokeniser     tokeniser context 
 */
void element_tokeniser_delete(
    element_tokeniser_ctx** tokeniser);

/**
 * @brief runs converts an input expression to a sequence of tokens
 *
 * @param[in] tokeniser         tokeniser context 
 * @param[in] input             file data
 * @param[in] filename          file name
 * *
 * @return ELEMENT_OK tokeniser executed successfully
 * @return ELEMENT_ERROR_API_TOKENISER_CTX_IS_NULL tokeniser pointer is null
 * @return ELEMENT_ERROR_API_STRING_IS_NULL file name pointer is null
 */
element_result element_tokeniser_run(
    element_tokeniser_ctx* tokeniser, 
    const char* input, 
    const char* filename);

/**
 * @brief clears the tokeniser context
 *
 * @param[in] tokeniser         tokeniser context 
 *
 * @return ELEMENT_OK tokeniser cleared successfully
 * @return ELEMENT_ERROR_API_TOKENISER_CTX_IS_NULL tokeniser pointer is null
 */
element_result element_tokeniser_clear(
    element_tokeniser_ctx* tokeniser);

/**
 * @brief sets the callback to be used to consume error message responses
 *
 * @param[in] tokeniser         tokeniser context  
 * @param[in] log_callback      log callback function pointer
 * @param[in] user_data         user data pointer
 *
 * @return ELEMENT_OK set log callback successfully
 * @return ELEMENT_ERROR_API_TOKENISER_CTX_IS_NULL tokeniser pointer is null
 */
element_result element_tokeniser_set_log_callback(
    element_tokeniser_ctx* tokeniser, 
    void (*log_callback)(const element_log_message*, void*), 
    void* user_data);

/**
 * @brief gets the file name associated with a tokeniser
 *
 * @param[in] tokeniser         tokeniser context  
 * @param[in] filename          file name
 *
 * @return ELEMENT_OK retrieved file name successfully
 * @return ELEMENT_ERROR_API_TOKENISER_CTX_IS_NULL tokeniser pointer is null
 * @return ELEMENT_ERROR_API_OUTPUT_IS_NULL file name pointer is null
 */
element_result element_tokeniser_get_filename(
    const element_tokeniser_ctx* tokeniser,
    const char** filename);

/**
 * @brief gets the input associated with a tokeniser
 *
 * @param[in] tokeniser         tokeniser context  
 * @param[in] input             input
 *
 * @return ELEMENT_OK retrieved input successfully
 * @return ELEMENT_ERROR_API_TOKENISER_CTX_IS_NULL tokeniser pointer is null
 * @return ELEMENT_ERROR_API_OUTPUT_IS_NULL input pointer is null
 */
element_result element_tokeniser_get_input(
    const element_tokeniser_ctx* tokeniser, 
    const char** input);

/**
 * @brief gets the token count associated with a tokeniser
 *
 * @param[in] tokeniser         tokeniser context
 * @param[in] count             token count
 *
 * @return ELEMENT_OK retrieved token count successfully
 * @return ELEMENT_ERROR_API_TOKENISER_CTX_IS_NULL tokeniser pointer is null
 * @return ELEMENT_ERROR_API_OUTPUT_IS_NULL token count pointer is null
 */
element_result element_tokeniser_get_token_count(
    const element_tokeniser_ctx* tokeniser, 
    size_t* count);

/**
 * @brief gets a token from a given index
 *
 * @param[in] tokeniser         tokeniser context  
 * @param[in] index             index of token
 * @param[in] token             token output
 * @param[in] msg               error message, if present
 *
 * @return ELEMENT_OK retrieved a token successfully
 * @return ELEMENT_ERROR_API_TOKENISER_CTX_IS_NULL tokeniser pointer is null
 * @return ELEMENT_ERROR_API_OUTPUT_IS_NULL token count pointer is null
 */
element_result element_tokeniser_get_token(
    const element_tokeniser_ctx* tokeniser, 
    size_t index, 
    const element_token** token, 
    const char* msg);

/**
 * @brief converts a token to string
 *
 * @param[in] tokeniser             tokeniser context  
 * @param[in] token_to_mark         token to highlight in output string 
 * @param[in] output_buffer         output buffer
 * @param[in] output_buffer_size    output buffer size
 *
 * @return ELEMENT_OK converted tokeniser to string successfully
 * @return ELEMENT_ERROR_API_TOKENISER_CTX_IS_NULL tokeniser pointer is null
 * @return ELEMENT_ERROR_API_OUTPUT_IS_NULL output buffer pointer is null
 * @return ELEMENT_ERROR_API_INSUFFICIENT_BUFFER buffer size too small
 */
element_result element_tokeniser_to_string(
    const element_tokeniser_ctx* tokeniser, 
    const element_token* token_to_mark, 
    char* output_buffer,
    int output_buffer_size);

#if defined(__cplusplus)
}
#endif
#endif