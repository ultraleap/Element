#pragma once

//SELF
#include "type_annotation.hpp"

namespace element
{
    class port final
    {
    public:
        explicit port(const declaration* declarer, identifier name, std::unique_ptr<type_annotation> annotation);

        [[nodiscard]] bool has_annotation() const { return annotation != nullptr; };

        [[nodiscard]] std::string typeof_info() const;
        [[nodiscard]] std::string to_code(int depth) const;

        [[nodiscard]] const std::string& get_name() const { return name.value;  };
        [[nodiscard]] const type_annotation* get_annotation() const { return annotation.get(); };

        [[nodiscard]] const declaration* resolve_annotation(const compilation_context& context) const;
        [[nodiscard]] object_const_shared_ptr generate_placeholder(const compilation_context& context, int& placeholder_index) const;

    private:
        const declaration* declarer;
        identifier name;
        std::unique_ptr<type_annotation> annotation;
    };
}
