#include "element/parser.h"

//STD

//LIBS

//SELF
#include "ast/parser_internal.hpp"
#include "token_internal.hpp"

element_result element_parser_create(element_tokeniser_ctx* tokeniser, element_parser_ctx** parser)
{
    *parser = new element_parser_ctx();
    (*parser)->logger = tokeniser->logger;
    (*parser)->tokeniser = tokeniser;
    return ELEMENT_OK;
}

void element_parser_delete(element_parser_ctx** parser)
{
    delete *parser;
    *parser = nullptr;
}

element_result element_parser_get_ast(element_parser_ctx* parser, element_ast** ast)
{
    *ast = parser->root;
    return ELEMENT_OK;
}

element_result element_parser_build_ast(element_parser_ctx* parser)
{
    parser->ast_build();
    return ELEMENT_OK;
}