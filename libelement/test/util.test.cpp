#include "util.test.hpp"

#include <vector>
#include <catch2/catch.hpp>

void log_callback(const element_log_message* msg, void* user_data)
{
    //TODO: This is a bit of a hack for now... Setting a constant length instead and using for both buffers
    const auto space = 512;
    char buffer[space];
    buffer[0] = '^';
    buffer[1] = '\0';
    const char* buffer_str = nullptr;
    if (msg->character - 1 >= 0) {
        const auto padding_count = msg->character - 1;
        for (auto i = 0; i < padding_count; ++i) {
            buffer[i] = ' ';
        }

        const auto end = padding_count + msg->length;
        for (auto i = padding_count; i < end; ++i) {
            buffer[i] = '^';
        }

        buffer[end] = '\0';

        buffer_str = &buffer[0];
    }

    std::vector<char> output_buffer_array;
    output_buffer_array.resize(msg->message_length + 4 * space);
    auto* const output_buffer = output_buffer_array.data();

    sprintf(output_buffer, "\n----------ELE%d %s\n%d| %s\n%d| %s\n\n%s\n----------\n\n",
        msg->message_code,
        msg->filename,
        msg->line,
        msg->line_in_source ? msg->line_in_source : "",
        msg->line,
        buffer_str,
        msg->message);

    printf("%s", output_buffer);
    UNSCOPED_INFO(output_buffer);
}
