#include "etree/compiler.hpp"
#include <cassert>
#include <unordered_map>
#include <utility>
#include "ast/ast_indexes.hpp"

#include <fmt/format.h>

static std::vector<expression_shared_ptr> generate_placeholder_inputs(const element_type* t)
{
    std::vector<expression_shared_ptr> results;
    const size_t insize = t->inputs().size();
    results.reserve(insize);
    for (size_t i = 0; i < insize; ++i) {
        results.push_back(std::make_shared<element_expression_input>(i, t->inputs()[i].type->get_size()));
    }
    return results;
}

static expression_shared_ptr generate_intrinsic_expression(const element_intrinsic* fn, const std::vector<expression_shared_ptr>& args)
{
    if (auto ui = fn->as<element_unary_intrinsic>()) {
        assert(args.size() >= 1);
        return std::make_shared<element_expression_unary>(ui->operation(), args[0]);
    } else if (auto bi = fn->as<element_binary_intrinsic>()) {
        assert(args.size() >= 2);
        return std::make_shared<element_expression_binary>(bi->operation(), args[0], args[1]);
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
        expr = std::make_shared<element_expression_unary>(ui->operation(), inputs[0]);
        return ELEMENT_OK;
    } else if (auto bi = fn->as<element_binary_intrinsic>()) {
        assert(inputs.size() >= 2);
        // TODO: better error codes
        if (inputs[0]->get_size() != 1) return ELEMENT_ERROR_ARGS_MISMATCH;
        if (inputs[1]->get_size() != 1) return ELEMENT_ERROR_ARGS_MISMATCH;
        expr = std::make_shared<element_expression_binary>(bi->operation(), inputs[0], inputs[1]);
        return ELEMENT_OK;
    } else {
        // not implemented yet
        ctx.ictx.logger->log(ctx, ELEMENT_ERROR_NO_IMPL, fmt::format("Tried to compile intrinsic {} with no implementation.", fn->name()));
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
    expr = std::make_shared<element_expression_structure>(std::move(deps));
    return ELEMENT_OK;
}

static element_result compile_expression(
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
    if (node->type != ELEMENT_AST_NODE_FUNCTION) {
        ctx.ictx.logger->log(ctx, ELEMENT_ERROR_INVALID_OPERATION, 
            fmt::format("Tried to compile custom function scope {} but it's not a function.", scope->name),
            node);
        return ELEMENT_ERROR_INVALID_OPERATION; // TODO: better error code
    }

    if (node->children.size() <= ast_idx::fn::body)
    {
        ctx.ictx.logger->log(ctx, ELEMENT_ERROR_INVALID_OPERATION, 
            fmt::format("Tried to compile custom function scope {} but it has no body.", scope->name),
            node);
        return ELEMENT_ERROR_INVALID_OPERATION; // TODO: better error code
    }

    assert(scope->function() && scope->function()->inputs().size() >= inputs.size());
    auto frame = ctx.expr_cache.add_frame();
    for (size_t i = 0; i < inputs.size(); ++i) {
        const element_scope* input_scope = scope->lookup(scope->function()->inputs()[i].name, false);
        ctx.expr_cache.add(input_scope, inputs[i]);
    }

    // find output
    const element_scope* output = scope->lookup("return", false);
    if (output)
        return compile_expression(ctx, output, output->node, expr);

    ctx.ictx.logger->log(ctx, ELEMENT_ERROR_INVALID_OPERATION,
        fmt::format("Tried to find return scope in function scope {} and failed.", scope->name),
        node);
    return ELEMENT_ERROR_INVALID_OPERATION;
}


static element_result place_args(expression_shared_ptr& expr, const std::vector<expression_shared_ptr>& args)
{
    if (auto ua = expr->as<element_unbound_arg>()) {
        if (ua->index() < args.size()) {
            expr = args[ua->index()];
            return ELEMENT_OK;
        } else {
            // TODO: error code
            //logging is done by the caller
            return ELEMENT_ERROR_ARGS_MISMATCH;
        }
    } else {
        for (auto& dep : expr->dependents()) {
            auto result = place_args(dep, args);
            if (result != ELEMENT_OK)
                return result;
        }
        return ELEMENT_OK;
    }
}

static element_result compile_call(
    element_compiler_ctx& ctx,
    const element_scope* scope,
    const element_ast* bodynode,
    const element_scope*& fnscope,
    expression_shared_ptr& expr)
{
    if (bodynode->type == ELEMENT_AST_NODE_LITERAL) {
        expr = std::make_shared<element_expression_constant>(bodynode->literal);
        fnscope = scope->root()->lookup("Num", false); // HACK?
        return ELEMENT_OK;
    }

    std::vector<expression_shared_ptr> args;

    // scope is the current scope the outer call is happening in
    // fnscope tracks the current available scope of the nested call

    const element_scope* orig_fnscope = fnscope;
	//NOTE {2}: This looks like it can be simplified (see NOTE {1})
    const bool has_parent = bodynode->children.size() > ast_idx::call::parent && bodynode->children[ast_idx::call::parent]->type != ELEMENT_AST_NODE_NONE;
    // compound identifier with "parent" - could either be member access or method call
    expression_shared_ptr parent;
    if (has_parent) {
        assert(bodynode->children[ast_idx::call::parent]->type == ELEMENT_AST_NODE_CALL || bodynode->children[ast_idx::call::parent]->type == ELEMENT_AST_NODE_LITERAL);
        ELEMENT_OK_OR_RETURN(compile_call(ctx, scope, bodynode->children[ast_idx::call::parent].get(), fnscope, parent));
        // TODO: check better, return error
        if (!parent) {
            //todo: not sure message is correct for any of these, but better than nothing right now. Fix as issues are found
            ctx.ictx.logger->log(ctx, ELEMENT_ERROR_INVALID_OPERATION,
                fmt::format("Failed to get expression for parent {} while indexing with {}",
                    bodynode->children[ast_idx::call::parent]->identifier, bodynode->identifier),
                bodynode);
            return ELEMENT_ERROR_INVALID_OPERATION; // TODO: better error code
        }
    }
    const element_scope* parent_fnscope = fnscope;

    if (!fnscope) {
        ctx.ictx.logger->log(ctx, ELEMENT_ERROR_INVALID_OPERATION,
            fmt::format("Was going to find {}{} while indexing {} but failed. Started at {}.",
                bodynode->identifier, !has_parent ? " recursively" : "", orig_fnscope->name, scope->name),
            bodynode);
        return ELEMENT_ERROR_INVALID_OPERATION; // TODO: better error code
    }

    fnscope = fnscope->lookup(bodynode->identifier, !has_parent);

    if (!fnscope) {
        ctx.ictx.logger->log(ctx, ELEMENT_ERROR_INVALID_OPERATION,
            fmt::format("Tried to find {} when indexing {}{} within {} but failed.",
                 bodynode->identifier, parent_fnscope->name, !has_parent ? " recursively" : "", scope->parent ? scope->parent->name : scope->name),
            bodynode);

        return ELEMENT_ERROR_INVALID_OPERATION; // TODO: better error code
    }

    // TODO: check if we're doing partial application

    if (bodynode->children.size() > ast_idx::call::args && bodynode->children[ast_idx::call::args]->type == ELEMENT_AST_NODE_EXPRLIST) {
        // call with args
        const element_ast* callargs = bodynode->children[ast_idx::call::args].get();
        args.resize(callargs->children.size());
        for (size_t i = 0; i < callargs->children.size(); ++i)
            ELEMENT_OK_OR_RETURN(compile_expression(ctx, scope, callargs->children[i].get(), args[i]));
    }

    assert(args.empty() || (fnscope->function() && fnscope->function()->inputs().size() >= args.size()));
    auto frame = ctx.expr_cache.add_frame();
    for (size_t i = 0; i < args.size(); ++i) {
        const element_scope* input_scope = fnscope->lookup(fnscope->function()->inputs()[i].name, false);
        ctx.expr_cache.add(input_scope, args[i]);
    }

    expr = ctx.expr_cache.search(fnscope);
    if (!expr) {
        // see if we need to redirect (e.g. method call)
        if (has_parent) {
            if (parent->is<element_expression_structure>())
                expr = parent->as<element_expression_structure>()->output(bodynode->identifier);

            if (expr) {
                auto result = place_args(expr, args);

                if (!result) {
                    ctx.ictx.logger->log(ctx, ELEMENT_ERROR_INVALID_OPERATION,
                        fmt::format("Failed to place arguments in output expression of an element_expression_structure. scope {}", scope->name),
                        bodynode);
                    return result;
                }

                // TODO: more here?
                return ELEMENT_OK;
            } else {
                // no member found - method access?
                auto type = parent_fnscope->function()->type();
                auto ctype = type ? type->as<element_custom_type>() : nullptr;
                if (ctype) {
                    const element_scope* tscope = ctype->scope();
                    fnscope = tscope->lookup(bodynode->identifier, false);
                    if (fnscope) {
                        // found a function in type's scope
                        auto fn = fnscope->function();
                        if (fn->inputs().size() == args.size() + 1 && fn->inputs()[0].type->is_satisfied_by(type)) {
                            // method call, inject parent as first arg
                            args.insert(args.begin(), parent);
                        }

                        if (fn->inputs().size() != args.size()) {
                            ctx.ictx.logger->log(ctx, ELEMENT_ERROR_INVALID_OPERATION,
                                fmt::format("Function input count doesn't match argument count in function {} in type {} for scope {}", fn->name(), ctype->name(), scope->name),
                                bodynode);
                            return ELEMENT_ERROR_INVALID_OPERATION;
                        }

                    } else {
                        ctx.ictx.logger->log(ctx, ELEMENT_ERROR_INVALID_OPERATION,
                            fmt::format("Unable to find method {} in type {}", bodynode->identifier, ctype->name()),
                            bodynode);
                        return ELEMENT_ERROR_INVALID_OPERATION;
                    }
                } else {
                    ctx.ictx.logger->log(ctx, ELEMENT_ERROR_INVALID_OPERATION,
                        fmt::format("Return type of {} could not be found", parent_fnscope->name),
                        bodynode);
                    return ELEMENT_ERROR_INVALID_OPERATION;
                }
            }
        }

        // TODO: temporary check if intrinsic
        if (fnscope->function() && fnscope->function()->is<element_intrinsic>()) {
            expr = generate_intrinsic_expression(fnscope->function()->as<element_intrinsic>(), args);

            if (!expr) {
                ctx.ictx.logger->log(ctx, ELEMENT_ERROR_INVALID_OPERATION,
                    fmt::format("Function {} is intrinsic but failed to generate intrinsic expression.", fnscope->name),
                    bodynode);
                return ELEMENT_ERROR_INVALID_OPERATION;
            }
        }
        else if (fnscope->function() && fnscope->function()->is<element_type_ctor>()) {
            expr = std::shared_ptr<element_expression_structure>(new element_expression_structure({}));
        }
        else {
            ELEMENT_OK_OR_RETURN(compile_custom_fn_scope(ctx, fnscope, args, expr));
            auto btype = fnscope->function()->type();
            auto type = btype ? btype->output("return")->type : nullptr;
            auto ctype = type ? type->as<element_custom_type>() : nullptr;
            if (ctype) {
                fnscope = ctype->scope();
            }
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
    ctx.ictx.logger->log(ctx, ELEMENT_ERROR_NO_IMPL,
        fmt::format("Tried to compile lambda {}. Lambdas are not implemented in the compiler.", scope->name),
        bodynode);
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
        ctx.ictx.logger->log(ctx, ELEMENT_ERROR_NO_IMPL, "Tried to compile an expression with no implementation.", bodynode);
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
