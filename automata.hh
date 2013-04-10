
/************************************************************************
        automata.hh

  Copyright (C) 2013 - BOSS14420

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

**************************************************************************/

#ifndef _AUTOMATA_HH_
#define _AUTOMATA_HH_

#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <utility>

template<typename T> using Set = std::unordered_set<T>;
template<typename Key, typename Value> using Map = std::unordered_map<Key, Value>;

namespace std {

    template <typename A, typename B>
        struct hash<pair<A, B>>
        {
            private:
                const hash<A> ah;
                const hash<B> bh;
            public:
                hash() : ah(), bh() {}
                size_t operator()(const std::pair<A, B> &p) const {
                    size_t seed = ah(p.first);
                    return bh(p.second) + 0x9e3779b9 + (seed<<6) + (seed>>2);
                }
        };
}


template <typename C>
class DFA {
public:
    typedef int State;
    typedef std::pair<std::pair<State, C>, State> Transition;

public:
    DFA() {}

    DFA (Set<std::basic_string<C>> const &code);

private:
    std::vector<C> alphabet;
    Set<State> states;
    State initState;
    Set<State> finishStates;
    Map<std::pair<State, C>, State> transitions;
};


template <typename C>
class NFA {
public:
    typedef int State;
    typedef std::pair<std::pair<State, C>, Set<State>> Transition;
    friend class DFA<C>;
public:
    NFA() {}

    NFA (Set<std::basic_string<C>> const &code);

private:
    std::vector<C> alphabet;
    Set<State> states;
    State initState;
    Set<State> finishStates; 
    Map<std::pair<State, C>, Set<State>> transitions;
};

#include "automata.cc"

#endif // _AUTOMATA_HH_
