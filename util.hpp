#include <utility>
#include <tuple>
#include <functional>
#include <fstream>
#include <unordered_set>

#ifndef _UTIL_HPP_
#define _UTIL_HPP_

namespace std {

    namespace custom {

    template <typename T>
        class HashCombine
        {
            std::hash<T> ht;

        public:
            size_t operator() (size_t seed, T const &t) const {
                return (seed ^ ht (t)) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            }
        };

    template <class Tuple, size_t Index = std::tuple_size<Tuple>::value -1>
        class HashValueImpl
        {
            HashValueImpl<Tuple, Index-1> hvi;
            HashCombine<typename std::tuple_element<Index, Tuple>::type> hc;

        public:
            size_t operator() (Tuple const &t) const {
                size_t seed = hvi (t);
                return hc (seed, std::get<Index> (t));
            }
        };

    template <class Tuple>
        class HashValueImpl<Tuple, 0>
        {
            hash<typename std::tuple_element<0, Tuple>::type> h0;

        public:
            size_t operator() (Tuple const &t) const {
                return h0 (std::get<0> (t));
            }
        };

    }

    template <typename A, typename B>
        class hash<pair<A, B>>
        {
            hash<A> ha;
            custom::HashCombine<B> hb;

        public:
            size_t operator()(const std::pair<A, B> &p) const {
                size_t seed = ha (p.first);
                return hb (seed, p.second);
            }
        };


    template <class Arg0, class... Args>
        class hash<tuple<Arg0, Args...>>
        {
            typedef tuple<Arg0, Args...> Tuple;
            custom::HashValueImpl<Tuple> hvi;

        public:
            size_t operator()(const Tuple &t) const {
                return hvi (t);
            }
        };

        
    template <class T>
        class hash<unordered_set<T>>
        {
            custom::HashCombine<T> ht;

        public:
            size_t operator()(const unordered_set<T> &ss) const {
                size_t seed = 0;
                for (auto &t : ss)
                    seed = ht (seed, t);
                return seed;
            }
        };
   
}

//template <typename T>
//class container_hash<T>
//{
//    std::custom::HashCombine<T> ht;
//
//public:
//    template <template <typename> class Container>
//    size_t operator()(const Container<T> &c) const {
//        size_t seed = 0;
//        for (auto t : c)
//            seed = hb (seed, t);
//        return seed;
//    }
//};
//
//
//template <typename T, typename T2 = T>
//class container_hash<std::pair<T, T2>>
//{
//    hash<A> ha;
//    container_hash<
//    custom::HashCombine<B> hb;
//
//    public:
//    size_t operator()(const std::pair<A, B> &p) const {
//        size_t seed = ha (p.first);
//        return hb (seed, p.second);
//    }
//};

template <typename InIt1, typename InIt2, typename OutIt>
OutIt unordered_set_intersection(InIt1 b1, InIt1 e1, InIt2 b2, InIt2 e2, OutIt out) {
    while (!(b1 == e1)) {
        if (!(std::find(b2, e2, *b1) == e2)) {
            *out = *b1;
            ++out;
        }

        ++b1;
    }

    return out;
}


#endif // _UTIL_HPP_
