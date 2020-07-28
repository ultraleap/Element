#pragma once

//SELF
#include "type_annotation.hpp"

namespace element
{
    class port final
    {
    public:
        explicit port(identifier name, std::unique_ptr<type_annotation> annotation);

        [[nodiscard]] bool has_annotation() const { return annotation != nullptr; };

        [[nodiscard]] std::string typeof_info() const;
        [[nodiscard]] std::string to_code(int depth) const;

        [[nodiscard]] const std::string& get_name() const { return name.value;  };
        [[nodiscard]] const type_annotation* get_annotation() const { return annotation.get(); };

    private:
        identifier name;
        std::unique_ptr<type_annotation> annotation;
    };
}
