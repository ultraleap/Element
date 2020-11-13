#include "token_internal.hpp"

//STD
#include <cassert>
#include "utf8.h"

//LIBS
#include <fmt/format.h>

//SELF
#include "interpreter_internal.hpp"

//#define INCREMENT_TOKEN_LEN(s) { ++((s)->pos); ++((s)->character); ++((s)->cur_token.tok_len); }

// #define UTF8_UNCHECKED
#if defined(UTF8_UNCHECKED)
#define UTF8_PEEK_NEXT(it, end) utf8::unchecked::peek_next(it)
#define UTF8_NEXT(it, end) utf8::unchecked::next(it)
#define UTF8_ADVANCE(it, n, end) utf8::unchecked::advance(it, n)
#else
#define UTF8_PEEK_NEXT(it, end) utf8::peek_next(it, end)
#define UTF8_NEXT(it, end) utf8::next(it, end)
#define UTF8_ADVANCE(it, n, end) utf8::advance(it, n, end)
#endif

#define PRINTCASE(a) \
    case a:          \
        c = #a;      \
        break;

// replacements for C stdlib functions which rely on slow locale stuff
static bool element_isalpha(uint32_t c) { return unsigned((c & (~(1 << 5))) - 'A') <= 'Z' - 'A'; }
static bool element_isdigit(uint32_t c) { return unsigned(c - '0') <= '9' - '0'; }
static bool element_isalnum(uint32_t c) { return element_isalpha(c) || element_isdigit(c); }
static bool element_isspace(uint32_t c) { return c == ' ' || (c >= 0x09 && c <= 0x0D); }
static bool element_iseol(uint32_t c) { return c == '\n'; }
static bool isid_alpha(uint32_t c) { return element_isalpha(c) || c == '_' || (c >= 0x00F0 && c <= 0xFFFF); }
static bool isid_alnum(uint32_t c) { return element_isalnum(c) || (c >= 0x00F0 && c <= 0xFFFF); }

void advance_to_end_of_line(std::string::const_iterator& it, const std::string::const_iterator& end)
{
    try
    {
        do
        {
            UTF8_NEXT(it, end);
        } while (it != end && !element_iseol(UTF8_PEEK_NEXT(it, end)));
    }
    catch (...)
    {
        //exceptions are thrown for just about any utf issue, ignore them
    }
}

element_tokeniser_ctx::element_tokeniser_ctx()
{
    tokens.reserve(512);
    cur_token.type = ELEMENT_TOK_NONE;
    cur_token.post_pos = -1;
    cur_token.post_len = -1;
    cur_token.tok_pos = -1;
    cur_token.tok_len = -1;
    cur_token.pre_pos = -1;
    cur_token.pre_len = -1;
}

void element_tokeniser_ctx::reset_token()
{
    if (cur_token.type != ELEMENT_TOK_NONE)
    {
        // save current token
        tokens.push_back(cur_token);
        cur_token.pre_pos = -1;
        cur_token.pre_len = 0;
    }

    cur_token.type = ELEMENT_TOK_NONE;
    cur_token.tok_pos = -1;
    cur_token.tok_len = 0;
    cur_token.post_pos = -1;
    cur_token.post_len = 0;

    cur_token.line = -1;
    cur_token.character = -1;
    cur_token.line_start_position = -1;
    cur_token.file_name = raw_filename;
}

element_result element_tokeniser_ctx::run(const char* cinput, const char* cfilename)
{
    raw_filename = cfilename;
    filename = cfilename;
    input = cinput;
    pos = 0;
    line = 1;
    character = 1;
    reset_token();

    try
    {
        auto it = input.begin();
        auto end = input.end();
        uint32_t c;
        while (it != end)
        {
            c = UTF8_PEEK_NEXT(it, end);
            switch (c)
            {
            case '.':
                add_token(ELEMENT_TOK_DOT, 1);
                UTF8_NEXT(it, end);
                break;
            case '(':
                add_token(ELEMENT_TOK_BRACKETL, 1);
                UTF8_NEXT(it, end);
                break;
            case ')':
                add_token(ELEMENT_TOK_BRACKETR, 1);
                UTF8_NEXT(it, end);
                break;
            case ',':
                add_token(ELEMENT_TOK_COMMA, 1);
                UTF8_NEXT(it, end);
                break;
            case ':':
                add_token(ELEMENT_TOK_COLON, 1);
                UTF8_NEXT(it, end);
                break;
            case '{':
                add_token(ELEMENT_TOK_BRACEL, 1);
                UTF8_NEXT(it, end);
                break;
            case '}':
                add_token(ELEMENT_TOK_BRACER, 1);
                UTF8_NEXT(it, end);
                break;
            case '=':
                add_token(ELEMENT_TOK_EQUALS, 1);
                UTF8_NEXT(it, end);
                break;
            case '#':
                tokenise_comment(it, end);
                break;
            case '_':
            {
                const auto next = UTF8_PEEK_NEXT(it + 1, end);
                if (isid_alpha(next))
                {
                    const auto begin_it = it;
                    const auto result = tokenise_identifier(it, end);
                    if (result != ELEMENT_OK)
                        return log(result, fmt::format("Failed to parse identifier '{}' following '_'.", std::string(begin_it, it)));
                }
                else
                {
                    add_token(ELEMENT_TOK_UNDERSCORE, 1);
                    UTF8_NEXT(it, end);
                }
                break;
            }
            default:
            {
                if (isid_alpha(c))
                {
                    const auto begin_it = it;
                    const auto result = tokenise_identifier(it, end);
                    if (result != ELEMENT_OK)
                        return log(result, fmt::format("Failed to parse identifier '{}'", std::string(begin_it, it)));
                }
                else if (element_isdigit(c) || c == '-' || c == '+')
                {
                    const auto begin_it = it;
                    const auto result = tokenise_number(it, end);
                    if (result != ELEMENT_OK)
                        return log(result, fmt::format("Failed to parse number '{}'", std::string(begin_it, it)));
                }
                else if (element_iseol(c))
                {
                    ++line;
                    character = 1;
                    pos += 1;
                    line_start_position = pos;
                    line_number_to_line_pos.push_back(line_start_position);
                    reset_token();
                    UTF8_NEXT(it, end);
                }
                else if (element_isspace(c))
                {
                    //just eat it!
                    pos += 1;
                    character += 1;
                    UTF8_NEXT(it, end);
                }
                else
                {
                    const auto begin_it = it;
                    UTF8_NEXT(it, end);
                    std::string source_line;
                    source_line.resize(1024);
                    assert(pos - line_start_position >= 0);
                    assert(pos >= 0);
                    memcpy(source_line.data(), cinput + line_start_position, static_cast<std::size_t>(pos) - line_start_position);
                    return log(ELEMENT_ERROR_PARSE,
                               fmt::format("Encountered invalid character '{}' in file {} on line {} character {}\n{}",
                                           std::string(begin_it, it), cfilename, line, character, source_line));
                }
            }
            }
        }
        add_token(ELEMENT_TOK_EOF, 0);
        return ELEMENT_OK;
    }
    catch (const std::exception& e)
    {
        return log(ELEMENT_ERROR_EXCEPTION,
                   fmt::format("Exception occured: {}", e.what()));
    }
    catch (...) //potentially EOF when last source character is UTF?
    {
        return log(ELEMENT_ERROR_EXCEPTION,
                   fmt::format("Exception occured"));
    }
}

std::string element_tokeniser_ctx::text(const element_token* t) const
{
    return input.substr(t->tok_pos, t->tok_len);
}

element_result element_tokeniser_ctx::tokenise_number(std::string::iterator& it, const std::string::iterator& end)
{
    //TODO: Go through this in detail, also, waaaaaaaaaaaaaaaaaaaaay too long for parsing a number
    assert(cur_token.type == ELEMENT_TOK_NONE);
    cur_token.type = ELEMENT_TOK_NUMBER;
    cur_token.tok_pos = pos;
    cur_token.line = line;
    cur_token.line_start_position = line_start_position;
    cur_token.character = character;

    const auto it_begin = it;
    uint32_t c = UTF8_PEEK_NEXT(it, end);
    if (c == '-' || c == '+')
    {
        UTF8_NEXT(it, end);
        c = UTF8_PEEK_NEXT(it, end);
    }

    if (!element_isdigit((c)))
    {
        return ELEMENT_ERROR_PARSE;
    }

    while (it != end && element_isdigit(UTF8_PEEK_NEXT(it, end)))
        c = UTF8_NEXT(it, end);

    if (it == end)
    {
        const size_t len = std::distance(it_begin, it);
        pos += (int)len;
        character += (int)len;
        cur_token.tok_len += (int)len;
        reset_token();
        return ELEMENT_OK;
    }

    c = UTF8_PEEK_NEXT(it, end);
    if (c == '.')
    {
        auto it_next = it;
        UTF8_NEXT(it_next, end);

        auto c_next = UTF8_PEEK_NEXT(it_next, end);
        if (element_isdigit(c_next))
        {
            // number
            UTF8_NEXT(it, end);
            while (it != end && element_isdigit(UTF8_PEEK_NEXT(it, end)))
                c = UTF8_NEXT(it, end);
        }
        else if (element_isalpha(c_next))
        {
            // indexing into a literal, do nothing and let the cleanup back out
        }
        else
        {
            const auto it_rhs_start = it_next;
            UTF8_NEXT(it_next, end);
            
            return log(ELEMENT_ERROR_BAD_INDEX_INTO_NUMBER,
                       fmt::format("Found {} which was thought to be a number being indexed, "
                                   "but encountered invalid character '{}' on the right hand side of '.'",
                                   std::string(it_begin, it), std::string(it_rhs_start, it_next)));
        }
    }

    if (it != end)
    {
        c = UTF8_PEEK_NEXT(it, end);
        if (c == 'e' || c == 'E')
        {
            auto it_prev_character = it;
            UTF8_NEXT(it, end);
            c = UTF8_PEEK_NEXT(it, end);

            if (c == '-' || c == '+')
            {
                it_prev_character = it;
                UTF8_NEXT(it, end);
                c = UTF8_PEEK_NEXT(it, end);
            }

            if (!element_isdigit(c))
                return log(ELEMENT_ERROR_BAD_NUMBER_EXPONENT,
                           fmt::format("Found {} which was thought to be a number in scientific notation, "
                                       "but encountered invalid character '{}' instead of the exponent number",
                                       std::string(it_begin, it), std::string(it_prev_character, it)));

            while (it != end && element_isdigit(UTF8_PEEK_NEXT(it, end)))
                c = UTF8_NEXT(it, end);
        }
    }

    // determine length in bytes
    const size_t len = std::distance(it_begin, it);
    pos += (int)len;
    character += (int)len;
    cur_token.tok_len += (int)len;
    reset_token();
    return ELEMENT_OK;
}

element_result element_tokeniser_ctx::tokenise_comment(std::string::iterator& it, const std::string::iterator& end)
{
    //TODO: Go through this in detail
    if (cur_token.post_pos < 0)
        cur_token.post_pos = pos;

    //consume all characters until end of file or end of line
    const auto it_before = it;
    try
    {
        //will throw at EOF
        while (it != end && !element_iseol(UTF8_PEEK_NEXT(it, end)))
            UTF8_NEXT(it, end);
    }
    catch (...)
    {
        //we probably hit EOF. not the nicest way of handling this issue
    }

    // calculate correct length
    const size_t len = std::distance(it_before, it);
    cur_token.post_len += (int)len;
    pos += (int)len;
    character += (int)len;

    return ELEMENT_OK;
}

element_result element_tokeniser_ctx::tokenise_identifier(std::string::iterator& it, const std::string::iterator& end)
{
    //TODO: Go through this in detail
    assert(cur_token.type == ELEMENT_TOK_NONE);
    cur_token.type = ELEMENT_TOK_IDENTIFIER;
    cur_token.tok_pos = pos;
    cur_token.line = line;
    cur_token.line_start_position = line_start_position;
    cur_token.character = character;
    const auto it_begin = it;
    uint32_t c = UTF8_PEEK_NEXT(it, end);
    if (c == '_')
    {
        c = UTF8_NEXT(it, end);
    }

    assert(isid_alpha(c));

    while (it != end
           && (isid_alnum(UTF8_PEEK_NEXT(it, end)) || UTF8_PEEK_NEXT(it, end) == '_'))
    {
        c = UTF8_NEXT(it, end);
    }

    // determine length in bytes
    const size_t len = std::distance(it_begin, it);
    pos += (int)len;
    character += (int)len;
    cur_token.tok_len += (int)len;
    reset_token();
    return ELEMENT_OK;
}

void element_tokeniser_ctx::add_token(element_token_type t, int n)
{
    assert(cur_token.type == ELEMENT_TOK_NONE);
    cur_token.type = t;
    cur_token.tok_pos = pos;
    cur_token.tok_len = n;
    cur_token.line = line;
    cur_token.line_start_position = line_start_position;
    cur_token.character = character;
    pos += n;
    character += n;
    reset_token();
}

std::string element_tokeniser_ctx::text_on_line(int line) const
{
    //lines start at 1, arrays at 0
    line--;

    if (line < 0 || line > line_number_to_line_pos.size())
        return "invalid line";

    const auto start_pos = line_number_to_line_pos[line];
    const auto start_it = input.begin() + start_pos;
    auto end_it = start_it;
    advance_to_end_of_line(end_it, input.end());

    return std::string(start_it, end_it);
}

void element_tokeniser_ctx::clear()
{
    tokens.clear();
    filename.clear();
    input.clear();
    line = 1;
    line_start_position = 0;
    character = 1;
    pos = 0;
    reset_token();
}

element_result element_tokeniser_ctx::log(element_result message_code, const std::string& message) const
{
    if (logger == nullptr)
        return message_code;

    logger->log(*this, message_code, message);
    return message_code;
}

element_result element_tokeniser_ctx::log(element_result message_code, const std::string& message, int length, element_log_message* related_message) const
{
    if (logger == nullptr)
        return message_code;

    logger->log(*this, message_code, message, length, related_message);
    return message_code;
}

element_result element_tokeniser_ctx::log(const std::string& message) const
{
    if (logger == nullptr)
        return ELEMENT_OK;

    logger->log(message, element_stage::ELEMENT_STAGE_MISC);
    return ELEMENT_OK;
}

void element_tokeniser_ctx::set_log_callback(LogCallback callback, void* user_data)
{
    logger = std::make_shared<element_log_ctx>();
    logger->callback = callback;
    logger->user_data = user_data;
}

element_token* element_tokeniser_ctx::get_token(std::size_t token_index, element_result& out_result)
{
    if (token_index >= tokens.size())
    {
        out_result = ELEMENT_ERROR_ACCESSED_TOKEN_PAST_END;
        return nullptr;
    }

    return &tokens[token_index];
}
