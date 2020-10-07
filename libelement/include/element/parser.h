#if !defined(ELEMENT_PARSER_H)
#define ELEMENT_PARSER_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "element/ast.h"

typedef struct element_parser_ctx element_parser_ctx;
element_result element_parser_create(element_tokeniser_ctx* tokeniser, element_parser_ctx** parser);
void element_parser_delete(element_parser_ctx** parser);

//call element_ast_delete when you're done with it, unless you've already rebuilt the AST.
element_result element_parser_get_ast(element_parser_ctx* parser, element_ast** ast);
//invalidates any previously built ast. Will delete any previously built AST.
element_result element_parser_build_ast(element_parser_ctx* parser);

#if defined(__cplusplus)
}
#endif
#endif