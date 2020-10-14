#include "element/parser.h"

//STD

//LIBS

//SELF
#include "ast/parser_internal.hpp"
#include "token_internal.hpp"

element_result element_parser_create(element_tokeniser_ctx* tokeniser, element_parser_ctx** parser)
{
    if (!tokeniser)
        return ELEMENT_ERROR_API_TOKENISER_CTX_IS_NULL;

    if (!parser)
        return ELEMENT_ERROR_API_OUTPUT_IS_NULL;

    *parser = new element_parser_ctx();
    (*parser)->logger = tokeniser->logger;
    (*parser)->tokeniser = tokeniser;
    return ELEMENT_OK;
}

void element_parser_delete(element_parser_ctx** parser)
{
    if (!parser)
        return;

    delete *parser;
    *parser = nullptr;
}

element_result element_parser_get_ast(element_parser_ctx* parser, element_ast** ast)
{
    if (!parser)
        return ELEMENT_ERROR_API_PARSER_CTX_IS_NULL;

    if (!parser)
        return ELEMENT_ERROR_API_OUTPUT_IS_NULL;

    *ast = parser->root;
    return ELEMENT_OK;
}

element_result element_parser_build_ast(element_parser_ctx* parser)
{
    if (!parser)
        return ELEMENT_ERROR_API_PARSER_CTX_IS_NULL;

    parser->ast_build();
    return ELEMENT_OK;
}