#include "interpreter_internal.hpp"
#include "common_internal.hpp"
#include "etree/compiler.hpp"
#include "etree/evaluator.hpp"
#include "ast/ast_indexes.hpp"

#include <algorithm>
#include <functional>
#include <cassert>


static scope_unique_ptr get_names(element_scope* parent, element_ast* node)
{
    switch (node->type) {
    case ELEMENT_AST_NODE_ROOT:
    {
        auto root = scope_new(parent, "<global>", node);
        for (const auto& t : node->children) {
            auto cptr = get_names(root.get(), t.get());
            root->children.emplace(cptr->name, std::move(cptr));
        }
        return std::move(root);
    }
    case ELEMENT_AST_NODE_PORT:
    {
        if (!node->identifier.empty())
            return scope_new(parent, node->identifier, node);
        else
            return scope_new_anonymous(parent, node);
    }
    case ELEMENT_AST_NODE_NAMESPACE:
    {
        auto item = scope_new(parent, node->identifier, node);
        // body
        if (node->children[ast_idx::ns::body]->type == ELEMENT_AST_NODE_SCOPE) {
            for (const auto& t : node->children[ast_idx::ns::body]->children) {
                auto cptr = get_names(item.get(), t.get());
                item->children.try_emplace(cptr->name, std::move(cptr));
            }
        }
        return std::move(item);
    }
    case ELEMENT_AST_NODE_FUNCTION:
    case ELEMENT_AST_NODE_STRUCT:
    {
        assert(node->children.size() > ast_idx::fn::body);
        element_ast* declnode = node->children[ast_idx::fn::declaration].get();
        assert(declnode->type == ELEMENT_AST_NODE_DECLARATION);
        auto item = scope_new(parent, declnode->identifier, node);
        // inputs
        if (declnode->children.size() > ast_idx::decl::inputs && declnode->children[ast_idx::decl::inputs]->type == ELEMENT_AST_NODE_PORTLIST) {
            for (const auto& t : declnode->children[ast_idx::decl::inputs]->children) {
                auto cptr = get_names(item.get(), t.get());
                item->children.emplace(cptr->name, std::move(cptr));
            }
        }
        // body
        if (node->children[ast_idx::fn::body]->type == ELEMENT_AST_NODE_SCOPE) {
            for (const auto& t : node->children[ast_idx::fn::body]->children) {
                auto cptr = get_names(item.get(), t.get());
                item->children.try_emplace(cptr->name, std::move(cptr));
            }
        }
        // outputs
        if (declnode->children.size() > ast_idx::decl::outputs) {
            element_ast* outputnode = declnode->children[ast_idx::decl::outputs].get();
            // these should typically already exist from the body, so just try
            if (outputnode->type == ELEMENT_AST_NODE_PORTLIST) {
                for (const auto& t : outputnode->children) {
                    auto cptr = get_names(item.get(), t.get());
                    item->children.try_emplace(cptr->name, std::move(cptr));
                }
            } else if (outputnode->type == ELEMENT_AST_NODE_TYPENAME) {
                auto cptr = scope_new(item.get(), "return", node->children[ast_idx::fn::body].get());
                item->children.try_emplace(cptr->name, std::move(cptr));
            } else if (outputnode->type == ELEMENT_AST_NODE_NONE) {
                // implied any return
                auto cptr = scope_new(item.get(), "return", node->children[ast_idx::fn::body].get());
                item->children.try_emplace(cptr->name, std::move(cptr));
            } else {
                assert(false);
            }
        }
        return std::move(item);
    }
    default:
        return scope_unique_ptr(nullptr);
    }
}

static element_result add_ast_names(std::unordered_map<const element_ast*, const element_scope*>& map, const element_scope* root)
{
    if (map.try_emplace(root->node, root).second) {
        for (const auto& kv : root->children)
            ELEMENT_OK_OR_RETURN(add_ast_names(map, kv.second.get()));
        return ELEMENT_OK;
    } else {
        return ELEMENT_ERROR_INVALID_OPERATION;
    }
}

static element_result merge_names(scope_unique_ptr& a, scope_unique_ptr b, const element_scope* parent)
{
    if (!a) {
        a = std::move(b);
        a->parent = parent;
        return ELEMENT_OK;
    }

    if (a->item_type() != b->item_type()) {
        return ELEMENT_ERROR_INVALID_ARCHIVE; // TODO
    }

    if (a->item_type() != ELEMENT_ITEM_NAMESPACE && a->item_type() != ELEMENT_ITEM_ROOT) {
        return ELEMENT_ERROR_INVALID_ARCHIVE;
    }

    b->parent = a->parent;

    for (auto& bc : b->children) {
        // get the target scope, or create it if it doesn't already exist
        auto& child = a->children[bc.first];
        ELEMENT_OK_OR_RETURN(merge_names(child, std::move(bc.second), a.get()));
    }

    return ELEMENT_OK;
}

element_result element_interpreter_ctx::load(const char* str, const char* filename)
{
    element_tokeniser_ctx* raw_tctx;
    ELEMENT_OK_OR_RETURN(element_tokeniser_create(&raw_tctx));
    // Make a smart pointer out of the tokeniser so it's deleted on an early return
    auto tctx = std::unique_ptr<element_tokeniser_ctx, decltype(&element_tokeniser_delete)>(raw_tctx, element_tokeniser_delete);
    ELEMENT_OK_OR_RETURN(element_tokeniser_run(raw_tctx, str, filename));

    element_ast* raw_ast = NULL;
    ELEMENT_OK_OR_RETURN(element_ast_build(raw_tctx, &raw_ast));
    // element_ast_print(raw_ast);
    auto ast = ast_unique_ptr(raw_ast, element_ast_delete);

    scope_unique_ptr root = get_names(nullptr, raw_ast);
    ELEMENT_OK_OR_RETURN(add_ast_names(ast_names, root.get()));
    ELEMENT_OK_OR_RETURN(merge_names(names, std::move(root), nullptr));
    trees.push_back(std::make_pair(filename, std::move(ast)));

    return ELEMENT_OK;
}

element_interpreter_ctx::element_interpreter_ctx()
{
    // TODO: hack, remove
    clear();
}

element_result element_interpreter_ctx::clear()
{
    trees.clear();
    names.reset();
    ast_names.clear();

    // TODO: temporary hack to get intrinsics in
    std::string input = " \
    num; \
    add(a:num, b:num):num; \
    acos(a:num); \
    asin(a:num); \
    atan(a:num); \
    atan2(a:num, b : num); \
    ceil(a:num); \
    cos(a:num); \
    div(a:num, b:num); \
    floor(a:num); \
    ln(a:num):num; \
    log(a:num, b:num):num; \
    max(a:num, b:num):num; \
    min(a:num, b:num):num; \
    mul(a:num, b:num):num; \
    pow(a:num, b:num):num; \
    rem(a:num, b:num):num; \
    sin(a:num):num; \
    sub(a:num, b:num):num; \
    tan(a:num):num; \
    ";
    load(input.c_str());

    return ELEMENT_OK;
}


element_result element_interpreter_create(element_interpreter_ctx** ctx)
{
    *ctx = new element_interpreter_ctx();
    return ELEMENT_OK;
}

void element_interpreter_delete(element_interpreter_ctx* ctx)
{
    delete ctx;
}

element_result element_interpreter_load_string(element_interpreter_ctx* ctx, const char* string, const char* filename)
{
    assert(ctx);
    return ctx->load(string, filename);
}

element_result element_interpreter_clear(element_interpreter_ctx* ctx)
{
    assert(ctx);
    return ctx->clear();
}

element_result element_interpreter_get_function(element_interpreter_ctx* ctx, const char* name, const element_function** fn)
{
    assert(ctx);
    assert(name);
    if (!ctx->names) return ELEMENT_ERROR_NOT_FOUND;
    const element_scope* scope = ctx->names->lookup(name);
    if (scope && scope->function()) {
        *fn = scope->function().get();
        return ELEMENT_OK;
    } else {
        return ELEMENT_ERROR_NOT_FOUND;
    }
}

element_result element_interpreter_compile_function(
    element_interpreter_ctx* ctx,
    const element_function* fn,
    element_compiled_function** cfn,
    const element_compiler_options* opts)
{
    assert(ctx);
    assert(fn);
    assert(cfn);
    element_compiler_options options;
    if (opts)
        options = *opts;
    expression_shared_ptr fn_expr;
    ELEMENT_OK_OR_RETURN(element_compile(*ctx, fn, fn_expr, options));
    *cfn = new element_compiled_function;
    (*cfn)->function = fn;
    (*cfn)->expression = std::move(fn_expr);
    return ELEMENT_OK;
}

void element_interpreter_delete_compiled_function(element_compiled_function* cfn)
{
    delete cfn;
}

element_result element_interpreter_evaluate_function(
    element_interpreter_ctx* ctx,
    const element_compiled_function* cfn,
    const element_value* inputs, size_t inputs_count,
    element_value* outputs, size_t outputs_count,
    const element_evaluator_options* opts)
{
    assert(ctx);
    assert(cfn);
    element_evaluator_options options;
    if (opts)
        options = *opts;
    return element_evaluate(*ctx, cfn->expression, inputs, inputs_count, outputs, outputs_count, options);
}
