#include <utility>
#include <tuple>
#include <functional>
#include <fstream>

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
}



#endif // _UTIL_HPP_
