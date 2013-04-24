
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
#include <string>
#include <exception>
#include "util.hpp"

template<typename T> using Set = std::unordered_set<T>;
template<typename Key, typename Value> using Map = std::unordered_map<Key, Value>;



class XmlParseFailed : public std::exception {
public:
    XmlParseFailed (char const *str = nullptr) : str (str) {}
private:
    std::string str;
};


class FiniteAutomaton {
public:
    typedef int State;
    typedef char C;

    static const State invalid_state = -1;
    static const C empty_letter;

    typedef std::uint64_t FAFlag;
    static const FAFlag FlagDFA = 1;
    static const FAFlag FlagNFA = 2;
    static const FAFlag FlagHasEMove = 4;
    static const FAFlag FlagAccessible = 8;
    static const FAFlag FlagCoaccessible = 16;

//    enum FAFlag { FlagDFA = 1, FlagNFA = 2, FlagHasEMove = 4, 
//                  FlagAccessible = 8, FlagCoaccessible = 16 };

    typedef std::pair<std::pair<State, C>, Set<State>> Transition;

public:
    FiniteAutomaton() 
        : states ({0}), initState (0),
          flags (FlagDFA | FlagAccessible | FlagCoaccessible)
    {}

    FiniteAutomaton (char const *filename);

    FiniteAutomaton (Set<C> const& alphabet, Set<State> const &states, 
                      State initState, Set<State> const &finishStates,
                      Map<std::pair<State,C>, Set<State>> const &transitions,
                      FAFlag flags)
                : alphabet (alphabet), states (states), initState (initState), 
                  finishStates (finishStates), transitions (transitions),
                  flags (flags)
    {}

    FiniteAutomaton (Set<C> const& alphabet, Set<State> const &states, 
                      State initState, Set<State> const &finishStates,
                      Map<std::pair<State,C>, Set<State>> const &transitions)
                : alphabet (alphabet), states (states), initState (initState), 
                  finishStates (finishStates), transitions (transitions)
    {}

    FiniteAutomaton (Set<std::basic_string<C>> const &code);

public:
    bool recognizeEmptyString() const;
    bool isEmpty() const;

    FiniteAutomaton& excludeEmptyString();
    FiniteAutomaton& removeEMoves();
    FiniteAutomaton& removeInAccessibleStates();
    FiniteAutomaton& removeNotCoaccessibleStates();
    FiniteAutomaton& trim();

    FiniteAutomaton& cutByPrefix (FiniteAutomaton prefix);

    friend 
    bool operator== (FiniteAutomaton const&, FiniteAutomaton const&);

    void writeToXmlFile (char const *filename);

private:
    Set<State> eClosure (State s) const;
    Set<State> eClosure (Set<State> const &ss) const;
    
    FiniteAutomaton& normalizeStateIndex();

private:
    Set<C> alphabet;
    Set<State> states;
    State initState;
    Set<State> finishStates;
    Map<std::pair<State, C>, Set<State>> transitions;
    FAFlag flags;
};

//template <typename C>
//class DFA : public FiniteAutomaton<C> {
//public:
//    using typename FiniteAutomaton<C>::State;
//    typedef std::pair<std::pair<State, C>, State> Transition;
//
//    using FiniteAutomaton<C>::invalid_state;
//
//public:
//    DFA() = default;
//
//    DFA (NFA<C> const &nfa);
//
//    DFA (Set<std::basic_string<C>> const &code);
//
//public:
//    using FiniteAutomaton<C>::recognizeEmptyString;
//
//    DFA& trim();
//    DFA& removeInAccessibleStates();
//    DFA& removeNotCoaccessibleStates();
//
////    template <>
//    friend bool operator== <C> (DFA<C> const &dfa1, DFA<C> const &dfa2);
//
//private:
//    Map<std::pair<State, C>, State> transitions;
//    using FiniteAutomaton<C>::alphabet;
//    using FiniteAutomaton<C>::states;
//    using FiniteAutomaton<C>::initState;
//    using FiniteAutomaton<C>::finishStates;
//};
//
//
//template <typename C>
//class NFA : public FiniteAutomaton<C> {
//public:
//    using typename FiniteAutomaton<C>::State;
//    typedef std::pair<std::pair<State, C>, Set<State>> Transition;
//    friend class DFA<C>;
//
//private:
//    Map<std::pair<State, C>, Set<State>> transitions;
//    using FiniteAutomaton<C>::alphabet;
//    using FiniteAutomaton<C>::states;
//    using FiniteAutomaton<C>::initState;
//    using FiniteAutomaton<C>::finishStates;
//
//public:
//    NFA() {}
//
//    NFA (std::vector<C> const& alphabet, Set<State> const &states, 
//             State initState, Set<State> const &finishStates,
//             Map<std::pair<State,C>,Set<State>> const &transitions)
//        : FiniteAutomaton<C> (alphabet, states, initState, finishStates),
//          transitions (transitions) {}
//
//    NFA (Set<std::basic_string<C>> const &code);
//
//public:
//    using FiniteAutomaton<C>::recognizeEmptyString;
//    NFA notRecogEmptyNFA() const;
//
//};
//

#endif // _AUTOMATA_HH_
