#include "interpreter_internal.hpp"

#include <algorithm>
#include <functional>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

#include <fmt/format.h>

#include "common_internal.hpp"
#include "etree/compiler.hpp"
#include "etree/evaluator.hpp"
#include "ast/ast_indexes.hpp"
#include "token_internal.hpp"

bool file_exists(const std::string& file)
{
    return std::filesystem::exists(file) && std::filesystem::is_regular_file(file);
}

bool directory_exists(const std::string& directory)
{
    return std::filesystem::exists(directory) && std::filesystem::is_directory(directory);
}


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
        assert(node->children.size() > ast_idx::fn::declaration);
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
        if (node->children.size() > ast_idx::fn::body) {
            if (node->children[ast_idx::fn::body]->type == ELEMENT_AST_NODE_SCOPE) {
                for (const auto& t : node->children[ast_idx::fn::body]->children) {
                    auto cptr = get_names(item.get(), t.get());
                    item->children.try_emplace(cptr->name, std::move(cptr));
                }
            }
        }
        // outputs
        if (declnode->children.size() > ast_idx::decl::outputs) {
            element_ast* outputnode = declnode->children[ast_idx::decl::outputs].get();
            // these should typically already exist from the body, so just try
            if (node->children.size() > ast_idx::fn::body) {
                if (outputnode->type == ELEMENT_AST_NODE_PORTLIST) {
                    for (const auto& t : outputnode->children) {
                        auto cptr = get_names(item.get(), t.get());
                        item->children.try_emplace(cptr->name, std::move(cptr));
                    }
                }
                else if (outputnode->type == ELEMENT_AST_NODE_TYPENAME) {
                    auto cptr = scope_new(item.get(), "return", node->children[ast_idx::fn::body].get());
                    item->children.try_emplace(cptr->name, std::move(cptr));
                }
                else if (outputnode->type == ELEMENT_AST_NODE_NONE) {
                    // implied any return
                    auto cptr = scope_new(item.get(), "return", node->children[ast_idx::fn::body].get());
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

element_result element_interpreter_ctx::TEMPORARY_LOG_MESSAGE(element_result result, std::string function, std::string str, std::string filename)
{
    if (result != ELEMENT_OK) 
    {
        std::stringstream ss;

        ss << "PARSE_ERROR" << std::endl;
        ss << "element_result: " << result << std::endl;
        ss << "function:" << function << std::endl;
        ss << "file: " << filename << std::endl;
        ss << "content: " << str << std::endl;

        log(9, ss.str());
    }
    return result;
}

element_result element_interpreter_ctx::load(const char* str, const char* filename)
{
    element_result result;
    element_tokeniser_ctx* raw_tctx;
    ELEMENT_OK_OR_RETURN(TEMPORARY_LOG_MESSAGE(element_tokeniser_create(&raw_tctx), "element_tokeniser_create", str, filename))

    //todo: not great having it here, but also don't want logging to be static.
    //logging should be on a specific object that the interpreter owns, but that the tokenizer has reference to
    raw_tctx->interpreter = this;

    // Make a smart pointer out of the tokeniser so it's deleted on an early return
    auto tctx = std::unique_ptr<element_tokeniser_ctx, decltype(&element_tokeniser_delete)>(raw_tctx, element_tokeniser_delete);

    ELEMENT_OK_OR_RETURN(TEMPORARY_LOG_MESSAGE(element_tokeniser_run(raw_tctx, str, filename), "element_tokeniser_run", str, filename))
    if (raw_tctx->tokens.empty())
        return ELEMENT_OK;

    element_ast* raw_ast = NULL;
    ELEMENT_OK_OR_RETURN(TEMPORARY_LOG_MESSAGE(element_ast_build(raw_tctx, &raw_ast), "element_ast_build", str, filename))

    // element_ast_print(raw_ast);
    auto ast = ast_unique_ptr(raw_ast, element_ast_delete);
    scope_unique_ptr root = get_names(nullptr, raw_ast);
    ELEMENT_OK_OR_RETURN(TEMPORARY_LOG_MESSAGE(add_ast_names(ast_names, root.get()), "add_ast_names", str, filename))
    ELEMENT_OK_OR_RETURN(TEMPORARY_LOG_MESSAGE(merge_names(names, std::move(root), nullptr), "merge_names", str, filename))

    trees.push_back(std::make_pair(filename, std::move(ast)));

    // TODO: HACK
    update_scopes(names.get());

    return ELEMENT_OK;
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

    element_result result = load(buffer.c_str(), file.c_str());
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

        element_result result = load_file(filename);
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

        element_result result = load_package(package);
        if (result != ELEMENT_OK && ret != ELEMENT_OK) //todo: only returns first error
            ret = result;
    }

    return ret;
}

element_result element_interpreter_ctx::load_prelude()
{
    if (prelude_loaded)
        return ELEMENT_ERROR_PRELUDE_ALREADY_LOADED;

    element_result result = load_package("Prelude");
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

element_result element_interpreter_ctx::set_log_callback(void (*callback)(const element_log_message* const))
{
    log_callback = callback;
    return ELEMENT_OK;
}

void element_interpreter_ctx::log(element_result code, const std::string& message)
{
    assert(log_callback);

    auto log = element_log_message();
    log.message = message.c_str();
    log.message_code = code;
    log.line = -1;
    log.column = -1;
    log.length = -1;
    //log.related_log_message = nullptr;

    log_callback(&log);
}

void element_interpreter_ctx::log(const element_log_message& message)
{
    assert(log_callback);
    log_callback(&message);
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

    return ELEMENT_OK;
}

element_result element_interpreter_ctx::print_ast(const std::string& name)
{

    auto it = std::find_if(trees.begin(), trees.end(),
        [&name](const std::pair<std::string, ast_unique_ptr>& element) { return element.first == name; });

    if (it == trees.end()) {
        return ELEMENT_ERROR_NOT_FOUND;
    }

    element_ast* ast = it->second.get();
    element_ast_print(ast);
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

element_result element_interpreter_load_file(element_interpreter_ctx* ctx, const char* file)
{
    assert(ctx);
    return ctx->load_file(file);
}

element_result element_interpreter_load_files(element_interpreter_ctx* ctx, const char** files, int files_count)
{
    assert(ctx);

    std::vector<std::string> actual_files;
    for (int i = 0; i < files_count; ++i) {
        //std::cout << fmt::format("load_file {}\n", files[i]); //todo: proper logging
        actual_files.push_back(files[i]);
    }

    return ctx->load_files(actual_files);
}


element_result element_interpreter_load_package(element_interpreter_ctx* ctx, const char* package)
{
    assert(ctx);
    //std::cout << fmt::format("load_package {}\n", package); //todo: proper logging
    return ctx->load_package(package);
}

element_result element_interpreter_load_packages(element_interpreter_ctx* ctx, const char** packages, int packages_count)
{
    assert(ctx);

    std::vector<std::string> actual_packages;
    for (int i = 0; i < packages_count; ++i) {
        //std::cout << fmt::format("load_packages {}\n", packages[i]); //todo: proper logging
        actual_packages.push_back(packages[i]);
    }

    return ctx->load_packages(actual_packages);
}

element_result element_interpreter_load_prelude(element_interpreter_ctx* ctx)
{
    assert(ctx);
    return ctx->load_prelude();
}

element_result element_interpreter_set_log_callback(element_interpreter_ctx* ctx, void (*log_callback)(const element_log_message* const))
{
    assert(ctx);
    return ctx->set_log_callback(log_callback);
}

element_result element_interpreter_clear(element_interpreter_ctx* ctx)
{
    assert(ctx);
    return ctx->clear();
}

element_result element_interpreter_print_ast(element_interpreter_ctx* ctx, const char* name)
{
    assert(ctx);
    return ctx->print_ast(name);
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
