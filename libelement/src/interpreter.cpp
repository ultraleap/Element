#include "interpreter_internal.hpp"

//STD
#include <algorithm>
#include <functional>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>

//LIBS
#include <fmt/format.h>

//SELF
#include "common_internal.hpp"
#include "etree/compiler.hpp"
#include "etree/evaluator.hpp"
#include "etree/expressions.hpp"
#include "token_internal.hpp"
#include "configuration.hpp"
#include "obj_model/object_model.hpp"
#include "obj_model/declarations.hpp"
#include "obj_model/intermediaries.hpp"
#include "obj_model/errors.hpp"
#include "log_errors.hpp"
#include "element/ast.h"

bool file_exists(const std::string& file)
{
    return std::filesystem::exists(file) && std::filesystem::is_regular_file(file);
}

bool directory_exists(const std::string& directory)
{
    return std::filesystem::exists(directory) && std::filesystem::is_directory(directory);
}

element_result element_interpreter_ctx::load(const char* str, const char* filename)
{
	//HACK: JM - Not a fan of this...
    const std::string file = filename;
    const auto starts_with_prelude = file.rfind("Prelude\\", 0) == 0;
	
    element_tokeniser_ctx* tokeniser;
    ELEMENT_OK_OR_RETURN(element_tokeniser_create(&tokeniser))

    tokeniser->logger = logger;

    // Make a smart pointer out of the tokeniser so it's deleted on an early return
    auto tctx = std::unique_ptr<element_tokeniser_ctx, decltype(&element_tokeniser_delete)>(tokeniser, element_tokeniser_delete);

    src_context = std::make_shared<element::source_context>();
    //create the file info struct to be used by the object model later
    element::file_information info;
    info.file_name = std::make_unique<std::string>(filename);
    //pass the pointer to the filename, so that the pointer stored in tokens matches the one we have
    ELEMENT_OK_OR_RETURN(element_tokeniser_run(tokeniser, str, info.file_name.get()->data()))
    if (tokeniser->tokens.empty())
        return ELEMENT_OK;

    const auto total_lines_parsed = tokeniser->line;

    for (auto i = 0; i < total_lines_parsed; ++i)
    {
        //lines start at 1
        info.source_lines.emplace_back(std::make_unique<std::string>(tokeniser->text_on_line(i + 1)));
    }

    auto* const data = info.file_name->data();
    src_context->file_info[data] = std::move(info);

    const auto log_tokens = starts_with_prelude
                                ? flag_set(logging_bitmask, log_flags::output_prelude) && flag_set(logging_bitmask, log_flags::output_tokens)
                                : flag_set(logging_bitmask, log_flags::debug | log_flags::output_tokens);
	
    if (log_tokens) {
			log("\n------\nTOKENS\n------\n" + tokens_to_string(tokeniser));
    }

    element_parser_ctx parser;
    parser.tokeniser = tokeniser;
    parser.logger = logger;
    parser.src_context = src_context;
     
    auto result = parser.ast_build();
    ELEMENT_OK_OR_RETURN(result)

    const auto log_ast = starts_with_prelude
        ? flag_set(logging_bitmask, log_flags::output_prelude) && flag_set(logging_bitmask, log_flags::output_ast)
        : flag_set(logging_bitmask, log_flags::debug | log_flags::output_ast);
	
    if (log_ast) {
        log("\n---\nAST\n---\n" + ast_to_string(parser.root));
    }

#ifdef LEGACY_COMPILER
    auto ast = ast_unique_ptr(parser.root, element_ast_delete);
    scope_unique_ptr root = get_names(nullptr, parser.root);
    result = add_ast_names(ast_names, root.get());
    //todo: hacky message to help with unit tests until we add logging for all error cases
    if (result < ELEMENT_OK) {
        log(result, std::string("add_ast_names failed with element_result " + std::to_string(result)), filename);
    }
    ELEMENT_OK_OR_RETURN(result)

    result = merge_names(names, std::move(root), nullptr);
    //todo: hacky message to help with unit tests until we add logging for all error cases
    if (result < ELEMENT_OK) {
        log(result, std::string("merge_names failed with element_result " + std::to_string(result)), filename);
    }
    ELEMENT_OK_OR_RETURN(result)

    trees.push_back(std::make_pair(filename, std::move(ast)));

    // TODO: HACK
    update_scopes(names.get());
#else

    //parse only enabled, skip object model generation to avoid error codes with positive values
    //i.e. errors returned other than ELEMENT_ERROR_PARSE
    if (parse_only)
    {
        element_ast_delete(parser.root);
        return ELEMENT_OK;
    }

    auto object_model = element::build_root_scope(this, parser.root, result);
    if (result != ELEMENT_OK) {
        log(result, fmt::format("building object model failed with element_result {}", result), filename);
    }

    result = global_scope->merge(std::move(object_model));
    if (result != ELEMENT_OK) {
        log(result, fmt::format("merging object models failed with element_result {}", result), filename);
    }
#endif

    return result;
}

element_result element_interpreter_ctx::load_file(const std::string& file)
{
    if (!file_exists(file)) {
        const auto abs = std::filesystem::absolute(std::filesystem::path(file)).string();
        std::cout << fmt::format("file {} was not found at path {}\n",
            file, abs.c_str()); //todo: proper logging
        return ELEMENT_ERROR_FILE_NOT_FOUND;
    }

    std::string buffer;

    std::ifstream f(file);
    f.seekg(0, std::ios::end);
    buffer.resize(f.tellg());
    f.seekg(0);
    f.read(buffer.data(), buffer.size());

    const auto result = load(buffer.c_str(), file.c_str());
    if (result != ELEMENT_OK) {
        //std::cout << fmt::format("interpreter failed to parse file {}. element_result = {}\n", 
        //    file, result); //todo: proper logging
    }

    return result;
}

element_result element_interpreter_ctx::load_files(const std::vector<std::string>& files)
{
    element_result ret = ELEMENT_OK;

    for (const auto& filename : files) {
        if (!file_exists(filename)) {
            auto abs = std::filesystem::absolute(std::filesystem::path(filename)).string();
            std::cout << fmt::format("file {} was not found at path {}\n",
                filename, abs); //todo: proper logging
            continue; //todo: error handling
        }

        const element_result result = load_file(filename);
        if (result != ELEMENT_OK && ret == ELEMENT_OK) //todo: only returns first error
            ret = result;
    }

    return ret;
}

element_result element_interpreter_ctx::load_package(const std::string& package)
{
    if (!directory_exists(package))
    {
        auto abs = std::filesystem::absolute(std::filesystem::path(package)).string();
        std::cout << fmt::format("package {} does not exist at path {}\n",
            package, abs); //todo: proper logging
        return ELEMENT_ERROR_DIRECTORY_NOT_FOUND;
    }

    element_result ret = ELEMENT_OK;

    for (const auto& file : std::filesystem::recursive_directory_iterator(package)) {
        const auto filename = file.path().string();
        const auto extension = file.path().extension().string();
        if (extension == ".ele")
        {
            const element_result result = load_file(file.path().string());
            if (result != ELEMENT_OK && ret == ELEMENT_OK) //todo: only returns first error
                ret = result;
        }
        else
        {
            std::cout << fmt::format("file {} in package {} has extension {} instead of '.ele'\n", 
                filename, package, extension); //todo: proper logging
        }
    }

    return ret;
}

element_result element_interpreter_ctx::load_packages(const std::vector<std::string>& packages)
{
    element_result ret = ELEMENT_OK;

    for (const auto& package : packages) {
        if (!directory_exists(package)) {
            auto abs = std::filesystem::absolute(std::filesystem::path(package)).string();
            std::cout << fmt::format("package {} was not found at location {}\n",
                package, abs); //todo: proper logging
            continue; //todo: error handling
        }

        const auto result = load_package(package);
        if (result != ELEMENT_OK && ret != ELEMENT_OK) //todo: only returns first error
            ret = result;
    }

    return ret;
}

element_result element_interpreter_ctx::load_prelude()
{
    if (prelude_loaded)
        return ELEMENT_ERROR_PRELUDE_ALREADY_LOADED;

    auto result = load_package("Prelude");
    if (result == ELEMENT_OK) {
        prelude_loaded = true;
        return result;
    }

    if (result == ELEMENT_ERROR_DIRECTORY_NOT_FOUND) {
        auto abs = std::filesystem::absolute(std::filesystem::path("Prelude")).string();
        std::cout << fmt::format("could not find prelude at {}\n", abs); //todo: proper logging
    }

    return result;
}

void element_interpreter_ctx::set_log_callback(LogCallback callback)
{
    logger = std::make_shared<element_log_ctx>();
    logger->callback = callback;
}

void element_interpreter_ctx::log(element_result code, const std::string& message, const std::string& filename) const
{
    if (logger == nullptr)
        return;

	logger->log(*this, code, message, filename);
}

void element_interpreter_ctx::log(const std::string& message) const
{
    if (logger == nullptr)
        return;

    logger->log(message, message_stage::ELEMENT_STAGE_MISC);
}

element_interpreter_ctx::element_interpreter_ctx()
{
    element::detail::register_errors();
    element::detail::register_log_errors();

    // TODO: hack, remove
    global_scope = std::make_unique<element::scope>(nullptr, nullptr);
    clear();
}

element_result element_interpreter_ctx::clear()
{
    //trees.clear();
    //names.reset();
    //ast_names.clear();

    return ELEMENT_OK;
}

element_result element_interpreter_create(element_interpreter_ctx** context)
{
    *context = new element_interpreter_ctx();
    return ELEMENT_OK;
}

void element_interpreter_delete(element_interpreter_ctx* context)
{
    delete context;
}

element_result element_interpreter_load_string(element_interpreter_ctx* context, const char* string, const char* filename)
{
    assert(context);
    return context->load(string, filename);
}

element_result element_interpreter_load_file(element_interpreter_ctx* context, const char* file)
{
    assert(context);
    return context->load_file(file);
}

element_result element_interpreter_load_files(element_interpreter_ctx* context, const char** files, const int files_count)
{
    assert(context);

    std::vector<std::string> actual_files;
    actual_files.resize(files_count);
    for (auto i = 0; i < files_count; ++i) {
        //std::cout << fmt::format("load_file {}\n", files[i]); //todo: proper logging
        actual_files[i] = files[i];
    }

    return context->load_files(actual_files);
}

element_result element_interpreter_load_package(element_interpreter_ctx* context, const char* package)
{
    assert(context);
    //std::cout << fmt::format("load_package {}\n", package); //todo: proper logging
    return context->load_package(package);
}

element_result element_interpreter_load_packages(element_interpreter_ctx* context, const char** packages, const int packages_count)
{
    assert(context);

    std::vector<std::string> actual_packages;
    actual_packages.resize(packages_count);
    for (auto i = 0; i < packages_count; ++i) {
        //std::cout << fmt::format("load_packages {}\n", packages[i]); //todo: proper logging
        actual_packages[i] = packages[i];
    }

    return context->load_packages(actual_packages);
}

element_result element_interpreter_load_prelude(element_interpreter_ctx* context)
{
    assert(context);
    return context->load_prelude();
}

void element_interpreter_parse_only_mode(element_interpreter_ctx* context, bool parse_only)
{
    assert(context);
    context->parse_only = parse_only;
}

void element_interpreter_set_log_callback(element_interpreter_ctx* context, void (*log_callback)(const element_log_message* const))
{
    assert(context);
    context->set_log_callback(log_callback);
}

element_result element_interpreter_clear(element_interpreter_ctx* context)
{
    assert(context);
    return context->clear();
}

element_result element_delete_compilable(element_interpreter_ctx* context, element_compilable** compilable)
{
    delete *compilable;
    *compilable = nullptr;
    return ELEMENT_OK;
}

element_result element_delete_evaluable(element_interpreter_ctx* context, element_evaluable** evaluable)
{
    delete *evaluable;
    *evaluable = nullptr;
    return ELEMENT_OK;
}

element_result element_interpreter_find(element_interpreter_ctx* context, const char* path, element_compilable** compilable)
{
    auto obj = context->global_scope->find(element::identifier(path), false);
    if (!obj)
    {
        *compilable = nullptr;
        context->log(ELEMENT_ERROR_IDENTIFIER_NOT_FOUND, fmt::format("failed to find '{}'.", path));
        return ELEMENT_ERROR_IDENTIFIER_NOT_FOUND;
    }

    *compilable = new element_compilable{std::move(obj)};
    return ELEMENT_OK;
}

element_result valid_boundary_function(
    element_interpreter_ctx* context,
    const element::compilation_context& compilation_context,
    const element_compiler_options* options,
    const element_compilable* compilable)
{
    const auto func_decl = std::dynamic_pointer_cast<element::function_declaration>(compilable->object);
    if (!func_decl)
        return ELEMENT_ERROR_UNKNOWN;

    const bool is_valid = func_decl->valid_at_boundary(compilation_context);
    if (!is_valid)
        return ELEMENT_ERROR_UNKNOWN;


    return ELEMENT_OK;
}

std::vector<std::shared_ptr<element::object>> generate_placeholder_inputs(
    element_interpreter_ctx* context,
    const element::compilation_context& compilation_context,
    const element_compiler_options* options,
    const element_compilable* compilable,
    element_result& out_result)
{
    std::vector<std::shared_ptr<element::object>> placeholder_inputs;
    int placeholder_index = 0;

    for (const auto& input : compilable->object->get_inputs())
    {
        auto placeholder = input.generate_placeholder(compilation_context, placeholder_index);
        if (!placeholder)
            out_result = ELEMENT_ERROR_UNKNOWN;

        placeholder_inputs.push_back(std::move(placeholder));
    }

    return placeholder_inputs;
}

element_result element_interpreter_compile(
    element_interpreter_ctx* context,
    const element_compiler_options* options,
    const element_compilable* compilable,
    element_evaluable** evaluable)
{
    const element::compilation_context compilation_context(context->global_scope.get(), context);

    auto result = valid_boundary_function(context, compilation_context, options, compilable);
    if (result != ELEMENT_OK)
    {
        assert(!"this is not a valid boundary function");
        *evaluable = nullptr;
        return result;
    }

    auto placeholder_inputs = generate_placeholder_inputs(context, compilation_context, options, compilable, result);
    if (result != ELEMENT_OK)
    {
        assert(!"failed to generate placeholder inputs despite being a valid boundary function, bug?");
        *evaluable = nullptr;
        return result;
    }

    const auto compiled = compilable->object->call(compilation_context, std::move(placeholder_inputs), {});

    if (!compiled)
    {
        assert(!"tried to compile something but it resulted in a nullptr");
        *evaluable = nullptr;
        return ELEMENT_ERROR_UNKNOWN;
    }

    const auto err = std::dynamic_pointer_cast<element::error>(compiled);
    if (err)
    {
        *evaluable = nullptr;
        return err->log_once(context->logger.get());
    }

    auto expression = compiled->to_expression();
    if (!expression)
    {
        //the actual type doesn't match the expected one for the boundary function, we should handle this error somewhere else
        //for now we don't, so leave it
        assert(!"this type can't be serialised");
        *evaluable = nullptr;
        return ELEMENT_ERROR_UNKNOWN;
    }

    *evaluable = new element_evaluable{ std::move(expression) };

    return ELEMENT_OK;
}

element_result element_interpreter_evaluate(
    element_interpreter_ctx* context,
    const element_evaluator_options* options,
    const element_evaluable* evaluable,
    const element_inputs* inputs,
    element_outputs* outputs)
{
    element_evaluator_options opts{};

    if (options)
        opts = *options;

    if (!evaluable->evaluable)
    {
        assert(!"tried to evaluate something but it's nullptr");
        return ELEMENT_ERROR_UNKNOWN;
    }

    const auto err = std::dynamic_pointer_cast<element::error>(evaluable->evaluable);
    if (err)
        return err->log_once(context->logger.get());

    auto expr = std::dynamic_pointer_cast<element_expression>(evaluable->evaluable);
    if (!expr)
    {
        //todo: this is a quick hack just to test basic structs
        auto struct_instance = std::dynamic_pointer_cast<element::struct_instance>(evaluable->evaluable);
        if (!struct_instance)
        {
            //TODO: Handle as error
            assert(!"tried to evaluate something but it's not an element_expression or struct instance");
            return ELEMENT_ERROR_UNKNOWN;
        }

        //doesn't calc space of fields :b
        if (static_cast<int>(struct_instance->fields.size()) > outputs->count)
        {
            //TODO: Handle as error
            assert(!"tried to evaluate a struct instance but not enough output space to deserialize it");
            return ELEMENT_ERROR_UNKNOWN;
        }

        auto c = 0;

        for (auto& f : struct_instance->fields)
        {
            element_inputs sub_inputs{ nullptr, 0 };
            element_outputs sub_outputs{&outputs->values[c], outputs->count - c};
            element_evaluable sub_evaluable{ f.second };
            assert(sub_outputs.count > 0);
            const auto result = element_interpreter_evaluate(context, options, &sub_evaluable, &sub_inputs, &sub_outputs);
            if (result != ELEMENT_OK) {
                //TODO: Handle as error
                context->log(result, fmt::format("Failed to evaluate {}", f.second->typeof_info()), "<input>");
                return result;
            }

            c += sub_outputs.count;

            /*auto field_expr = std::dynamic_pointer_cast<element_expression>(f.second);
            if (!field_expr)
            {
                assert(!"tried to evaluate a struct instance but one of the fields is not an element_expression");
                return ELEMENT_ERROR_UNKNOWN;
            }

            std::size_t one = 1;
            const auto result = element_evaluate(
                *context,
                std::move(field_expr),
                nullptr,
                0,
                &outputs->values[c],
                one,
                opts);
            c++;

            if (result != ELEMENT_OK) {
                context->log(result, fmt::format("Failed to evaluate {}", f.second->to_string()), "<input>");
            }*/
        }

        outputs->count = c;
        return ELEMENT_OK;
    }

    std::size_t count = outputs->count;
    const auto result = element_evaluate(
        *context,
        std::move(expr),
        inputs->values,
        inputs->count,
        outputs->values,
        count,
        opts);
    outputs->count = static_cast<int>(count);

    if (result != ELEMENT_OK) {
        context->log(result, fmt::format("Failed to evaluate {}", evaluable->evaluable->typeof_info()), "<input>");
    }

    return result;
}

element_result element_interpreter_compile_expression(
    element_interpreter_ctx* context,
    const element_compiler_options* options,
    const char* expression_string,
    element_evaluable** evaluable)
{
    const element::compilation_context compilation_context(context->global_scope.get(), context);

    element_tokeniser_ctx* tokeniser;
    auto result = element_tokeniser_create(&tokeniser);
    if (result != ELEMENT_OK)
        return result;

    tokeniser->logger = context->logger;

    // Make a smart pointer out of the tokeniser so it's deleted on an early return
    auto tctx = std::unique_ptr<element_tokeniser_ctx, decltype(&element_tokeniser_delete)>(tokeniser, element_tokeniser_delete);

    //create the file info struct to be used by the object model later
    element::file_information info;
    info.file_name = std::make_unique<std::string>("<REMOVE>");

    //hack: forcing terminal on expression
    std::string hack = std::string(expression_string) + ";";
    //pass the pointer to the filename, so that the pointer stored in tokens matches the one we have
    result = element_tokeniser_run(tokeniser, hack.c_str(), info.file_name->data());
    if (result != ELEMENT_OK)
        return result;

    if (tokeniser->tokens.empty())
        return ELEMENT_OK;

    const auto total_lines_parsed = tokeniser->line;

    //lines start at 1
    for (auto i = 0; i < total_lines_parsed; ++i)
        info.source_lines.emplace_back(std::make_unique<std::string>(tokeniser->text_on_line(i + 1)));

    auto* const data = info.file_name->data();
    context->src_context->file_info[data] = std::move(info);

    const auto log_tokens = flag_set(logging_bitmask, log_flags::debug | log_flags::output_tokens);

    if (log_tokens)
        context->log("\n------\nTOKENS\n------\n" + tokens_to_string(tokeniser));

    element_parser_ctx parser;
    parser.tokeniser = tokeniser;
    parser.logger = context->logger;
    parser.src_context = context->src_context;

    element_ast root(nullptr);
    //root.nearest_token = &tokeniser->cur_token;
    parser.root = &root;

    size_t first_token = 0;
    auto* ast = ast_new_child(&root, ELEMENT_AST_NODE_EXPRESSION);
    result = parser.parse_expression(&first_token, ast);
    if (result != ELEMENT_OK)
        return result;

    const auto log_ast = flag_set(logging_bitmask, log_flags::debug | log_flags::output_ast);

    if (log_ast)
        context->log("\n---\nAST\n---\n" + ast_to_string(parser.root));

    //parse only enabled, skip object model generation to avoid error codes with positive values
    //i.e. errors returned other than ELEMENT_ERROR_PARSE
    if (context->parse_only)
    {
        root.children.clear();
        return ELEMENT_OK;
    }

    auto dummy_declaration = std::make_unique<element::function_declaration>(element::identifier{ "<REMOVE>" }, context->global_scope.get(), false);
    parser.root->nearest_token = &tokeniser->tokens[0];
    element::assign_source_information(context, dummy_declaration, parser.root);
    auto expression_chain = element::build_expression_chain(context, ast, dummy_declaration.get(), result);
    dummy_declaration->body = std::move(expression_chain);
    root.children.clear();

    if (result != ELEMENT_OK)
    {
        context->log(result, fmt::format("building object model failed with element_result {}", result), info.file_name->data());
        return result;
    }

    context->global_scope->add_declaration(std::move(dummy_declaration));

    auto found_dummy_decl = context->global_scope->find(element::identifier{ "<REMOVE>" }, false);
    auto compiled = found_dummy_decl->compile(compilation_context, found_dummy_decl->source_info);

    //stuff from below
    (*evaluable)->evaluable = compiled;
}

element_result element_interpreter_evaluate_expression(
    element_interpreter_ctx* context,
    const element_evaluator_options* options,
    const char* expression_string,
    element_outputs* outputs)
{
    //sure there is a shorthand for this
    element_evaluable evaluable;
    auto* evaluable_ptr = &evaluable;
    element_interpreter_compile_expression(context, nullptr, expression_string, &evaluable_ptr);

    if (!evaluable_ptr->evaluable)
    {
        assert(!"tried to evaluate an expression that failed to compile");
        //todo: log
        outputs->count = 0;
        return ELEMENT_ERROR_UNKNOWN;
    }

    const auto err = std::dynamic_pointer_cast<element::error>(evaluable_ptr->evaluable);
    if (err)
    {
        outputs->count = 0;
        return err->log_once(context->logger.get());
    }

    const auto compiled = evaluable_ptr->evaluable->to_expression();
    if (!compiled)
    {
        assert(false);
        outputs->count = 0;
        return ELEMENT_ERROR_UNKNOWN;
    }

    float inputs[] = { 0 };
    element_inputs input;
    input.values = inputs;
    input.count = 1;

    const auto result = element_interpreter_evaluate(context, options, evaluable_ptr, &input, outputs);

    //todo: remove declaration added to global scope
    //todo: remove file_info added to interpreter source context

    return result;
}

element_result element_interpreter_typeof_expression(
    element_interpreter_ctx* context,
    const element_evaluator_options* options,
    const char* expression_string,
    char* buffer,
    const int buffer_size)
{
    //sure there is a shorthand for this
    element_evaluable evaluable;
    auto* evaluable_ptr = &evaluable;
    element_interpreter_compile_expression(context, nullptr, expression_string, &evaluable_ptr);

    if (!evaluable_ptr->evaluable)
    {
        assert(!"tried to evaluate an expression that failed to compile");
        //todo: log
        return ELEMENT_ERROR_UNKNOWN;
    }

    const auto err = std::dynamic_pointer_cast<element::error>(evaluable_ptr->evaluable);
    if (err)
    {
        return err->log_once(context->logger.get());
    }

    const auto typeof = evaluable.evaluable->typeof_info();
    if (buffer_size < static_cast<int>(typeof.size()))
        return ELEMENT_ERROR_UNKNOWN;

    #define _CRT_SECURE_NO_WARNINGS
    strncpy(buffer, typeof.c_str(), typeof.size());
    #undef _CRT_SECURE_NO_WARNINGS

    return ELEMENT_OK;
}

//element_result element_metainfo_for_evaluable(const element_evaluable* evaluable, element_metainfo** metainfo)
//{
//    //todo: error checking and stuff
//
//    *metainfo = new element_metainfo();
//    (*metainfo)->typeof = evaluable->evaluable->typeof_info();
//    (*metainfo)->code = evaluable->evaluable->to_code(0);
//
//    return ELEMENT_OK;
//}
//
//element_result element_metainfo_get_typeof(const element_metainfo* metainfo, char* buffer, const int buffer_size)
//{
//    if (buffer_size < static_cast<int>(metainfo->typeof.size()))
//        return ELEMENT_ERROR_UNKNOWN;
//
//    #define _CRT_SECURE_NO_WARNINGS
//    strncpy(buffer, metainfo->typeof.c_str(), metainfo->typeof.size());
//    #undef _CRT_SECURE_NO_WARNINGS
//
//    return ELEMENT_OK;
//}



#ifdef LEGACY_COMPILER

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
    case ELEMENT_AST_NODE_CONSTRAINT:
    case ELEMENT_AST_NODE_STRUCT:
    {
        assert(node->children.size() > ast_idx::function::declaration);
        element_ast* declnode = node->children[ast_idx::function::declaration].get();
        assert(declnode->type == ELEMENT_AST_NODE_DECLARATION);
        auto item = scope_new(parent, declnode->identifier, node);
        // inputs
        if (declnode->children.size() > ast_idx::declaration::inputs && declnode->children[ast_idx::declaration::inputs]->type == ELEMENT_AST_NODE_PORTLIST) {
            for (const auto& t : declnode->children[ast_idx::declaration::inputs]->children) {
                auto cptr = get_names(item.get(), t.get());
                item->children.emplace(cptr->name, std::move(cptr));
            }
        }
        // body
        if (node->children.size() > ast_idx::function::body) {
            if (node->children[ast_idx::function::body]->type == ELEMENT_AST_NODE_SCOPE) {
                for (const auto& t : node->children[ast_idx::function::body]->children) {
                    auto cptr = get_names(item.get(), t.get());
                    item->children.try_emplace(cptr->name, std::move(cptr));
                }
            }
        }
        // outputs
        if (declnode->children.size() > ast_idx::declaration::outputs) {
            element_ast* outputnode = declnode->children[ast_idx::declaration::outputs].get();
            // these should typically already exist from the body, so just try
            if (node->children.size() > ast_idx::function::body) {
                if (outputnode->type == ELEMENT_AST_NODE_PORTLIST) {
                    for (const auto& t : outputnode->children) {
                        auto cptr = get_names(item.get(), t.get());
                        item->children.try_emplace(cptr->name, std::move(cptr));
                    }
                }
                else if (outputnode->type == ELEMENT_AST_NODE_TYPENAME) {
                    auto cptr = scope_new(item.get(), "return", node->children[ast_idx::function::body].get());
                    item->children.try_emplace(cptr->name, std::move(cptr));
                }
                else if (outputnode->type == ELEMENT_AST_NODE_UNSPECIFIED_TYPE) {
                    // implied any return
                    auto cptr = scope_new(item.get(), "return", node->children[ast_idx::function::body].get());
                    item->children.try_emplace(cptr->name, std::move(cptr));
                }
                else {
                    assert(false);
                }
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
    }
    else {
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

#endif

#ifdef LEGACY_COMPILER

element_result element_interpreter_get_function(element_interpreter_ctx* context, const char* name, const element_function** function)
{
    assert(context);
    assert(name);
    if (!context->names) return ELEMENT_ERROR_NOT_FOUND;
    const element_scope* scope = context->names->lookup(name);
    if (scope && scope->function()) {
        *function = scope->function().get();
        return ELEMENT_OK;
    }
    else {

        auto result = ELEMENT_ERROR_NOT_FOUND;
        context->log(result, fmt::format("Cannot find {}", name), "<input>");
        return result;
    }
}

element_result element_interpreter_compile_function(
    element_interpreter_ctx* context,
    const element_function* function,
    element_compiled_function** compiled_function,
    const element_compiler_options* opts)
{
    assert(context);
    assert(function);
    assert(compiled_function);
    element_compiler_options options;
    if (opts)
        options = *opts;
    compilation compiled_result;
    ELEMENT_OK_OR_RETURN(element_compile(*context, function, compiled_result, options));
    *compiled_function = new element_compiled_function;
    (*compiled_function)->function = function;
    (*compiled_function)->expression = std::move(compiled_result.expression);
    (*compiled_function)->constraint = std::move(compiled_result.constraint);

    if (flag_set(logging_bitmask, log_flags::debug | log_flags::output_expression_tree))
        context->log("\n---------------\nEXPRESSION TREE\n---------------\n" + expression_to_string(*(*compiled_function)->expression));

    return ELEMENT_OK;
}

void element_interpreter_delete_compiled_function(element_compiled_function* compiled_function)
{
    delete compiled_function;
}

//HACK: This is horrible and temporary, find a better way to size the outputs
size_t get_outputs_size(
    element_interpreter_ctx* context,
    const element_compiled_function* compiled_function)
{
    assert(context);
    assert(compiled_function);
    assert(compiled_function->expression);

    if (compiled_function->expression->as<element_expression_structure>())
        return compiled_function->expression->dependents().size();

    return 1;
}

element_result element_interpreter_evaluate_function(
    element_interpreter_ctx* context,
    const element_compiled_function* compiled_function,
    const element_value* inputs, size_t inputs_count,
    element_value* outputs, size_t outputs_count,
    const element_evaluator_options* opts)
{
    assert(context);
    assert(compiled_function);
    element_evaluator_options options;
    if (opts)
        options = *opts;
    auto result = element_evaluate(*context, compiled_function->expression, inputs, inputs_count, outputs, outputs_count, options);
    if (result != ELEMENT_OK) {
        context->log(result, fmt::format("Failed to evaluate {}", compiled_function->function->name()), "<input>");
    }

    return result;
}

element_result element_interpreter_get_internal_typeof(
    element_interpreter_ctx* context,
    const char* string,
    const char* filename,
    char* output_string_buffer,
    unsigned int output_string_buffer_size)
{
    assert(context);
    assert(string);
    assert(filename);

    element_result result = ELEMENT_OK;

    if (context->trees.empty())
        return ELEMENT_ERROR_UNKNOWN;

    auto& global_scope = context->names;
    const auto found_scope = global_scope->lookup(string);

    std::string internal_typeof = "Unknown";

    if (found_scope)
    {
        //todo: if the function is a nullary binding to a constant, then the test suite is expecting it to be Num. I don't know if that's valid, seems special-cased
        if (found_scope->node->type == ELEMENT_AST_NODE_FUNCTION)
            internal_typeof = "Function";
        else if (found_scope->node->type == ELEMENT_AST_NODE_CONSTRAINT)
            internal_typeof = "Constraint";
        else if (found_scope->node->type == ELEMENT_AST_NODE_STRUCT)
            internal_typeof = "Type";
        else if (found_scope->node->type == ELEMENT_AST_NODE_NAMESPACE)
            internal_typeof = "Namespace";
    }

    //It's not something that exists in the AST
    if (internal_typeof == "Unknown")
    {
        try
        {
            //todo: create a tokeniser and check to see if it can parse it as a number. this is a hack
            std::stof(string);
            internal_typeof = "Num";
        }
        catch (...) {}
    }

    //It's not a number, so it could be a struct instance or something more complex, try compiling it and getting the resulting type
    //todo: this is relying on the lack of error checking for top-level functions having serializable types (ie. the return type is always known)
    //todo: this polutes the interpreter, do we need to make a new interpreter? can we delete what we've added?
    if (internal_typeof == "Unknown")
    {
        std::string eval_wrapper = "evaluate = " + std::string(string) + ";";
        result = context->load(eval_wrapper.c_str(), "totes_not_a_hack_upon_a_hack");
        if (result != ELEMENT_OK)
            return result;

        const element_function* fn;
        result = element_interpreter_get_function(context, "evaluate", &fn);
        if (result != ELEMENT_OK)
            return result;

        element_compiled_function* compiled_function;
        result = element_interpreter_compile_function(context, fn, &compiled_function, nullptr);
        if (result != ELEMENT_OK)
            return result;

        internal_typeof.resize(output_string_buffer_size);
        element_compiled_function_get_typeof_compilation(compiled_function, internal_typeof.data(), output_string_buffer_size);
    }

    //todo: safer C function?
    strncpy_s(output_string_buffer, output_string_buffer_size, internal_typeof.c_str(), output_string_buffer_size);

    //:(
    assert(internal_typeof != "Unknown");

    return result;
}

//todo: this is relying on the lack of error checking for top-level functions having serializable types (ie. the return type is always known)
element_result element_compiled_function_get_typeof_compilation(element_compiled_function* compiled_function, char* string_buffer, unsigned int string_buffer_size)
{
    if (string_buffer == nullptr)
        return ELEMENT_ERROR_UNKNOWN;

    std::string str = "Constraint";

    const auto anonymous_type = compiled_function->constraint->as<element_type_anonymous>();
    const auto named_type = compiled_function->constraint->as<element_type_named>();
    const auto type = compiled_function->constraint->as<element_type>();

    if (anonymous_type)
    {
        str = "Anonymous";
    }
    else if (named_type)
    {
        str = named_type->name();
    }
    else if (type) //todo: I'm not sure this is valid
    {
        if (type == element_type::binary.get()
            || type == element_type::unary.get())
        {
            str = "Function";
        }
        else if (type == element_type::num.get())
        {
            str = "Num"; //todo: num isn't Num in source, rather the numeral type?
        }
    }
    else
    {
        if (compiled_function->constraint == element_constraint::any)
        {
            str = "Any"; //todo: any isn't Any in source, rather the implicit type?
        }
        else if (compiled_function->constraint == element_constraint::function)
        {
            str = "Function";
        }
    }

    //todo: safer C function?
    strncpy_s(string_buffer, string_buffer_size, str.c_str(), string_buffer_size);
    return ELEMENT_OK;
}

#endif
