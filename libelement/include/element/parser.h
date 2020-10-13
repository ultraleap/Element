#if !defined(ELEMENT_PARSER_H)
#define ELEMENT_PARSER_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "element/ast.h"

/// <summary>
/// 
/// </summary>
typedef struct element_parser_ctx element_parser_ctx;

/// <summary>
/// 
/// </summary>
/// <param name="tokeniser"></param>
/// <param name="parser"></param>
/// <returns></returns>
element_result element_parser_create(
    element_tokeniser_ctx* tokeniser, 
    element_parser_ctx** parser);

/// <summary>
/// 
/// </summary>
/// <param name="parser"></param>
void element_parser_delete(
    element_parser_ctx** parser);

/// <summary>
/// call element_ast_delete when you're done with it, unless you've already rebuilt the AST.
/// </summary>
/// <param name="parser"></param>
/// <param name="ast"></param>
/// <returns></returns>
element_result element_parser_get_ast(
    element_parser_ctx* parser, 
    element_ast** ast);

/// <summary>
/// invalidates any previously built ast. Will delete any previously built AST.
/// </summary>
/// <param name="parser"></param>
/// <returns></returns>
element_result element_parser_build_ast(
    element_parser_ctx* parser);

#if defined(__cplusplus)
}
#endif
#endif