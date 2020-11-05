#include "vector_util.h"

//template <typename T, typename E>
//void put(const T data, vector<E>& buf)
//{
//    put(data.data(), buf.size(), buf);
//}
//
//template <typename T, typename E>
//void put(const T* data, int len, vector<E>& buf)
//{
//    const auto end = data + len;
//    for (auto ptr = data; ptr != end; ++ptr)
//    {
//        buf.emplace_back(*ptr);
//    }
//}

template <>
void put<const char*,vector<char>>(const char* data, vector<char>& buf)
{
    put<char,vector<char> >(data, (int)strlen(data), buf);
}

//template <typename T>
//T &duplicate_back(vector<T>& source)
//{
//    return source.emplace_back(source.back());
//}

//template <typename T>
//void remove_obj(T obj, vector<T>& list)
//{
//    list.erase(
//        std::remove_if(
//            list.begin(), list.end(),
//            [&obj](const T& ele){
//                return ele == obj;
//            }
//        ),
//        list.end()
//    );
//}

//template <typename T>
//void remove_obj(T* obj, vector<T*>& list)
//{
//    list.erase(
//        std::remove_if(
//            list.begin(), list.end(),
//            [&obj](const T* ele){
//                return ele == obj;
//            }
//        ),
//        list.end()
//    );
//}

//template <typename T>
//bool in_list(T obj, const vector<T>& list)
//{
//    return std::find(list.begin(), list.end(), obj) != list.end();
//}

//template <typename T>
//T* pad(int n, vector<T>& list)
//{
//    const auto oldSize = list.size();
//    list.resize(oldSize + n);
//    return list.begin() + oldSize;
//}

//template <typename T, typename N>
//bool in_range(N i, const vector<T>& v)
//{
//    return i<N(v.size());
//}

#include <vector>

namespace TEST
{
    vector<uint> cubevector;
    std::vector<uint> stlvector;

    bool compare()
    {
        if (cubevector.size() != stlvector.size())
            return false;

        if (cubevector.empty())
        {
            if (stlvector.empty())
            {
                return true;
            }
            else
            {
                return false;
            }
        }

        for (int i = 0; i < cubevector.size(); ++i)
        {
            if (cubevector[i] != stlvector[i])
                return false;
        }

        return true;
    }

    void test()
    {
        cubevector.emplace_back(0);
        cubevector.emplace_back(1);
        cubevector.emplace_back(2);
        cubevector.emplace_back(3);
        cubevector.emplace_back(4);
        cubevector.emplace_back(-1);
        stlvector.emplace_back(0);
        stlvector.emplace_back(1);
        stlvector.emplace_back(2);
        stlvector.emplace_back(3);
        stlvector.emplace_back(4);
        stlvector.emplace_back(-1);

        assert(compare() && "init");

        put(100, cubevector);
        put(100, stlvector);

        assert(compare() && "put 1");

        std::vector lst = {101,102,103,100};
        put(lst, cubevector);
        put(lst, stlvector);

        assert(compare() && "put more");

        duplicate_back(cubevector);
        duplicate_back(stlvector);

        assert(compare() && "duplicate_back");

        remove_obj(100, cubevector);
        remove_obj(100, stlvector);

        assert(compare() && "remove_obj multi");

        remove_obj(-1, cubevector);
        remove_obj(-1, stlvector);

        assert(compare() && "remove_obj single");

        std::vector<char> chrs = {'a', 'b', 'c', 'd'};
        put(chrs, cubevector);
        put(chrs, stlvector);

        assert(compare() && "put chars");

        putraw(chrs, cubevector);
        putraw(chrs, stlvector);

        assert(compare() && "putraw chars");
        assert(cubevector.back() == 1684234849 && "valuecheck");
        assert(stlvector.back() == 1684234849 && "valuecheck");


    }
}