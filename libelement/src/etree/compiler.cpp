#include "etree/compiler.hpp"

#include <cassert>
#include <utility>

#include <fmt/format.h>

#include "ast/ast_indexes.hpp"

//When compiling a function that needs direct input from the boundary, generate placeholder expressions to represent that input when it's evaluated
static std::vector<compilation> generate_placeholder_inputs(const element_type* t)
{
    std::vector<compilation> results;
    const size_t insize = t->inputs().size();
    results.reserve(insize);
    for (size_t i = 0; i < insize; ++i) {
        auto expression = std::make_shared<element_expression_input>(i, t->inputs()[i].type->get_size());
        constraint_const_shared_ptr constraint = t->inputs()[i].type; //todo: have a look and see if these types make sense
        results.emplace_back(compilation{ std::move(expression), std::move(constraint) });
    }
    return results;
}

static expression_shared_ptr generate_intrinsic_expression(const element_intrinsic* fn, const std::vector<compilation>& args)
{
    //todo: logging rather than asserting?

    if (auto ui = fn->as<element_intrinsic_unary>()) {
        assert(args.size() >= 1);
        return std::make_shared<element_expression_unary>(ui->operation(), args[0].expression);
    }

    if (auto bi = fn->as<element_intrinsic_binary>()) {
        assert(args.size() >= 2);
        return std::make_shared<element_expression_binary>(bi->operation(), args[0].expression, args[1].expression);
    }

    assert(false);
    return nullptr;
}

static element_result compile_intrinsic(
    element_compiler_ctx& ctx,
    const element_function* fn,
    std::vector<compilation> inputs,
    expression_shared_ptr& expr)
{
    if (const auto ui = fn->as<element_intrinsic_unary>()) {
        assert(inputs.size() >= 1);
        // TODO: better error codes
        //todo: logging
        if (inputs[0].expression->get_size() != 1) return ELEMENT_ERROR_ARGS_MISMATCH;
        expr = std::make_shared<element_expression_unary>(ui->operation(), inputs[0].expression);
        return ELEMENT_OK;
    }

    if (const auto bi = fn->as<element_intrinsic_binary>()) {
        assert(inputs.size() >= 2);
        // TODO: better error codes
        //todo: logging
        if (inputs[0].expression->get_size() != 1) return ELEMENT_ERROR_ARGS_MISMATCH;
        if (inputs[1].expression->get_size() != 1) return ELEMENT_ERROR_ARGS_MISMATCH;
        expr = std::make_shared<element_expression_binary>(bi->operation(), inputs[0].expression, inputs[1].expression);
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
    expression_shared_ptr& expr)
{
    assert(fn->inputs().size() >= inputs.size());

    // TODO: is flat list here OK?
    std::vector<std::pair<std::string, expression_shared_ptr>> deps;
    deps.reserve(inputs.size());
    for (size_t i = 0; i < inputs.size(); ++i)
        deps.emplace_back(fn->inputs()[i].name, inputs[i].expression);
    expr = std::make_shared<element_expression_structure>(std::move(deps));
    return ELEMENT_OK;
}

static element_result compile_expression(
    element_compiler_ctx& ctx,
    const element_scope* scope,
    const element_ast* bodynode,
    compilation& output_compilation);

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

    const auto fn = scope->function();
    //todo: understand what this chunk of code does, what it's caching, and when that cache will be used again
    //todo: DRY
    
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

static element_result compile_call_experimental_literal(
    element_compiler_ctx& ctx,
    const element_scope* callsite_root,
    const element_ast* callsite_node,
    const element_scope*& callsite_current,
    compilation& output_compilation);

static element_result compile_call_experimental(
    element_compiler_ctx& ctx,
    const element_scope* callsite_root,
    const element_ast* callsite_node,
    const element_scope*& callsite_current,
    compilation& output_compilation); //note: might contain an expression, if we had a parent

static element_result compile_call_experimental_function(
    element_compiler_ctx& ctx,
    const element_scope* callsite_root,
    const element_ast* callsite_node,
    const element_scope* parent_scope,
    const element_scope*& callsite_current,
    compilation& output_compilation);  //note: might contain an expression, if we had a parent

static element_result compile_call_experimental_namespace(
    const element_ast* callsite_node,
    const element_scope* parent_scope,
    const expression_shared_ptr& expr);

static element_result compile_call_experimental_literal(
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

static element_result compile_call_experimental_namespace(
    const element_ast* callsite_node,
    const element_scope* parent_scope,
    const expression_shared_ptr& expr)
{
    const bool has_parent = parent_scope;

    //todo; re-enable one james has fixed the lack of parentage on these reversed call nodes
    /*const bool has_child = callsite_node->parent
                        && callsite_node->parent->type == ELEMENT_AST_NODE_CALL;

    //Having a namespace that isn't being indexed is an error
    if (!has_child)
        return ELEMENT_ERROR_UNKNOWN; //todo*/

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

//This does a bunch of stuff to get the type that the constructor returns, but that seems unecessary, as callsite_current that our parent modified should already point to that structs scope
static element_result compile_call_experimental_function_find_ourselves_when_parent_is_constructor(
    const element_ast* callsite_node,
    const element_scope* parent_scope,
    const element_scope*& our_scope)
{
    if (our_scope && our_scope->name == callsite_node->identifier)
        return ELEMENT_OK; //we already found ourselves

    const auto parent_as_function = parent_scope->function();
    if (!parent_as_function) {
        assert(false);
        return ELEMENT_ERROR_UNKNOWN;
    }

    //the type of a custom function isn't something that could contain us
    if (parent_as_function->is<element_custom_function>()) {
        assert(false);
        return ELEMENT_ERROR_UNKNOWN;
    }

    //the type of an intrinsic function isn't something that could contain us
    if (parent_as_function->is<element_intrinsic>()) {
        assert(false); 
        return ELEMENT_ERROR_UNKNOWN;
    }

    //must be a constructor, maybe we missed a case
    assert(parent_as_function->is<element_type_ctor>());

    //the type of a constructor is the type that constructor creates
    const auto type = parent_as_function->type();
    assert(type);
    const auto named_type = type->as<element_type_named>();
    assert(named_type);

    const element_scope* named_type_scope = named_type->scope();
    assert(named_type_scope); //todo
    assert(parent_scope == named_type_scope); //debug. This is at least true when dealing with literals/Num

    assert(our_scope == named_type_scope->lookup(callsite_node->identifier, false)); //debug. //This is at least true when dealing with literals/Num, so this is all pointless for that case

    //find ourselves in the struct
    our_scope = named_type_scope->lookup(callsite_node->identifier, false);
    assert(our_scope); //todo
    return ELEMENT_OK;
}

//this does partial application of parent to arguments for this call. This only works when the parent is a constructor (including literals)
static element_result compile_call_experimental_function_partial_application(
    const element_scope* our_scope,
    std::vector<compilation>& args,
    const element_scope* parent_scope,
    const compilation& compiled_parent)
{
    if (!compiled_parent.expression)
        return ELEMENT_OK; //without a compiled parent, there's nothing to do partial application with. todo: check that the argument counts are still correct in this situation

    //We must be a function for partial application to make sense
    const auto fn = our_scope->function();
    assert(fn); //todo, should be if, because if it's not a function then there's no partial application to do

    //if we're missing an argument to a method call while indexing, then pass the parent as the first argument
    const bool mising_one_argument = fn->inputs().size() == args.size() + 1;
    //One of the few places that does type checking in libelement?
    const bool argument_one_matches_parent_type = !fn->inputs().empty() && fn->inputs()[0].type->is_satisfied_by(compiled_parent.constraint);
    if (mising_one_argument && argument_one_matches_parent_type) {
        args.insert(args.begin(), compiled_parent);
    }

    assert(fn->inputs().size() == args.size()); //todo
    return ELEMENT_OK;
}

static element_result compile_call_experimental_function(
    element_compiler_ctx& ctx,
    const element_scope* callsite_root,
    const element_ast* callsite_node,
    const element_scope* parent_scope,
    const element_scope*& callsite_current, //todo: rename, it's the indexing scope (of our parent), or our scope, or the scope we're being called from
    compilation& output_compilation)
{
    if (callsite_node->type != ELEMENT_AST_NODE_CALL) {
        assert(false); //todo
        return ELEMENT_ERROR_UNKNOWN;
    }

    //If we had a parent, their compiled expression will be what's passed to us. The exception is namespace parents, but we want to ignore those here.
    const bool has_compiled_parent = output_compilation.expression.get();
    const auto compiled_parent = output_compilation;

    const auto our_scope = callsite_current;

    //Handle any arguments to this function call
    std::vector<compilation> args;
    const auto result = fill_and_compile_arguments_from_callsite(args, ctx, callsite_root, callsite_node);
    assert(result == ELEMENT_OK); //todo

    //todo: understand what this chunk of code does, what it's caching, and when that cache will be used again
    const auto& fn = our_scope->function();
    assert(fn);
    
    //todo: DRY
    if (ctx.comp_cache.is_callstack_recursive(our_scope))
        return ELEMENT_ERROR_CIRCULAR_COMPILATION; //todo: logging

    assert(args.empty() || (fn->inputs().size() >= args.size()));
    auto frame = ctx.comp_cache.add_frame(our_scope); //frame is popped when it goes out of scope

    for (size_t i = 0; i < args.size(); ++i) {
        const auto& parameter = fn->inputs()[i];

        //todo: it seems like a parameters type can be empty in some situations, figure out why and if it's a problem or if it's equivelant to being Any (which is how I'm treating it right now)
        if (parameter.type && !parameter.type->is_satisfied_by(args[i].constraint)) {
            return ELEMENT_ERROR_CONSTRAINT_NOT_SATISFIED; //todo: logging
        }

        const element_scope* input_scope = our_scope->lookup(parameter.name, false);
        ctx.comp_cache.add(input_scope, args[i]);
    }

    //todo: I believe this is seeing if this function was compiled previously when resolving the inputs to another function
    //todo: This doesn't update the fnscope if it's found, which seems to be part of the reason why indexing has issues

    const auto& found_expr = ctx.comp_cache.search(our_scope); //todo: rename to compilation cache
    if (found_expr.expression) {
        output_compilation = found_expr;
        return ELEMENT_OK;
    }

    //Now we've compiled any and all of our parents, and we've compiled any and all of our arguments

    if (has_compiled_parent) {
        /* Compiling our parent resulted in a struct instance, so we index that instance with our name.
          This is struct instance indexing.
          The callsite_scope will be invalid, as we did a lookup of ourselves based on the scope our parent set.
          Struct instances don't have a scope in libelement, as a scope is somewhere in the source code.
          Literals in source code are not struct instances. */
        if (compiled_parent.expression->is<element_expression_structure>()) {
            output_compilation.expression = compiled_parent.expression->as<element_expression_structure>()->output(callsite_node->identifier);

            //We found ourselves in the struct instance, so we have our expression. We can leave now.
            if (output_compilation.expression) {
                //todo: understand what this does and document it
                const auto result = place_args(output_compilation.expression, args);
                //todo: this is broken, because now we're something in a struct instance, but we're just an expression, so good luck to anything indexing in to us (unless we're also a struct instance :b)
                return result;
            }

            //We failed to find ourselves in the struct instance. We fall back out and try something else.
        }

        //Our parent didn't compile to a struct instance, or it did and we couldn't find ourselves as a member of it.
        //Let's try and find ourselves in the struct, if our parent is a constructor. Will this work in the case of `instance = MyType(1)`?
        compile_call_experimental_function_find_ourselves_when_parent_is_constructor(callsite_node, parent_scope, callsite_current);

        //If our parent is something we can pass as an argument, let's try to do so if we're missing an argument
        compile_call_experimental_function_partial_application(our_scope, args, parent_scope, compiled_parent);
    }

    //todo: this branching compilation is basically element_compile? we could maybe try moving some stuff around and call that here instead?
    
    // TODO: temporary check if intrinsic
    //todo: why is this temporary?
    if (fn && fn->is<element_intrinsic>()) {
        output_compilation.expression = generate_intrinsic_expression(fn->as<element_intrinsic>(), args);
        //todo: this just gets the type as declared in source, which is fine when it's Num, but otherwise won't really work. expr_constraint should be determined from generate_intrinsic_expression. Do all intrinsics return numbers? unlikely
        output_compilation.constraint = fn->type()->output("return")->type; 
        //callsite_current remains unchanged, should continue pointing to the intrinsic function which is correct (I think)
        assert(output_compilation.expression); //todo
        return ELEMENT_OK;
    }

    if (fn && fn->is<element_type_ctor>()) {
        //todo: should we not be calling compile_type_ctor?
        std::vector<std::pair<std::string, expression_shared_ptr>> struct_instance_dependents;
        struct_instance_dependents.resize(args.size());
        for (unsigned int i = 0; i < args.size(); ++i) {
            const auto& arg = args[i];
            const auto& param = fn->type()->inputs()[i];
            struct_instance_dependents[i].first = param.name;
            struct_instance_dependents[i].second = arg.expression;
        }
        output_compilation.expression = std::make_shared<element_expression_structure>(std::move(struct_instance_dependents));
        output_compilation.constraint = fn->type(); //the type of a constructor is also the type of the struct it creates
        return ELEMENT_OK;
    }

    if (fn && fn->is<element_custom_function>()) {
        //todo: we do some compiling in ourselves, and some in this function, which is kinda messy. maybe it's possible to make it work together nicely, less duplicated code
        ELEMENT_OK_OR_RETURN(compile_custom_fn_scope(ctx, callsite_current, args, output_compilation));
        const auto btype = fn->type();
        const auto type = btype ? btype->output("return")->type : nullptr;
        const auto ctype = type ? type->as<element_type_named>() : nullptr;
        if (ctype) //Update the scope to point to the scope of the type that our return function is returning. todo: is this just the compilation constraint now?
            callsite_current = ctype->scope();
        else if (output_compilation.constraint == element_type::num) //If the function we compiled resulted in a number, then update the scope to be a Num for the next thing indexing in to us
            callsite_current = callsite_current->root()->lookup("Num", false);
        //otherwise we leave the callsite as it is.. not sure that's correct, but then do we ever hit this case? maybe when returning a first-class function, but we lack a lot of support for t
        //pretend there's an else() assert for now
        return ELEMENT_OK;
    }

    assert(false); //todo
    return ELEMENT_ERROR_UNKNOWN;
}

static element_result compile_call_experimental_compile_parent(
    element_compiler_ctx& ctx,
    const element_scope* callsite_root,
    const element_ast* callsite_node,
    const element_scope*& callsite_current,
    compilation& output_compilation)
{
    const auto our_scope = callsite_current;
    //NOTE {2}: This looks like it can be simplified (see NOTE {1})
    const bool has_parent = callsite_node->children.size() > ast_idx::call::parent && callsite_node->children[ast_idx::call::parent]->type != ELEMENT_AST_NODE_NONE;
    if (!has_parent)
        return ELEMENT_OK;

    //We're starting from the right-most call in the source and found out that we have a parent
    //Before we can start compiling this call, we need to find and compile our parent

    //Our parent could be anything (namespace, struct instance, number, number literal)
    //This will continue recursing until we're at the left-most call in the source (bottom of the AST)
    const auto callsite_node_parent = callsite_node->children[ast_idx::call::parent].get();
    const auto result = compile_call_experimental(ctx, callsite_root, callsite_node_parent, callsite_current, output_compilation);
    assert(result == ELEMENT_OK); //todo
    assert(callsite_current != our_scope); //Our parent is done compiling, so it must update the scope we're indexing in to. This should happen for all situations.
    return result;
}

static element_result compile_call_experimental(
    element_compiler_ctx& ctx,
    const element_scope* callsite_root,
    const element_ast* callsite_node,
    const element_scope*& callsite_current,
    compilation& output_compilation)
{
    if (callsite_node->type == ELEMENT_AST_NODE_LITERAL)
        return compile_call_experimental_literal(ctx, callsite_root, callsite_node, callsite_current, output_compilation);

    if (callsite_node->type != ELEMENT_AST_NODE_CALL)
        return ELEMENT_ERROR_UNKNOWN;

    const auto original_scope = callsite_current;
    const bool has_parent = callsite_node->children.size() > ast_idx::call::parent
                         && callsite_node->children[ast_idx::call::parent]->type != ELEMENT_AST_NODE_NONE;
    if (has_parent) {
        const auto result = compile_call_experimental_compile_parent(ctx, callsite_root, callsite_node, callsite_current, output_compilation);
        assert(result == ELEMENT_OK);
    }
    const auto parent_scope = has_parent ? callsite_current : nullptr;

    //We've now compiled any parents (if we had any parents), so let's find what this call was meant to be
    //If we did have a parent then we don't want to recurse when doing the lookup, what we're looking for should be directly in that scope
    //callsite_current could be null if our parent was a dependent in an element_structure, since struct instances have no scope
    if (callsite_current) {
        callsite_current = callsite_current->lookup(callsite_node->identifier, !has_parent);

        //todo: this is necessary because the current handling for struct body/instance indexing happens when we know we're a function, but right now we don't know what we are. We should be doing that stuff here, not in compiling the function
        //We couldn't find ourselves but we do have a parent, so let's try indexing the constraint in case we're part of a struct body
        if (!callsite_current && has_parent) {
            //todo: this first searches the struct body before the struct instance, which means shadowing names will probably cause reverse behaviour to what is expected
            if (output_compilation.constraint->is<element_type>()
             && output_compilation.constraint->as<element_type>() == element_type::num.get()) {
                callsite_current = parent_scope->root()->lookup("Num", false);
                assert(callsite_current); //we failed to find Num in source, but we're indexing in to a number. You can't index in to a number without the Num intrinsic, so this is a user error
                callsite_current = callsite_current->lookup(callsite_node->identifier, false); //Now we can find ourselves within Num
                assert(callsite_current); //we failed to find ourselves in Num, some kind of error
            }
            else if (output_compilation.constraint->is<element_type_named>()
                  && output_compilation.constraint->as<element_type_named>()) {
                const auto named_type = output_compilation.constraint->as<element_type_named>();
                callsite_current = named_type->scope()->lookup(callsite_node->identifier, false); //Now we can find ourselves within the struct body
                assert(callsite_current); //we failed to find ourselves in there, some kind of error
            }
            else if (output_compilation.constraint->is<element_type_anonymous>()
                  && output_compilation.constraint->as<element_type_anonymous>()) {
                const auto anonymous_type = output_compilation.constraint->as<element_type_named>();
                //todo: a function type is anonymous, but indexing a function isn't valid. Not sure if there are situations where an anonymous type is generated, requires more research
            }
            //Something else, so don't try indexing in to it?
        }
    }

    const auto our_scope = callsite_current;
    assert(our_scope);

    if (our_scope->function())
        return compile_call_experimental_function(ctx, callsite_root, callsite_node, parent_scope, callsite_current, output_compilation);

    if (our_scope->node->type == ELEMENT_AST_NODE_NAMESPACE)
        return compile_call_experimental_namespace(callsite_node, parent_scope, output_compilation.expression);

    //This node we're compiling came from a port, so its expression should be cached from the function that called the function this port is for
    if (our_scope->node->type == ELEMENT_AST_NODE_PORT) {
        const auto& cached_compilation = ctx.comp_cache.search(our_scope); //todo: rename to compilation cache
        if (cached_compilation.expression) {
            output_compilation = cached_compilation;
            return ELEMENT_OK;
        }
    }

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
    compilation& output_compilation)
{
    // do we have a body?
    if (bodynode->type == ELEMENT_AST_NODE_CALL || bodynode->type == ELEMENT_AST_NODE_LITERAL) {
        // literal or non-constant expression
        return compile_call_experimental(ctx, scope, bodynode, scope, output_compilation);
    } else if (bodynode->type == ELEMENT_AST_NODE_LAMBDA) {
        // lambda
        return compile_lambda(ctx, scope, bodynode, output_compilation.expression);
    } else if (bodynode->type == ELEMENT_AST_NODE_SCOPE) {
        // function in current scope
        // generate inputs
        auto inputs = generate_placeholder_inputs(scope->function()->type().get());
        // now compile function using those inputs
        return compile_custom_fn_scope(ctx, scope, std::move(inputs), output_compilation);
    }
    else if (bodynode->type == ELEMENT_AST_NODE_FUNCTION) {
        //todo: a function declaration isn't an expression, but NestedConstructs/AddUsingLocal seems to generate a scope hiearchy with nested returns, so compile_custom_fn_scope calls us with a "return" function
        //maybe this is valid, considering there's also a case for ELEMENT_AST_NODE_LAMBDA (although lambdas aren't implemented yet)
        //todo: we should figure out why the returns are being nested (return of addUsingLocal, is a function that also has a return, which contains the actual call to localAdd
        // generate inputs
        auto inputs = generate_placeholder_inputs(scope->function()->type().get());
        return compile_custom_fn_scope(ctx, scope, std::move(inputs), output_compilation);
    } else {
        // interface
        ctx.ictx.logger->log(ctx, ELEMENT_ERROR_NO_IMPL, "Tried to compile an expression with no implementation.", bodynode);
        return ELEMENT_ERROR_NO_IMPL;  // TODO: better error code
    }
}

static element_result compile_custom_fn(
    element_compiler_ctx& ctx,
    const element_function* fn,
    std::vector<compilation> inputs,
    compilation& output_compilation)
{
    const auto cfn = fn->as<element_custom_function>();
    const element_scope* scope = cfn->scope();
    if (ctx.comp_cache.is_callstack_recursive(scope))
        return ELEMENT_ERROR_CIRCULAR_COMPILATION; //todo: logging
    auto frame = ctx.comp_cache.add_frame(scope); //frame is popped when it goes out of scope
    return compile_custom_fn_scope(ctx, scope, std::move(inputs), output_compilation);
}

static element_result element_compile(
    element_interpreter_ctx& ctx,
    const element_function* fn,
    std::vector<compilation> inputs,
    compilation& output_compilation,
    element_compiler_options opts)
{
    element_compiler_ctx cctx = { ctx, std::move(opts) };

    if (fn->is<element_intrinsic>()) {
        return compile_intrinsic(cctx, fn, std::move(inputs), output_compilation.expression);
    } else if (fn->is<element_type_ctor>()) {
        return compile_type_ctor(cctx, fn, std::move(inputs), output_compilation.expression);
    } else if (fn->is<element_custom_function>()) {
        auto result = compile_custom_fn(cctx, fn, std::move(inputs), output_compilation);
        //todo: check the expr_constraint matches the constraint of the function (probably `evaluate` if CLI) we're compiling, but only if result is OK
        return result;
    } else {
        assert(false);
        //todo: logging
        return ELEMENT_ERROR_INVALID_OPERATION; // TODO: better error code
    }
}

element_result element_compile(
    element_interpreter_ctx& ctx,
    const element_function* fn,
    compilation& output_compilation,
    element_compiler_options opts)
{
    auto inputs = generate_placeholder_inputs(fn->type().get());
    return element_compile(ctx, fn, std::move(inputs), output_compilation, std::move(opts));
}
