#pragma once
#include "shared/geom/vec4.h"
#include "shared/geom/vec.h"
#include "shared/geom/ivec4.h"
#include "shared/geom/ivec.h"
#include <string>
#include <map>
#include <functional>
#include <string>
#include <variant>
#include <numeric>


namespace detail
{
    struct RecursiveVariant;
    using CoreVariant = std::variant<std::monostate, std::string, float, int, bool, vec4, vec, ivec4, ivec, std::vector<RecursiveVariant>>;
    struct RecursiveVariant : CoreVariant
    {
        RecursiveVariant() = default;
        virtual ~RecursiveVariant() = default;
        RecursiveVariant(const RecursiveVariant& c) = default;
        RecursiveVariant(RecursiveVariant&& c) = default;
        RecursiveVariant& operator=(const RecursiveVariant& c) = default;
        RecursiveVariant& operator=(RecursiveVariant&& c) = default;
        RecursiveVariant(const std::monostate& c) : CoreVariant(c) {}
        RecursiveVariant(const std::string& c) : CoreVariant(c) {}
        RecursiveVariant(const float& c) : CoreVariant(c) {}
        RecursiveVariant(const int& c) : CoreVariant(c) {}
        RecursiveVariant(const bool& c) : CoreVariant(c) {}
        RecursiveVariant(const vec4& c) : CoreVariant(c) {}
        RecursiveVariant(const vec& c) : CoreVariant(c) {}
        RecursiveVariant(const ivec4& c) : CoreVariant(c) {}
        RecursiveVariant(const ivec& c) : CoreVariant(c) {}
        RecursiveVariant(const std::vector<RecursiveVariant>& c) : CoreVariant(c) {}
    };
}

#if __cpp_lib_variant == 201606L
namespace std
{
    template<>
    struct variant_size<detail::RecursiveVariant> : variant_size<detail::CoreVariant> {
    };

    template<std::size_t I>
    struct variant_alternative<I, detail::RecursiveVariant> :  variant_alternative<I, detail::CoreVariant> {
    };
}
#endif

using Attribute_T = detail::RecursiveVariant;
using AttributeRow_T = std::vector<detail::RecursiveVariant>;
using AttributeList_T = std::vector<AttributeRow_T>;
using AttributeTree_T = std::vector<AttributeList_T>;

template <typename TargetType>
struct AttributeVisitCoercer
{
    TargetType operator()(const std::monostate& value) const;
    TargetType operator()(const std::string& value) const;
    TargetType operator()(float value) const;
    TargetType operator()(int value) const;
    TargetType operator()(bool value) const;
    TargetType operator()(const vec4& value) const;
    TargetType operator()(const vec& value) const;
    TargetType operator()(const ivec4& value) const;
    TargetType operator()(const ivec& value) const;
    TargetType operator()(const AttributeRow_T& value) const
    {
        for (const auto& element : value)
        {
            if (std::holds_alternative<TargetType>(element))
            {
                return std::get<TargetType>(element);
            }
        }

        return TargetType();
    }
};

template <> std::string AttributeVisitCoercer<std::string>::operator()(const AttributeRow_T& value) const
{
    using std::literals::string_literals::operator""s;
    static constexpr auto DELIMITER = ", ";
    static constexpr auto ARRAY_OPEN = "[";
    static constexpr auto ARRAY_CLOSE = "]";

    return ""s + ARRAY_OPEN + ARRAY_OPEN + std::accumulate(
        value.begin(), value.end(),
        ""s,
        [](const std::string& head, const Attribute_T& attr) -> std::string
        {
            std::string innerString = std::visit(AttributeVisitCoercer<std::string>(), attr);
            if (!head.empty())
                return head + ARRAY_CLOSE + DELIMITER + "["s + innerString;
            return innerString;
        }
    ) + ARRAY_CLOSE + ARRAY_CLOSE;
}