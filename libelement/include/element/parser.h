#if !defined(ELEMENT_PARSER_H)
#define ELEMENT_PARSER_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "element/ast.h"
#include "element/common.h"
#include "element/token.h"

/**
 * @brief opaque pointer to element_parser_ctx
*/
typedef struct element_parser_ctx element_parser_ctx;

/**
 * @brief creates a parser context from a tokeniser context
 *
 * @param[in] tokeniser           tokeniser context
 * @param[out] parser             parser context
 *
 * @return ELEMENT_OK tokeniser created successfully
 * @return ELEMENT_ERROR_API_TOKENISER_CTX_IS_NULL tokeniser pointer is null
 * @return ELEMENT_ERROR_API_OUTPUT_IS_NULL parser pointer is null
 */
element_result element_parser_create(
    element_tokeniser_ctx* tokeniser, 
    element_parser_ctx** parser);

/**
 * @brief deletes a parser context
 *
 * @param[in,out] parser            parser context
 */
void element_parser_delete(
    element_parser_ctx** parser);

/**
 * @brief obtains the root ast node of the parser context
 *
 * @param[in] parser                parser context
 * @param[out] ast                  ast node
 *
 * @return ELEMENT_OK root ast retrieved successfully
 * @return ELEMENT_ERROR_API_PARSER_CTX_IS_NULL parser pointer is null
 * @return ELEMENT_ERROR_API_OUTPUT_IS_NULL ast node pointer is null
 */
element_result element_parser_get_ast(
    element_parser_ctx* parser, 
    element_ast** ast);

/**
 * @brief builds the ast tree from the information supplied to the parser context
 *
 * @param[in] parser                parser context
 *
 * @return ELEMENT_OK ast tree created successfully
 * @return ELEMENT_ERROR_API_PARSER_CTX_IS_NULL parser pointer is null
 */
element_result element_parser_build_ast(
    element_parser_ctx* parser);

#if defined(__cplusplus)
}
#endif
#endif