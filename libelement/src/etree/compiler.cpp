#include "etree/compiler.hpp"

#include <cassert>
#include <utility>

#include <fmt/format.h>

#include "ast/ast_indexes.hpp"

static element_result element_compile(
    element_compiler_ctx& ctx,
    const element_function* fn,
    std::vector<compilation>&& inputs,
    compilation& output_compilation);

static std::vector<compilation> generate_placeholder_inputs(
    const element_type* t);

static element_result compile_type_ctor(
    element_compiler_ctx& ctx,
    const element_function* fn,
    std::vector<compilation> inputs,
    compilation& output_compilation);

static element_result compile_expression(
    element_compiler_ctx& ctx,
    const element_scope* scope,
    const element_ast* bodynode,
    compilation& output_compilation);

static element_result compile_custom_fn_scope(
    element_compiler_ctx& ctx,
    const element_scope* scope,
    std::vector<compilation> args,
    compilation& output_compilation);

static element_result compile_call_literal(
    element_compiler_ctx& ctx,
    const element_scope* callsite_root,
    const element_ast* callsite_node,
    const element_scope*& callsite_current,
    compilation& output_compilation);

static element_result compile_call(
    element_compiler_ctx& ctx,
    const element_scope* callsite_root,
    const element_ast* callsite_node,
    const element_scope*& callsite_current,
    compilation& output_compilation); //note: might contain an expression, if we had a parent

static element_result compile_call_function(
    element_compiler_ctx& ctx,
    const element_scope* callsite_root,
    const element_ast* callsite_node,
    const element_scope* our_scope, //todo: rename, it's the indexing scope (of our parent), or our scope, or the scope we're being called from
    compilation& output_compilation);  //note: might contain an expression, if we had a parent

static element_result compile_call_namespace(
    const element_ast* callsite_node,
    const element_scope* parent_scope,
    const expression_shared_ptr& expr);

static element_result place_args(
    expression_shared_ptr& expr,
    const std::vector<compilation>& args);

static element_result compile_custom_fn(
    element_compiler_ctx& ctx,
    const element_function* fn,
    std::vector<compilation> inputs,
    compilation& output_compilation);

static void compile_call_indexing_struct(
    const element_ast* callsite_node,
    const compilation& compiled_parent,
    const element_scope** output_scope
    );

//When compiling a function that needs direct input from the boundary, generate placeholder expressions to represent that input when it's evaluated
static std::vector<compilation> generate_placeholder_inputs(
    const element_type* t)
{
    std::vector<compilation> results;
    const auto insize = t->inputs().size();
    results.reserve(insize);
    for (size_t i = 0; i < insize; ++i) {
        auto expression = std::make_shared<element_expression_input>(i, t->inputs()[i].type->get_size());
        auto constraint = t->inputs()[i].type; //todo: have a look and see if these types make sense
        results.emplace_back(compilation{ std::move(expression), std::move(constraint) });
    }
    return results;
}

static element_result compile_intrinsic(
    element_compiler_ctx& ctx,
    const element_function* fn,
    std::vector<compilation> inputs,
    compilation& output_compilation)
{
    if (const auto* const ni = fn->as<element_intrinsic_nullary>()) {
        assert(inputs.size() == 0);
        // TODO: better error codes
        //todo: logging
        output_compilation.expression = std::make_shared<element_expression_nullary>(ni->operation());
        output_compilation.constraint = fn->type()->output("return")->type;
        return ELEMENT_OK;
    }
	
    if (const auto* const ui = fn->as<element_intrinsic_unary>()) {
        assert(inputs.size() >= 1);
        // TODO: better error codes
        //todo: logging
        if (inputs[0].expression->get_size() != 1) return ELEMENT_ERROR_ARGS_MISMATCH;
        output_compilation.expression = std::make_shared<element_expression_unary>(ui->operation(), inputs[0].expression);
        output_compilation.constraint = fn->type()->output("return")->type;
        return ELEMENT_OK;
    }

    if (const auto* const bi = fn->as<element_intrinsic_binary>()) {
        assert(inputs.size() >= 2);
        // TODO: better error codes
        //todo: logging
        if (inputs[0].expression->get_size() != 1) return ELEMENT_ERROR_ARGS_MISMATCH;
        if (inputs[1].expression->get_size() != 1) return ELEMENT_ERROR_ARGS_MISMATCH;
        output_compilation.expression = std::make_shared<element_expression_binary>(bi->operation(), inputs[0].expression, inputs[1].expression);
        output_compilation.constraint = fn->type()->output("return")->type;
        return ELEMENT_OK;
    }

    if (const auto* const bi = fn->as<element_intrinsic_if>()) {
        assert(inputs.size() >= 3);
        // TODO: better error codes
        //todo: logging
        if (inputs[0].expression->get_size() != 1) return ELEMENT_ERROR_ARGS_MISMATCH;
        if (inputs[1].expression->get_size() != 1) return ELEMENT_ERROR_ARGS_MISMATCH;
        if (inputs[2].expression->get_size() != 1) return ELEMENT_ERROR_ARGS_MISMATCH;
        output_compilation.expression = std::make_shared<element_expression_if>(inputs[0].expression, inputs[1].expression, inputs[2].expression);
        output_compilation.constraint = fn->type()->output("return")->type;
        return ELEMENT_OK;
    }

    // not implemented yet
    ctx.ictx.logger->log(ctx, ELEMENT_ERROR_NO_IMPL, fmt::format("Tried to compile intrinsic {} with no implementation.", fn->name()));
    assert(false);
    return ELEMENT_ERROR_NO_IMPL;
}

static element_result compile_type_ctor(
    element_compiler_ctx& ctx,
    const element_function* fn,
    std::vector<compilation> inputs,
    compilation& output_compilation)
{
    assert(!output_compilation.valid());
    assert(fn->inputs().size() >= inputs.size());

    // TODO: is flat list here OK?
    std::vector<std::pair<std::string, expression_shared_ptr>> deps;
    deps.reserve(inputs.size());

    for (size_t i = 0; i < inputs.size(); ++i)
        deps.emplace_back(fn->inputs()[i].name, inputs[i].expression);

    output_compilation.expression = std::make_shared<element_expression_structure>(std::move(deps));
    output_compilation.constraint = fn->type(); //A constructors type is the type that it creates
    return ELEMENT_OK;
}

static element_result compile_custom_fn_scope(
    element_compiler_ctx& ctx,
    const element_scope* scope,
    std::vector<compilation> args,
    compilation& output_compilation)
{
    const element_ast* node = scope->node;

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

    if (ctx.comp_cache.is_callstack_recursive(scope))
        return ELEMENT_ERROR_CIRCULAR_COMPILATION; //todo: logging

    auto frame = ctx.comp_cache.add_frame(scope); //frame is popped when it goes out of scope

    const auto fn = scope->function();

    //Ensure our arguments match up
    if (fn->inputs().size() != args.size())
        return ELEMENT_ERROR_ARGUMENT_COUNT_MISMATCH;

    assert(fn && fn->inputs().size() >= args.size());
    for (size_t i = 0; i < args.size(); ++i) {
        const auto& parameter = fn->inputs()[i];

        //todo: it seems like a parameters type can be empty in some situations, figure out why and if it's a problem or if it's equivelant to being Any (which is how I'm treating it right now)
        if (parameter.type && !parameter.type->is_satisfied_by(args[i].constraint)) {
            return ELEMENT_ERROR_CONSTRAINT_NOT_SATISFIED; //todo: logging
        }

        const element_scope* input_scope = scope->lookup(parameter.name, false);
        ctx.comp_cache.add(input_scope, args[i]);
    }

    // find output
    // output is a function that's always present in the body of a function/lambda, representing what it returns
    const element_scope* output = scope->lookup("return", false);

    if (output) {
        const auto result = compile_expression(ctx, output, output->node, output_compilation);
        if (result != ELEMENT_OK)
            return result;

        const auto& fn_type = fn->type();
        const auto fn_body = fn->type()->output("return");
        assert(fn_body); //todo: is it possible for a function to not have a "return" output? I don't think so. generate_port_cache doesn't seem to exclude "return"

        const auto& fn_return_type = fn_body->type;
        if (!fn_return_type->is_satisfied_by(output_compilation.constraint))
            return ELEMENT_ERROR_CONSTRAINT_NOT_SATISFIED; //todo: logging

        return ELEMENT_OK;
    }

    ctx.ictx.logger->log(ctx, ELEMENT_ERROR_INVALID_OPERATION,
        fmt::format("Tried to find return scope in function scope {} and failed.", scope->name),
        node);
    return ELEMENT_ERROR_INVALID_OPERATION;
}

//todo: understand what this does and document it
static element_result place_args(expression_shared_ptr& expr, const std::vector<compilation>& args)
{
    if (const auto ua = expr->as<element_expression_unbound_arg>()) {
        if (ua->index() < args.size()) {
            expr = args[ua->index()].expression;
            return ELEMENT_OK;
        } else {
            return ELEMENT_ERROR_ARGS_MISMATCH; //logging is done by the caller
        }
    } else {
        for (auto& dep : expr->dependents()) {
            const auto result = place_args(dep, args);
            if (result != ELEMENT_OK)
                return result; //logging is done by the caller
        }
        return ELEMENT_OK;
    }
}

static element_result compile_call_literal(
    element_compiler_ctx& ctx,
    const element_scope* callsite_root,
    const element_ast* callsite_node,
    const element_scope*& callsite_current,
    compilation& output_compilation)
{
    if (callsite_node->type != ELEMENT_AST_NODE_LITERAL) {
        assert(false); //todo
        return ELEMENT_ERROR_UNKNOWN;
    }

    output_compilation.expression = std::make_shared<element_expression_constant>(callsite_node->literal);
    output_compilation.constraint = element_type::num; //todo: should this be num (the internal type) or Num (the named type)? it matters a bit
    callsite_current = callsite_root->root()->lookup("Num", false); // HACK?
    return ELEMENT_OK;
}

static element_result compile_call_namespace(
    const element_ast* callsite_node,
    const element_scope* parent_scope,
    const expression_shared_ptr& expr)
{
    const bool has_parent = parent_scope;

    const bool has_child = callsite_node->parent
                        && callsite_node->parent->type == ELEMENT_AST_NODE_CALL;

    //Having a namespace that isn't being indexed is an error
    if (!has_child)
        return ELEMENT_ERROR_UNKNOWN; //todo

    //A namespace can index in to another namespace, but nothing else
    if (has_parent && parent_scope->node->type != ELEMENT_AST_NODE_NAMESPACE)
        return ELEMENT_ERROR_UNKNOWN; //todo

    //An expression implies something was compiled before getting to us, but that shouldn't be possible
    if (expr)
        return ELEMENT_ERROR_UNKNOWN; //todo

    //Our scope has already been updated, so our child will index in to us correctly
    return ELEMENT_OK;
}

static element_result fill_and_compile_arguments_from_callsite(
    std::vector<compilation>& args,
    element_compiler_ctx& ctx,
    const element_scope* callsite_root,
    const element_ast* callsite_node)
{
    assert(args.empty());
    assert(callsite_root);
    assert(callsite_node);

    const bool calling_with_arguments = callsite_node->children.size() > ast_idx::call::args
        && callsite_node->children[ast_idx::call::args]->type == ELEMENT_AST_NODE_EXPRLIST;

    if (calling_with_arguments) {
        const auto callargs_node = callsite_node->children[ast_idx::call::args].get();
        args.resize(callargs_node->children.size());

        //Compile all of the exprlist AST nodes and assign them to the arguments we're calling with
        for (size_t i = 0; i < callargs_node->children.size(); ++i)
        {
            ELEMENT_OK_OR_RETURN(compile_expression(
                ctx,
                callsite_root,
                callargs_node->children[i].get(),
                args[i]
            ));
        }
    }

    return ELEMENT_OK;
};

//this does partial application of parent to arguments for this call. This only works when the parent is a constructor (including literals)
static void compile_call_function_partial_application(
    const element_function* fn,
    std::vector<compilation>& args,
    const compilation& compiled_parent)
{
    assert(fn);

    //without a compiled parent, there's nothing to do partial application with
    if (!compiled_parent.valid())
        return;

    const bool mising_one_argument = fn->inputs().size() == args.size() + 1;
    const bool argument_one_matches_parent_type = !fn->inputs().empty()
                                                && fn->inputs()[0].type->is_satisfied_by(compiled_parent.constraint);

    //if we're missing an argument to a method call while indexing, then pass the parent as the first argument
    if (mising_one_argument && argument_one_matches_parent_type)
        args.insert(args.begin(), compiled_parent);
}

static element_result compile_call_function_indexing_parent_struct_instance(
    const element_ast* callsite_node,
    const std::vector<compilation>& args, //todo: I don't understand what the plan for these was
    const compilation& compiled_parent,
    compilation& output_compilation
    )
{
    //todo: compile_call_experimental checks to see if we're in a struct body, which happens before this check to see if we're in the struct index. this is because of our dependence on args (which is only a thing for functions), but can definitely be cleaned up
    /* Compiling our parent resulted in a struct instance, so we index that instance with our name.
      This is struct instance indexing.
      The callsite_scope will be invalid, as we did a lookup of ourselves based on the scope our parent set.
      Struct instances don't have a scope in libelement, as a scope is somewhere in the source code.
      Literals in source code are not struct instances. */

    //didn't have a parent
    if (!compiled_parent.valid())
        return ELEMENT_OK;

    //parent wasn't a struct instance
    if (!compiled_parent.expression->is<element_expression_structure>())
        return ELEMENT_OK;

    //our name isn't a member of the struct instance
    auto expr = compiled_parent.expression->as<element_expression_structure>()->output(callsite_node->identifier);
    if (!expr)
        return ELEMENT_OK;

    //We found ourselves in the struct instance, so we have our expression
    output_compilation.expression = std::move(expr);
    //todo: understand what this does and document it
    const auto result = place_args(output_compilation.expression, args);
    //todo: this is broken, because now we're something in a struct instance, but we're just an expression, so good luck to anything indexing in to us (unless we're also a struct instance :b) as there's no type information.
    return result;
}

static element_result compile_call_function(
    element_compiler_ctx& ctx,
    const element_scope* callsite_root,
    const element_ast* callsite_node,
    const element_scope* our_scope, //todo: rename, it's the indexing scope (of our parent), or our scope, or the scope we're being called from
    compilation& output_compilation)
{
    assert(callsite_root);
    assert(callsite_node);
    assert(our_scope);
    assert(callsite_node->type == ELEMENT_AST_NODE_CALL);

    if (callsite_node->type != ELEMENT_AST_NODE_CALL)
        return ELEMENT_ERROR_UNKNOWN;

    const auto& fn = our_scope->function();
    assert(fn);

    //If we're a port then we should be in the expression cache, due to the previous function adding us to it
    //todo: are there other situations where we would be in the cache?
    const auto& cached_compilation = ctx.comp_cache.search(our_scope);
    if (cached_compilation.valid()) {
        //we've already been compiled, so we're done
        output_compilation = cached_compilation;
        return ELEMENT_OK;
    }

    //Find and compile any arguments to this function call
    std::vector<compilation> args;
    auto result = fill_and_compile_arguments_from_callsite(args, ctx, callsite_root, callsite_node);
    assert(result == ELEMENT_OK); //todo

    //more arguments than expected
	if(fn->inputs().size() < args.size())
        return ELEMENT_ERROR_ARGUMENT_COUNT_MISMATCH;

    //If we had a parent, their compiled expression will be what's passed to us. The exception is namespace parents, but we want to ignore them as parents anyway
    const auto compiled_parent = std::move(output_compilation);

    //If our parent is a struct instance, let's find our name, as we're already compiled
    result = compile_call_function_indexing_parent_struct_instance(callsite_node, args, compiled_parent, output_compilation);
    if (output_compilation.valid()) {
        return result;
    }

    //If our parent is something we can pass as an argument, let's try to do so if we're missing one
    compile_call_function_partial_application(fn.get(), args, compiled_parent);

	//fewer arguments than expected
    if ( fn->inputs().size() != args.size())
        return ELEMENT_ERROR_ARGUMENT_COUNT_MISMATCH;

    //We can finally compile this function
    return element_compile(ctx, fn.get(), std::move(args), output_compilation);
}

static element_result compile_call_compile_parent(
    element_compiler_ctx& ctx,
    const element_scope* callsite_root,
    const element_ast* callsite_node,
    const element_scope*& callsite_current,
    compilation& output_compilation)
{
    const auto our_scope = callsite_current;
    //NOTE {2}: This looks like it can be simplified (see NOTE {1})
    const bool has_parent = callsite_node->children.size() > ast_idx::call::parent
                         && callsite_node->children[ast_idx::call::parent]->type != ELEMENT_AST_NODE_NONE;
    if (!has_parent)
        return ELEMENT_OK;

    //We're starting from the right-most call in the source and found out that we have a parent
    //Before we can start compiling this call, we need to find and compile our parent

    //Our parent could be anything (namespace, struct instance, number, number literal)
    //This will continue recursing until we're at the left-most call in the source (bottom of the AST)
    const auto callsite_node_parent = callsite_node->children[ast_idx::call::parent].get();
    const auto result = compile_call(ctx, callsite_root, callsite_node_parent, callsite_current, output_compilation);
    assert(result == ELEMENT_OK); //todo
    assert(callsite_current != our_scope); //Our parent is done compiling, so it must update the scope we're indexing in to. This should happen for all situations.
    return result;
}

static void compile_call_indexing_struct(
    const element_ast* callsite_node,
    const compilation& compiled_parent,
    const element_scope** output_scope
    )
{
    //has parent
    if (!compiled_parent.valid())
        return;

    //we already know who we are
    if (*output_scope)
        return;

    //todo: this is necessary because the current handling for struct body/instance indexing happens when we know we're a function, but right now we don't know what we are. We should be doing that stuff here, not in compiling the function
    //We couldn't find ourselves but we do have a parent, so let's try indexing the constraint in case we're part of a struct body
    //todo: this first searches the struct body before the struct instance, which means shadowing names will probably cause reverse behaviour to what is expected

    const auto named_type = compiled_parent.constraint->as<element_type_named>();
    const auto anonymous_type = compiled_parent.constraint->as<element_type_named>();

    if (named_type) {
        //Now we can find ourselves within the struct body
        *output_scope = named_type->scope()->lookup(callsite_node->identifier, false);
        //we failed to find ourselves in there, some kind of error
        assert(*output_scope);
    }
    else if (anonymous_type) {
        //todo: a function type is anonymous, but indexing a function isn't valid. Not sure if there are situations where an anonymous type is generated, requires more research
        //todo: log error
        assert(false);
    }
    else {
        //Something else?
        assert(false);
    }
}

static element_result compile_call(
    element_compiler_ctx& ctx,
    const element_scope* callsite_root,
    const element_ast* callsite_node,
    const element_scope*& callsite_current,
    compilation& output_compilation)
{
    if (callsite_node->type == ELEMENT_AST_NODE_LITERAL)
        return compile_call_literal(ctx, callsite_root, callsite_node, callsite_current, output_compilation);

    if (callsite_node->type != ELEMENT_AST_NODE_CALL)
        return ELEMENT_ERROR_UNKNOWN;

    const auto original_scope = callsite_current;
    const bool has_parent = callsite_node->children.size() > ast_idx::call::parent
                         && callsite_node->children[ast_idx::call::parent]->type != ELEMENT_AST_NODE_NONE;
    if (has_parent) {
        const auto result = compile_call_compile_parent(ctx, callsite_root, callsite_node, callsite_current, output_compilation);
        assert(result == ELEMENT_OK);
    }
    const auto parent_scope = has_parent ? callsite_current : nullptr;

    //We've now compiled any parents (if we had any parents), so let's find what this call was meant to be
    //If we did have a parent then we don't want to recurse when doing the lookup, what we're looking for should be directly in that scope
    assert(callsite_current);
    callsite_current = callsite_current->lookup(callsite_node->identifier, !has_parent);

    if (callsite_current
     && callsite_current->node->type == ELEMENT_AST_NODE_NAMESPACE)
        return compile_call_namespace(callsite_node, parent_scope, output_compilation.expression);

    //This node we're compiling came from a port, so its expression should be cached from the function that called the function this port is for
    if (callsite_current
     && callsite_current->node->type == ELEMENT_AST_NODE_PORT) {
        const auto& cached_compilation = ctx.comp_cache.search(callsite_current);
        if (cached_compilation.valid()) {
            output_compilation = cached_compilation;
            return ELEMENT_OK;
        }
    }

    compile_call_indexing_struct(callsite_node, output_compilation, &callsite_current);

    //if we got this far and still haven't found ourselves, maybe the user is just wrong :b
    assert(callsite_current);

    if (callsite_current->function())
        return compile_call_function(ctx, callsite_root, callsite_node, callsite_current, output_compilation);

    //Wasn't something we know about
    output_compilation.expression = nullptr;
    output_compilation.constraint = nullptr;
    callsite_current = nullptr;
    assert(false);
    return ELEMENT_ERROR_UNKNOWN; //todo
}

static element_result compile_lambda(
    element_compiler_ctx& ctx,
    const element_scope* scope,
    const element_ast* bodynode,
    compilation& output_compilation)
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
    compilation& output_compilation)
{
    assert(bodynode);

    // literal or non-constant expression
    if (bodynode->type == ELEMENT_AST_NODE_CALL || bodynode->type == ELEMENT_AST_NODE_LITERAL)
        return compile_call(ctx, scope, bodynode, scope, output_compilation);
    
    // lambda
    if (bodynode->type == ELEMENT_AST_NODE_LAMBDA)
        return compile_lambda(ctx, scope, bodynode, output_compilation);

    if (bodynode->type == ELEMENT_AST_NODE_SCOPE) {
        // function in current scope
        // generate inputs
        auto inputs = generate_placeholder_inputs(scope->function()->type().get());
        // now compile function using those inputs
        return compile_custom_fn_scope(ctx, scope, std::move(inputs), output_compilation);
    }

    if (bodynode->type == ELEMENT_AST_NODE_FUNCTION) {
        //todo: a function declaration isn't an expression, but NestedConstructs/AddUsingLocal seems to generate a scope hiearchy with nested returns, so compile_custom_fn_scope calls us with a "return" function
        //maybe this is valid, considering there's also a case for ELEMENT_AST_NODE_LAMBDA (although lambdas aren't implemented yet)
        //todo: we should figure out why the returns are being nested (return of addUsingLocal, is a function that also has a return, which contains the actual call to localAdd
        // generate inputs
        auto inputs = generate_placeholder_inputs(scope->function()->type().get());
        return compile_custom_fn_scope(ctx, scope, std::move(inputs), output_compilation);
    }

    // interface
    if (bodynode->type == ELEMENT_AST_NODE_CONSTRAINT) {
        ctx.ictx.logger->log(ctx, ELEMENT_ERROR_NO_IMPL, "Tried to compile an expression with no implementation.", bodynode);
        return ELEMENT_ERROR_NO_IMPL;  // TODO: better error code
    }

    ctx.ictx.logger->log(ctx, ELEMENT_ERROR_UNKNOWN, "Tried to compile an expression that is unknown", bodynode);
    return ELEMENT_ERROR_UNKNOWN;
}

static element_result compile_custom_fn(
    element_compiler_ctx& ctx,
    const element_function* fn,
    std::vector<compilation> inputs,
    compilation& output_compilation)
{
    const auto cfn = fn->as<element_custom_function>();
    const element_scope* scope = cfn->scope();
    return compile_custom_fn_scope(ctx, scope, std::move(inputs), output_compilation);
}

static element_result element_compile(
    element_compiler_ctx& ctx,
    const element_function* fn,
    std::vector<compilation>&& inputs,
    compilation& output_compilation)
{
    if (fn->is<element_intrinsic>())
        return compile_intrinsic(ctx, fn, std::move(inputs), output_compilation);

    if (fn->is<element_type_ctor>())
        return compile_type_ctor(ctx, fn, std::move(inputs), output_compilation);

    if (fn->is<element_custom_function>()) {
        return compile_custom_fn(ctx, fn, std::move(inputs), output_compilation);
    }

    assert(false);
    //todo: logging
    return ELEMENT_ERROR_INVALID_OPERATION; // TODO: better error code
}

element_result element_compile(
    element_interpreter_ctx& ctx,
    const element_function* fn,
    compilation& output_compilation,
    element_compiler_options opts)
{
    element_compiler_ctx cctx = { ctx, std::move(opts) };
    auto inputs = generate_placeholder_inputs(fn->type().get());
    return element_compile(cctx, fn, std::move(inputs), output_compilation);
}
