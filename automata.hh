
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
#include "util.hpp"

template<typename T> using Set = std::unordered_set<T>;
template<typename Key, typename Value> using Map = std::unordered_map<Key, Value>;


//template <typename C> class DFA;
//template <typename C> class NFA;

class FiniteAutomation {
public:
    typedef int State;
    typedef char C;

    static const State invalid_state = -1;
    static const C empty_letter;

    typedef std::uint64_t FAFlag;
    static const FAFlag FlagDFA = 1;
    static const FAFlag FlagNFA = 2;
    static const FAFlag FlagHasEMove = 4;
    static const FAFlag FlagAcceccable = 8;
    static const FAFlag FlagCoacceccable = 16;

//    enum FAFlag { FlagDFA = 1, FlagNFA = 2, FlagHasEMove = 4, 
//                  FlagAcceccable = 8, FlagCoacceccable = 16 };

    typedef std::pair<std::pair<State, C>, Set<State>> Transition;

public:
    FiniteAutomation() = default;

    FiniteAutomation (std::vector<C> const& alphabet, Set<State> const &states, 
                      State initState, Set<State> const &finishStates,
                      Map<std::pair<State,C>, Set<State>> const &transitions,
                      FAFlag flags)
                : alphabet (alphabet), states (states), initState (initState), 
                  finishStates (finishStates), transitions (transitions),
                  flags (flags)
    {}

    FiniteAutomation (std::vector<C> const& alphabet, Set<State> const &states, 
                      State initState, Set<State> const &finishStates,
                      Map<std::pair<State,C>, Set<State>> const &transitions)
                : alphabet (alphabet), states (states), initState (initState), 
                  finishStates (finishStates), transitions (transitions)
    {}

    FiniteAutomation (Set<std::basic_string<C>> const &code);

public:
    bool recognizeEmptyString() const;

    FiniteAutomation& excludeEmptyString();
    FiniteAutomation& removeEMoves();
    FiniteAutomation& removeInAccessibleStates();
    FiniteAutomation& removeNotCoaccessibleStates();
    FiniteAutomation& trim();

    friend 
    bool operator== (FiniteAutomation const&, FiniteAutomation const&);

private:
    Set<State> eClosure (State s) const;
    Set<State> eClosure (Set<State> const &ss) const;
    
    FiniteAutomation& normalizeStateIndex();

private:
    std::vector<C> alphabet;
    Set<State> states;
    State initState;
    Set<State> finishStates;
    Map<std::pair<State, C>, Set<State>> transitions;
    FAFlag flags;
};

//template <typename C>
//class DFA : public FiniteAutomation<C> {
//public:
//    using typename FiniteAutomation<C>::State;
//    typedef std::pair<std::pair<State, C>, State> Transition;
//
//    using FiniteAutomation<C>::invalid_state;
//
//public:
//    DFA() = default;
//
//    DFA (NFA<C> const &nfa);
//
//    DFA (Set<std::basic_string<C>> const &code);
//
//public:
//    using FiniteAutomation<C>::recognizeEmptyString;
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
//    using FiniteAutomation<C>::alphabet;
//    using FiniteAutomation<C>::states;
//    using FiniteAutomation<C>::initState;
//    using FiniteAutomation<C>::finishStates;
//};
//
//
//template <typename C>
//class NFA : public FiniteAutomation<C> {
//public:
//    using typename FiniteAutomation<C>::State;
//    typedef std::pair<std::pair<State, C>, Set<State>> Transition;
//    friend class DFA<C>;
//
//private:
//    Map<std::pair<State, C>, Set<State>> transitions;
//    using FiniteAutomation<C>::alphabet;
//    using FiniteAutomation<C>::states;
//    using FiniteAutomation<C>::initState;
//    using FiniteAutomation<C>::finishStates;
//
//public:
//    NFA() {}
//
//    NFA (std::vector<C> const& alphabet, Set<State> const &states, 
//             State initState, Set<State> const &finishStates,
//             Map<std::pair<State,C>,Set<State>> const &transitions)
//        : FiniteAutomation<C> (alphabet, states, initState, finishStates),
//          transitions (transitions) {}
//
//    NFA (Set<std::basic_string<C>> const &code);
//
//public:
//    using FiniteAutomation<C>::recognizeEmptyString;
//    NFA notRecogEmptyNFA() const;
//
//};
//

#endif // _AUTOMATA_HH_
