#include "Attribute.h"
#include <numeric>

using std::literals::string_literals::operator""s;
static constexpr auto DELIMITER = ", ";
static constexpr auto ARRAY_OPEN = "[";
static constexpr auto ARRAY_CLOSE = "]";

template <> std::string AttributeVisitCoercer<std::string>::operator()(const std::monostate& value) const
{
	return "";
}

template <> std::string AttributeVisitCoercer<std::string>::operator()(const std::string& value) const
{
	return value;
}

template <> std::string AttributeVisitCoercer<std::string>::operator()(const float value) const
{
	return std::to_string(value);
}

template <> std::string AttributeVisitCoercer<std::string>::operator()(int value) const
{
	return std::to_string(value);
}

template <> std::string AttributeVisitCoercer<std::string>::operator()(bool value) const
{
	return std::to_string(value);
}

template <> std::string AttributeVisitCoercer<std::string>::operator()(const vec4& value) const
{
	return std::to_string(value.x) + DELIMITER + std::to_string(value.y) + DELIMITER + std::to_string(value.z) + DELIMITER + std::to_string(value.w);
}

template <> std::string AttributeVisitCoercer<std::string>::operator()(const vec& value) const
{
	return std::to_string(value.x) + DELIMITER + std::to_string(value.y) + DELIMITER + std::to_string(value.z);
}

template <> std::string AttributeVisitCoercer<std::string>::operator()(const ivec4& value) const
{
	return std::to_string(value.x) + DELIMITER + std::to_string(value.y) + DELIMITER + std::to_string(value.z) + DELIMITER + std::to_string(value.w);
}

template <> std::string AttributeVisitCoercer<std::string>::operator()(const ivec& value) const
{
	return std::to_string(value.x) + DELIMITER + std::to_string(value.y) + DELIMITER + std::to_string(value.z);
}

template <> std::string AttributeVisitCoercer<std::string>::operator()(const AttributeRow_T& value) const
{
    return ""s + ARRAY_OPEN + ARRAY_OPEN + std::accumulate(
        value.begin(), value.end(),
        ""s,
        [](const std::string& head, const Attribute_T& attr) -> std::string
        {
            if (!head.empty())
                return head + ARRAY_CLOSE + DELIMITER + "["s + std::visit(AttributeVisitCoercer<std::string>(), attr);
            return std::visit(AttributeVisitCoercer<std::string>(), attr);
        }
    ) + ARRAY_CLOSE + ARRAY_CLOSE;
}


template <> float AttributeVisitCoercer<float>::operator()(const std::monostate& value) const
{
	return 0.0f;
}

template <> float AttributeVisitCoercer<float>::operator()(const std::string& value) const
{
    if (value.empty())
        return 0.0f;
	return std::stof(value);
}

template <> float AttributeVisitCoercer<float>::operator()(const float value) const
{
	return value;
}

template <> float AttributeVisitCoercer<float>::operator()(const int value) const
{
	return round(value);
}

template <> float AttributeVisitCoercer<float>::operator()(const bool value) const
{
	return value ? 1.0f : 0.0f;
}

template <> float AttributeVisitCoercer<float>::operator()(const vec4& value) const
{
	return value.x;
}

template <> float AttributeVisitCoercer<float>::operator()(const vec& value) const
{
	return value.x;
}

template <> float AttributeVisitCoercer<float>::operator()(const ivec4& value) const
{
	return value.x;
}

template <> float AttributeVisitCoercer<float>::operator()(const ivec& value) const
{
	return value.x;
}


template <> int AttributeVisitCoercer<int>::operator()(const std::monostate& value) const
{
	return 0;
}

template <> int AttributeVisitCoercer<int>::operator()(std::string const& value) const
{
    if (value.empty())
        return 0;
	return std::stoi(value);
}

template <> int AttributeVisitCoercer<int>::operator()(const float value) const
{
	return value;
}

template <> int AttributeVisitCoercer<int>::operator()(int value) const
{
	return value;
}

template <> int AttributeVisitCoercer<int>::operator()(bool value) const
{
	return value ? 1 : 0;
}

template <> int AttributeVisitCoercer<int>::operator()(const vec4& value) const
{
	return value.x;
}

template <> int AttributeVisitCoercer<int>::operator()(const vec& value) const
{
	return value.x;
}

template <> int AttributeVisitCoercer<int>::operator()(const ivec4& value) const
{
	return value.x;
}

template <> int AttributeVisitCoercer<int>::operator()(const ivec& value) const
{
	return value.x;
}


template <> bool AttributeVisitCoercer<bool>::operator()(const std::monostate& value) const
{
	return false;
}

template <> bool AttributeVisitCoercer<bool>::operator()(const std::string& value) const
{
    if (value.empty())
        return false;
	return value == "true" || value == "1" || value == "True" || value == "TRUE" || std::stoi(value) != 0 || std::stof(value) != 0.0f ? true : false;
}

template <> bool AttributeVisitCoercer<bool>::operator()(const float value) const
{
	return value != 0.0f ? true : false;
}

template <> bool AttributeVisitCoercer<bool>::operator()(const int value) const
{
	return value != 0 ? true : false;
}

template <> bool AttributeVisitCoercer<bool>::operator()(const bool value) const
{
	return value;
}

template <> bool AttributeVisitCoercer<bool>::operator()(const vec4& value) const
{
	return value.x != 0 ? true : false;
}

template <> bool AttributeVisitCoercer<bool>::operator()(const vec& value) const
{
	return value.x != 0 ? true : false;
}

template <> bool AttributeVisitCoercer<bool>::operator()(const ivec4& value) const
{
	return value.x != 0 ? true : false;
}

template <> bool AttributeVisitCoercer<bool>::operator()(const ivec& value) const
{
	return value.x != 0 ? true : false;
}


template <> vec4 AttributeVisitCoercer<vec4>::operator()(const std::monostate& value) const
{
	return vec4(0.0f, 0.0f, 0.0f, 0.0f);
}

template <> vec4 AttributeVisitCoercer<vec4>::operator()(const std::string& value) const
{
    //FIXME: implement tokenization + number extraction from string
	return vec4(0.0f, 0.0f, 0.0f, 0.0f);
}

template <> vec4 AttributeVisitCoercer<vec4>::operator()(const float value) const
{
	return vec4(value, 0.0f, 0.0f, 0.0f);
}

template <> vec4 AttributeVisitCoercer<vec4>::operator()(const int value) const
{
	return vec4(value, 0.0f, 0.0f, 0.0f);
}

template <> vec4 AttributeVisitCoercer<vec4>::operator()(const bool value) const
{
	return value ? vec4(1.0f, 1.0f, 1.0f, 1.0f) : vec4(0.0f, 0.0f, 0.0f, 0.0f);
}

template <> vec4 AttributeVisitCoercer<vec4>::operator()(const vec4& value) const
{
	return value;
}

template <> vec4 AttributeVisitCoercer<vec4>::operator()(const vec& value) const
{
	return vec4(value, 0.0f);
}

template <> vec4 AttributeVisitCoercer<vec4>::operator()(const ivec4& value) const
{
	return vec4(value.x, value.y, value.z, value.w);
}

template <> vec4 AttributeVisitCoercer<vec4>::operator()(const ivec& value) const
{
	return vec4(value.x, value.y, value.z, 0.0f);
}

//TODO: ? check for vec4 in AttributeRow_T, check for 4 floats, check for 4 ints, check for 4 bools (convert to int), check for string (convertable to int/float/bool)
//template <> vec4 AttributeVisitCoercer<vec4>::operator()(const AttributeRow_T& value) const
//{}


template <> vec AttributeVisitCoercer<vec>::operator()(const std::monostate& value) const
{
	return vec(0.0f, 0.0f, 0.0f);
}

template <> vec AttributeVisitCoercer<vec>::operator()(const std::string& value) const
{
    //FIXME: implement tokenization + number extraction from string
	return vec(0.0f, 0.0f, 0.0f);
}

template <> vec AttributeVisitCoercer<vec>::operator()(const float value) const
{
	return vec(value, 0.0f, 0.0f);
}

template <> vec AttributeVisitCoercer<vec>::operator()(const int value) const
{
	return vec(value, 0.0f, 0.0f);
}

template <> vec AttributeVisitCoercer<vec>::operator()(const bool value) const
{
	return value ? vec(1.0f, 1.0f, 1.0f) : vec(0.0f, 0.0f, 0.0f);
}

template <> vec AttributeVisitCoercer<vec>::operator()(const vec4& value) const
{
	return vec(value.x, value.y, value.z);
}

template <> vec AttributeVisitCoercer<vec>::operator()(const vec& value) const
{
	return value;
}

template <> vec AttributeVisitCoercer<vec>::operator()(const ivec4& value) const
{
	return vec(value.x, value.y, value.z);
}

template <> vec AttributeVisitCoercer<vec>::operator()(const ivec& value) const
{
	return vec(value.x, value.y, value.z);
}

//TODO: ? check for vec in AttributeRow_T, check for 3 floats, check for 3 ints, check for 3 bools (convert to int), check for string (convertable to int/float/bool)
//template <> vec AttributeVisitCoercer<vec>::operator()(const AttributeRow_T& value) const
//{}



template <> ivec4 AttributeVisitCoercer<ivec4>::operator()(const std::monostate& value) const
{
	return ivec4(0, 0, 0, 0);
}

template <> ivec4 AttributeVisitCoercer<ivec4>::operator()(const std::string& value) const
{
    //FIXME: implement tokenization + number extraction from string
	return ivec4(0, 0, 0, 0);
}

template <> ivec4 AttributeVisitCoercer<ivec4>::operator()(const float value) const
{
	return ivec4(value, 0, 0, 0);
}

template <> ivec4 AttributeVisitCoercer<ivec4>::operator()(const int value) const
{
	return ivec4(value, 0, 0, 0);
}

template <> ivec4 AttributeVisitCoercer<ivec4>::operator()(const bool value) const
{
	return value ? ivec4(1, 1, 1, 1) : ivec4(0, 0, 0, 0);
}

template <> ivec4 AttributeVisitCoercer<ivec4>::operator()(const vec4& value) const
{
	return ivec4(value.x, value.y, value.z, value.w);
}

template <> ivec4 AttributeVisitCoercer<ivec4>::operator()(const vec& value) const
{
	return ivec4(value.x, value.y, value.z, 0);
}

template <> ivec4 AttributeVisitCoercer<ivec4>::operator()(const ivec4& value) const
{
	return value;
}

template <> ivec4 AttributeVisitCoercer<ivec4>::operator()(const ivec& value) const
{
	return ivec4(value.x, value.y, value.z, 0.0f);
}

//TODO: ? check for ivec4 in AttributeRow_T, check for 4 ints, check for 4 floats, check for 4 bools (convert to int), check for string (convertable to int/float/bool)
//template <> ivec4 AttributeVisitCoercer<ivec4>::operator()(const AttributeRow_T& value) const
//{}


template <> ivec AttributeVisitCoercer<ivec>::operator()(const std::monostate& value) const
{
	return ivec(0, 0, 0);
}

template <> ivec AttributeVisitCoercer<ivec>::operator()(const std::string& value) const
{
    //FIXME: implement tokenization + number extraction from string
	return ivec(0, 0, 0);
}

template <> ivec AttributeVisitCoercer<ivec>::operator()(const float value) const
{
	return ivec(value, 0, 0);
}

template <> ivec AttributeVisitCoercer<ivec>::operator()(const int value) const
{
	return ivec(value, 0, 0);
}

template <> ivec AttributeVisitCoercer<ivec>::operator()(const bool value) const
{
	return value ? ivec(1, 1, 1) : ivec(0, 0, 0);
}

template <> ivec AttributeVisitCoercer<ivec>::operator()(const vec4& value) const
{
	return ivec(value.x, value.y, value.z);
}

template <> ivec AttributeVisitCoercer<ivec>::operator()(const vec& value) const
{
	return ivec(value.x, value.y, value.z);
}

template <> ivec AttributeVisitCoercer<ivec>::operator()(const ivec4& value) const
{
	return ivec(value.x, value.y, value.z);
}

template <> ivec AttributeVisitCoercer<ivec>::operator()(const ivec& value) const
{
	return value;
}

//TODO: ? check for ivec in AttributeRow_T, check for 3 ints, check for 3 floats, check for 3 bools (convert to int), check for string (convertable to int/float/bool)
//template <> ivec AttributeVisitCoercer<ivec>::operator()(const AttributeRow_T& value) const
//{}