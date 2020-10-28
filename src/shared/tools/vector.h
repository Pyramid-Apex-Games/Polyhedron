#pragma once
#include "loop.h"
#include "isclass.h"
#include "hash.h"
#include "databuf.h"
#include "macros.h"
#include "sort.h"
#include <type_traits>

template <class T> struct vector
{
    static const int MINSIZE {8};

    T *buf;
    int alen, ulen;

    vector() : buf(NULL), alen(0), ulen(0)
    {
    }

    vector(const vector &v) : buf(NULL), alen(0), ulen(0)
    {
        *this = v;
    }
    vector(vector<T>&& v)
    {
        if(!ulen)
        {
            std::swap(buf, v.buf);
            std::swap(ulen, v.ulen);
            std::swap(alen, v.alen);
        }
        else
        {
            reserve(ulen + v.ulen);
            if(v.ulen)
            {
                memcpy(&buf[ulen], (void *) v.buf, v.ulen * sizeof(T));
            }
            ulen += v.ulen;
            v.ulen = 0;
        }
    }

    ~vector() { shrink(0); if(buf) delete[] (uchar *)buf; }

    vector<T>& operator=(vector<T>&& v)
    {
        if(!ulen)
        {
            std::swap(buf, v.buf);
            std::swap(ulen, v.ulen);
            std::swap(alen, v.alen);
        }
        else
        {
            reserve(ulen + v.ulen);
            if(v.ulen) memcpy(&buf[ulen], (void  *)v.buf, v.ulen*sizeof(T));
            ulen += v.ulen;
            v.ulen = 0;
        }

        return *this;
    }

    T* begin() { return buf; }
    const T * begin() const { return buf; }
    T* end() { return buf + ulen; }
    const T * end() const { return buf + ulen; }

    vector<T> &operator=(const vector<T> &v)
    {
        shrink(0);
        if(v.size() > alen) reserve(v.size());
        loopv(v) emplace_back(v[i]);
        return *this;
    }

    template <typename... Ts>
    T &emplace_back(Ts&&... args)
    {
        if(ulen==alen) reserve(ulen + 1);
        new (&buf[ulen]) T(std::forward<Ts>(args)...);
        return buf[ulen++];
    }

    T &emplace_back(const T &x)
    {
        if(ulen==alen) reserve(ulen + 1);
        new (&buf[ulen]) T(x);
        return buf[ulen++];
    }

    T &emplace_back()
    {
        if(ulen==alen) reserve(ulen + 1);
        new (&buf[ulen]) T;
        return buf[ulen++];
    }

    bool inrange(size_t i) const { return i<size_t(ulen); }
    bool inrange(int i) const { return i>=0 && i<ulen; }
    T *disown() { T *r = buf; buf = NULL; alen = ulen = 0; return r; }
    bool inbuf(const T *e) const { return e >= buf && e < &buf[ulen]; }
private:
//    void drop() { ulen--; buf[ulen].~T(); }

//    void deletecontents(int n = 0) { while(ulen > n) delete &pop_back(); }
//
//    void deletearrays(int n = 0) { while(ulen > n) delete[] &pop_back(); }

    void call_finalizers(const T* begin, const T* end)
    {
        if (inbuf(begin) && inbuf(end))
        {
            T* ptr = buf + (begin - buf) + (end - begin);
            while(ptr != begin)
            {
                (*(--ptr)).~T();
            }
            (*begin).~T();
        }
    }

public:
    void clear()
    {
        call_finalizers(buf, buf + ulen);
        ulen = 0;
    }

    void erase(const T* begin, const T* end)
    {
        if (inbuf(begin) && inbuf(end))
        {
            if constexpr (std::is_destructible<T>::value)
            {
                call_finalizers(begin, end);
            }

            int holeSize = end - begin;
            int beginIndex = begin - buf;
            for(auto i = 0; beginIndex + holeSize + i < ulen; ++i)
            {
                std::swap(buf[beginIndex + i], buf[beginIndex + holeSize + i]);
            }
            ulen -= holeSize;
        }
    }

    T &pop_back() { return buf[--ulen]; }
    T &back() { return buf[ulen - 1]; }
    bool empty() const { return ulen==0; }

    int capacity() const { return alen; }
    int size() const { return ulen; }
    T &operator[](int i) { ASSERT(i>=0 && i<ulen); return buf[i]; }
    const T &operator[](int i) const { ASSERT(i >= 0 && i<ulen); return buf[i]; }

    T *data() { return buf; }
    const T *data() const { return buf; }

private:
    void shrink(int i) { ASSERT(i<=ulen); if constexpr (isclass<T>::yes) call_finalizers(buf + i, buf + ulen); ulen = i; }

    template<class F>
    void sort(F fun, int i = 0, int n = -1)
    {
        quicksort(&buf[i], n < 0 ? ulen-i : n, fun);
    }

    void sort() { sort(sortless()); }
    void sortname() { sort(sortnameless()); }

public:
    void resize(int i, T value = T())
    {
        if (alen < i)
            reserve(i+1);
        while (ulen < i)
            emplace_back(value);
        if (ulen > i)
        {
            shrink(i);
        }
    }

    void reserve(int sz)
    {
        int olen = alen;
        if(!alen) alen = std::max(MINSIZE, sz);
        else while(alen < sz) alen += alen/2;
        if(alen <= olen) return;
        uchar *newbuf = new uchar[alen*sizeof(T)];
        if(olen > 0)
        {
            if(ulen > 0) memcpy(newbuf, (void *)buf, ulen*sizeof(T));
            delete[] (uchar *)buf;
        }
        buf = (T *)newbuf;
    }

    databuf<T> reserve_raw_return(int sz)
    {
        if(alen-ulen < sz) reserve(ulen + sz);
        return databuf<T>(&buf[ulen], sz);
    }

    void advance(int sz)
    {
        ulen += sz;
    }

    void addbuf(const databuf<T> &p)
    {
        advance(p.length());
    }

    T *pad(int n)
    {
        T *buf = reserve_raw_return(n).buf;
        advance(n);
        return buf;
    }

    void put(const T &v) { emplace_back(v); }

    void put(const T *v, int n)
    {
        databuf<T> buf = reserve_raw_return(n);
        buf.put(v, n);
        addbuf(buf);
    }

    void remove(int i, int n)
    {
        for(int p = i+n; p<ulen; p++) buf[p-n] = buf[p];
        ulen -= n;
    }

    T remove(int i)
    {
        T e = buf[i];
        for(int p = i+1; p<ulen; p++) buf[p-1] = buf[p];
        ulen--;
        return e;
    }

    T removeunordered(int i)
    {
        T e = buf[i];
        ulen--;
        if(ulen>0) buf[i] = buf[ulen];
        return e;
    }

    template<class U>
    int find(const U &o)
    {
        loopi(ulen) if(buf[i]==o) return i;
        return -1;
    }

    void addunique(const T &o)
    {
        if(find(o) < 0) emplace_back(o);
    }

    void removeobj(const T &o)
    {
        loopi(ulen) if(buf[i] == o)
        {
            int dst = i;
            for(int j = i+1; j < ulen; j++) if(!(buf[j] == o)) buf[dst++] = buf[j];
            resize(dst);
            break;
        }
    }

    void replacewithlast(const T &o)
    {
        if(!ulen) return;
        loopi(ulen-1) if(buf[i]==o)
        {
            buf[i] = buf[ulen-1];
            break;
        }
        ulen--;
    }

    T &insert(int i, const T &e)
    {
        emplace_back(T());
        for(int p = ulen-1; p>i; p--) buf[p] = buf[p-1];
        buf[i] = e;
        return buf[i];
    }

    T *insert(int i, const T *e, int n)
    {
        if(alen-ulen < n) reserve(ulen + n);
        loopj(n) emplace_back(T());
        for(int p = ulen-1; p>=i+n; p--) buf[p] = buf[p-n];
        loopj(n) buf[i+j] = e[j];
        return &buf[i];
    }

    void reverse()
    {
        loopi(ulen/2) std::swap(buf[i], buf[ulen-1-i]);
    }

private:
    static int heapparent(int i) { return (i - 1) >> 1; }
    static int heapchild(int i) { return (i << 1) + 1; }

    void buildheap()
    {
        for(int i = ulen/2; i >= 0; i--) downheap(i);
    }

    int upheap(int i)
    {
        float score = heapscore(buf[i]);
        while(i > 0)
        {
            int pi = heapparent(i);
            if(score >= heapscore(buf[pi])) break;
            std::swap(buf[i], buf[pi]);
            i = pi;
        }
        return i;
    }

    T &addheap(const T &x)
    {
        emplace_back(x);
        return buf[upheap(ulen-1)];
    }

    int downheap(int i)
    {
        float score = heapscore(buf[i]);
        for(;;)
        {
            int ci = heapchild(i);
            if(ci >= ulen) break;
            float cscore = heapscore(buf[ci]);
            if(score > cscore)
            {
                if(ci+1 < ulen && heapscore(buf[ci+1]) < cscore) { std::swap(buf[ci+1], buf[i]); i = ci+1; }
                else { std::swap(buf[ci], buf[i]); i = ci; }
            }
            else if(ci+1 < ulen && heapscore(buf[ci+1]) < score) { std::swap(buf[ci+1], buf[i]); i = ci+1; }
            else break;
        }
        return i;
    }

    T removeheap()
    {
        T e = removeunordered(0);
        if(ulen) downheap(0);
        return e;
    }

    template<class K>
    int htfind(const K &key)
    {
        loopi(ulen) if(htcmp(key, buf[i])) return i;
        return -1;
    }

//#define UNIQUE(overwrite, cleanup) \
//        for(int i = 1; i < ulen; i++) if(htcmp(buf[i-1], buf[i])) \
//        { \
//            int n = i; \
//            while(++i < ulen) if(!htcmp(buf[n-1], buf[i])) { overwrite; n++; } \
//            cleanup; \
//            break; \
//        }
//    void unique() // contents must be initially sorted
//    {
//        UNIQUE(buf[n] = buf[i], resize(n));
//    }
//    void uniquedeletecontents()
//    {
//        UNIQUE(std::swap(buf[n], buf[i]), deletecontents(n));
//    }
//    void uniquedeletearrays()
//    {
//        UNIQUE(std::swap(buf[n], buf[i]), deletearrays(n));
//    }
//#undef UNIQUE
};
template <class T> const int vector<T>::MINSIZE;

template <typename T>
T &duplicate_back(vector<T>& source)
{
    return source.emplace_back(source.back());
}