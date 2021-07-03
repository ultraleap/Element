#pragma once

//STD
#include <string>
#include <cassert>

namespace element
{
class source_information
{
public:
    source_information() = default;

    source_information(int line, int character_start, int character_end, const std::string* line_in_source, const char* filename)
        : line(line)
        , character_start(character_start)
        , character_end(character_end)
        , line_in_source(line_in_source)
        , filename(filename)
    {}

    const std::string& get_text() const
    {
        if (text.empty()) {
            //todo: UTF8 concerns?
            assert(character_end - character_start >= 0);
            assert(character_start > 0);
            text = line_in_source->substr(static_cast<std::size_t>(character_start) - 1, static_cast<std::size_t>(character_end) - character_start);
        }

        return text;
    }

    int line = 0;
    int character_start = 0;
    int character_end = 0;

    const std::string* line_in_source = nullptr;
    const char* filename = nullptr;

private:
    mutable std::string text;
};
} // namespace element