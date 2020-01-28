#include "etree/compiler.hpp"
#include <cassert>
#include <unordered_map>
#include <utility>
#include "ast/ast_indexes.hpp"


static std::vector<expression_shared_ptr> generate_placeholder_inputs(const element_type* t)
{
    std::vector<expression_shared_ptr> results;
    const size_t insize = t->inputs().size();
    results.reserve(insize);
    for (size_t i = 0; i < insize; ++i) {
        results.push_back(std::make_shared<element_input>(i, t->inputs()[i].type->get_size()));
    }
    return results;
}

static expression_shared_ptr generate_intrinsic_expression(const element_intrinsic* fn, const std::vector<expression_shared_ptr>& args)
{
    if (auto ui = fn->as<element_unary_intrinsic>()) {
        assert(args.size() >= 1);
        return std::make_shared<element_unary>(ui->operation(), args[0]);
    } else if (auto bi = fn->as<element_binary_intrinsic>()) {
        assert(args.size() >= 2);
        return std::make_shared<element_binary>(bi->operation(), args[0], args[1]);
    } else {
        assert(false);
        return nullptr;
    }
}

static element_result compile_intrinsic(
    element_compiler_ctx& ctx,
    const element_function* fn,
    std::vector<expression_shared_ptr> inputs,
    expression_shared_ptr& expr)
{
    if (auto ui = fn->as<element_unary_intrinsic>()) {
        assert(inputs.size() >= 1);
        // TODO: better error codes
        if (inputs[0]->get_size() != 1) return ELEMENT_ERROR_ARGS_MISMATCH;
        expr = std::make_shared<element_unary>(ui->operation(), inputs[0]);
        return ELEMENT_OK;
    } else if (auto bi = fn->as<element_binary_intrinsic>()) {
        assert(inputs.size() >= 2);
        // TODO: better error codes
        if (inputs[0]->get_size() != 1) return ELEMENT_ERROR_ARGS_MISMATCH;
        if (inputs[1]->get_size() != 1) return ELEMENT_ERROR_ARGS_MISMATCH;
        expr = std::make_shared<element_binary>(bi->operation(), inputs[0], inputs[1]);
        return ELEMENT_OK;
    } else {
        // not implemented yet
        assert(false);
        return ELEMENT_ERROR_NO_IMPL;
    }
}

static element_result compile_type_ctor(
    element_compiler_ctx& ctx,
    const element_function* fn,
    std::vector<expression_shared_ptr> inputs,
    expression_shared_ptr& expr)
{
    assert(fn->inputs().size() >= inputs.size());

    // TODO: is flat list here OK?
    std::vector<std::pair<std::string, expression_shared_ptr>> deps;
    deps.reserve(inputs.size());
    for (size_t i = 0; i < inputs.size(); ++i)
        deps.emplace_back(fn->inputs()[i].name, inputs[i]);
    expr = std::make_shared<element_structure>(std::move(deps));
    return ELEMENT_OK;
}

element_result compile_expression(
    element_compiler_ctx& ctx,
    const element_scope* scope,
    const element_ast* bodynode,
    expression_shared_ptr& expr);

static element_result compile_custom_fn_scope(
    element_compiler_ctx& ctx,
    const element_scope* scope,
    std::vector<expression_shared_ptr> inputs,
    expression_shared_ptr& expr)
{
    const element_ast* node = scope->node; // FUNCTION
    if (node->type != ELEMENT_AST_NODE_FUNCTION || node->children.size() <= ast_idx::fn::body)
        return ELEMENT_ERROR_INVALID_OPERATION; // TODO: better error code

    assert(scope->function() && scope->function()->inputs().size() >= inputs.size());
    auto frame = ctx.expr_cache.add_frame();
    for (size_t i = 0; i < inputs.size(); ++i) {
        const element_scope* input_scope = scope->lookup(scope->function()->inputs()[i].name, false);
        ctx.expr_cache.add(input_scope, inputs[i]);
    }

    // find output
    const element_scope* output = scope->lookup("return", false);
    return output ? compile_expression(ctx, output, output->node, expr) : ELEMENT_ERROR_INVALID_OPERATION;
}

static element_result compile_call(
    element_compiler_ctx& ctx,
    const element_scope* scope,
    const element_ast* bodynode,
    const element_scope*& fnscope,
    expression_shared_ptr& expr,
    size_t depth = 0)
{
    if (bodynode->type == ELEMENT_AST_NODE_LITERAL) {
        expr = std::make_shared<element_constant>(bodynode->literal);
        fnscope = scope->root()->lookup("num", false); // HACK?
        return ELEMENT_OK;
    }

    std::vector<expression_shared_ptr> args;

    // scope is the current scope the outer call is happening in
    // fnscope tracks the current available scope of the nested call

    if (bodynode->children.size() > ast_idx::call::parent && bodynode->children[ast_idx::call::parent]->type != ELEMENT_AST_NODE_NONE) {
        // we have a parent, compile it first
        expression_shared_ptr parent_expr;
        ELEMENT_OK_OR_RETURN(compile_call(ctx, scope, bodynode->children[ast_idx::call::parent].get(), fnscope, parent_expr, depth + 1));
        args.push_back(std::move(parent_expr));
    } else {
        // we are the root
        // TODO: check for methods
    }

    fnscope = fnscope->lookup(bodynode->identifier, depth == 0);
    if (!fnscope) return ELEMENT_ERROR_INVALID_OPERATION; // TODO: better error code

    // TODO: apply resolve_returns behaviour

    // TODO: check if we're doing partial application

    if (bodynode->children.size() > ast_idx::call::args && bodynode->children[ast_idx::call::args]->type == ELEMENT_AST_NODE_EXPRLIST) {
        // call with args
        const element_ast* callargs = bodynode->children[ast_idx::call::args].get();
        const size_t existing_args = args.size();
        args.resize(existing_args + callargs->children.size());
        for (size_t i = 0; i < callargs->children.size(); ++i)
            ELEMENT_OK_OR_RETURN(compile_expression(ctx, scope, callargs->children[i].get(), args[existing_args + i]));
    }

    assert(args.empty() || (fnscope->function() && fnscope->function()->inputs().size() >= args.size()));
    auto frame = ctx.expr_cache.add_frame();
    for (size_t i = 0; i < args.size(); ++i) {
        const element_scope* input_scope = fnscope->lookup(fnscope->function()->inputs()[i].name, false);
        ctx.expr_cache.add(input_scope, args[i]);
    }

    expr = ctx.expr_cache.search(fnscope);
    if (!expr) {
        // TODO: temporary check if intrinsic
        if (fnscope->function() && fnscope->function()->is<element_intrinsic>()) {
            expr = generate_intrinsic_expression(fnscope->function()->as<element_intrinsic>(), args);
            if (!expr) return ELEMENT_ERROR_INVALID_OPERATION;
        } else if (bodynode->children.size() > ast_idx::call::parent && bodynode->children[ast_idx::call::parent]->type != ELEMENT_AST_NODE_NONE) {
            // compound identifier with "parent"
            // TODO: fix this for methods, currently won't go looking in the type
            assert(bodynode->children[ast_idx::call::parent]->type == ELEMENT_AST_NODE_CALL);
            expression_shared_ptr parent;
            ELEMENT_OK_OR_RETURN(compile_call(ctx, scope, bodynode->children[ast_idx::call::parent].get(), fnscope, parent, depth + 1));
            // TODO: check better, return error
            if (!parent || !parent->is<element_structure>()) return ELEMENT_ERROR_INVALID_OPERATION; // TODO: better error code
            expr = parent->as<element_structure>()->output(bodynode->identifier);
        } else {
            ELEMENT_OK_OR_RETURN(compile_custom_fn_scope(ctx, fnscope, args, expr));
        }
    }
    return ELEMENT_OK;
}

static element_result compile_lambda(
    element_compiler_ctx& ctx,
    const element_scope* scope,
    const element_ast* bodynode,
    expression_shared_ptr& expr)
{
    // TODO: this
    return ELEMENT_ERROR_NO_IMPL;
}

static element_result compile_expression(
    element_compiler_ctx& ctx,
    const element_scope* scope,
    const element_ast* bodynode,
    expression_shared_ptr& expr)
{
    // do we have a body?
    if (bodynode->type == ELEMENT_AST_NODE_CALL || bodynode->type == ELEMENT_AST_NODE_LITERAL) {
        // literal or non-constant expression
        return compile_call(ctx, scope, bodynode, scope, expr);
    } else if (bodynode->type == ELEMENT_AST_NODE_LAMBDA) {
        // lambda
        return compile_lambda(ctx, scope, bodynode, expr);
    } else if (bodynode->type == ELEMENT_AST_NODE_SCOPE) {
        // function in current scope
        // generate inputs
        auto inputs = generate_placeholder_inputs(scope->function()->type().get());
        // now compile function using those inputs
        return compile_custom_fn_scope(ctx, scope, std::move(inputs), expr);
    } else {
        // interface
        return ELEMENT_ERROR_NO_IMPL;  // TODO: better error code
    }
}

static element_result compile_custom_fn(
    element_compiler_ctx& ctx,
    const element_function* fn,
    std::vector<expression_shared_ptr> inputs,
    expression_shared_ptr& expr)
{
    auto cfn = fn->as<element_custom_function>();
    const element_scope* scope = cfn->scope();
    return compile_custom_fn_scope(ctx, scope, std::move(inputs), expr);
}

static element_result element_compile(
    element_interpreter_ctx& ctx,
    const element_function* fn,
    std::vector<expression_shared_ptr> inputs,
    expression_shared_ptr& expr,
    element_compiler_options opts)
{
    element_compiler_ctx cctx = { ctx, std::move(opts) };
    if (fn->is<element_intrinsic>()) {
        return compile_intrinsic(cctx, fn, std::move(inputs), expr);
    } else if (fn->is<element_type_ctor>()) {
        return compile_type_ctor(cctx, fn, std::move(inputs), expr);
    } else if (fn->is<element_custom_function>()) {
        return compile_custom_fn(cctx, fn, std::move(inputs), expr);
    } else {
        assert(false);
        return ELEMENT_ERROR_INVALID_OPERATION; // TODO: better error code
    }
}

element_result element_compile(
    element_interpreter_ctx& ctx,
    const element_function* fn,
    expression_shared_ptr& expr,
    element_compiler_options opts)
{
    auto inputs = generate_placeholder_inputs(fn->type().get());
    return element_compile(ctx, fn, std::move(inputs), expr, std::move(opts));
}
