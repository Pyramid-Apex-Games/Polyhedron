#pragma once

#include "vector.h"
#include <cstring>
#include <algorithm>

template <typename T, typename E>
void put(const T* data, int len, E& buf)
{
    const auto end = data + len;
    buf.reserve(buf.size() + len);
    for (auto ptr = data; ptr != end; ++ptr)
    {
        buf.emplace_back(*ptr);
    }
}

template <typename T, typename E>
void put(const T data, E& buf)
{
    buf.emplace_back(data);
}

template <typename T, typename E>
void put(const vector<T>& data, E& buf)
{
    put(data.data(), data.size(), buf);
}
template <typename T, typename E>
void put(const std::vector<T>& data, E& buf)
{
    put(data.data(), data.size(), buf);
}


template <typename T, typename E>
void putraw(T& src, E& tgt)
{
    if (src.empty()) return;

    using src_t = typename T::value_type;
    using tgt_t = typename E::value_type;
    const auto ePerT = sizeof(tgt_t) / sizeof(src_t);
    auto e_val = &tgt.emplace_back(tgt_t());
    for (int t = 0, e = 0; t < src.size(); t++)
    {
        const auto shift = t % ePerT;
        (*e_val) |= src[t] << (shift * 8);

        if (shift == ePerT - 1 && t + 1 < src.size())
        {
            e_val = &tgt.emplace_back(tgt_t());
            e++;
        }
    }
}

template <>
void put<const char*,vector<char> >(const char* data, vector<char>& buf);

template <typename V>
typename V::value_type &duplicate_back(V& source)
{
    return source.emplace_back(source.back());
}

template <typename T, typename V>
void remove_obj(T obj, V& list)
{
    list.erase(
        std::remove_if(
            list.begin(), list.end(),
            [&obj](const T& ele){
                return ele == obj;
            }
        ),
        list.end()
    );
}

template <typename T, typename V>
void remove_obj(T* obj, V& list)
{
    list.erase(
        std::remove_if(
            list.begin(), list.end(),
            [&obj](const T* ele){
                return ele == obj;
            }
        ),
        list.end()
    );
}

template <typename T>
bool in_list(T obj, const vector<T>& list)
{
    return std::find(list.begin(), list.end(), obj) != list.end();
}

template <typename T>
T* pad(int n, vector<T>& list)
{
    const auto oldSize = list.size();
    list.resize(oldSize + n);
    return list.begin() + oldSize;
}

template <typename T, typename N>
bool in_range(N i, const vector<T>& v)
{
    return i>=N(0) && i<N(v.size());
}
