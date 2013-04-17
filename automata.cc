
/************************************************************************
        automata.cc

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

#include <queue>
#include <algorithm>
#include "automata.hh"

#define CONTAIN(c,k) (c.find (k) != c.end())

template <typename C>
NFA<C>::NFA (Set<std::basic_string<C>> const &code)
{
    // alphabet
    //
    Set<C> alphabet_set;
    for (auto &w : code)
        for (auto &c : w)
            alphabet_set.insert (c);
    alphabet.assign (alphabet_set.begin(), alphabet_set.end());


    // construct NFA
    //

    int iStateCount = 0;
    initState = iStateCount;
    states.insert (initState);
    finishStates.insert (initState);

    for (auto &w : code) {
        State prevState = 0;
        auto wi = w.begin();
        for (; wi + 1 != w.end(); ++wi) {
            auto iTranst = transitions.find ({prevState, *wi});
            if (iTranst == transitions.end()) {
                transitions[{prevState, *wi}] = {++iStateCount};
                prevState = iStateCount;
                states.insert (iStateCount);
            } else {
                prevState = -1;
                for (auto s : iTranst->second)
                    if (s != 0) {
                        prevState = s;
                        break;
                    }

                if (prevState == -1) {
                    iTranst->second.insert (prevState = ++iStateCount);
                    states.insert (iStateCount);
                }
            }
        }
        transitions[{prevState, *wi}].insert (0);
    }
}

template <typename C>
bool NFA<C>::recognizeEmptyString() const
{
    return finishStates.find (initState) != finishStates.end();
}

template <typename C>
NFA<C> NFA<C>::notRecogEmptyNFA() const
{
    if (!recognizeEmptyString())
        return *this;
    
    NFA neNfa (*this);
    neNfa.finishStates.erase (initState);

    std::deque<C> charList;
    for (C c : alphabet)
        if (CONTAIN (transitions, {initState, c}))
            charList.push_back (c);
    
    if (charList.empty())
        return neNfa;

    State newInitState (*std::max_element (states.begin(), states.end()) + 1);
    for (C c : charList)
        neNfa.transitions[{newInitState,c}] = {initState};

    return neNfa;
}


template <typename C>
DFA<C>::DFA (Set<std::basic_string<C>> const &code) 
{

    NFA<C> nfa (code);


    // construct DFA
    //

    // DFA alphabet
    alphabet = nfa.alphabet;

    // DFA initState
    initState = nfa.initState;
    states.insert (initState);

    // DFA transitions && state
    std::queue<State> openSet;

    openSet.push (initState);

    Map<State, Set<State>> unitStates;
    unitStates[initState] = { initState };

    int iStateCount = nfa.states.size();
    int goto_initState = 0;
    while (!openSet.empty()) {
        auto currentState = openSet.front();
        openSet.pop();

        for (auto &c : alphabet) {
            goto_initState = 0;

            Set<State> nextStates;

            for (auto &s : unitStates[currentState]) {
                auto iNTranst = nfa.transitions.find ({s, c});
                if (iNTranst == nfa.transitions.end())
                    continue;
                auto &nextNfaStates = iNTranst->second;
                if (CONTAIN(nextNfaStates, 0))
                    if (++goto_initState == 2)
                        goto _exit;

//                if (!CONTAIN(transitions, {currentState,c})) {
//                    states.insert (++iStateCount);
//                    openSet.push (iStateCount);
//                    transitions[{currentState, c}] = iStateCount;
//                    unitStates [iStateCount] = {};
//                }
//                unitStates[transitions[{currentState,c}]]
//                                .insert (nextStates.begin(), nextStates.end());

                nextStates.insert (nextNfaStates.begin(), nextNfaStates.end());
            }

            if (nextStates.empty())
                continue;
            else if (nextStates.size() == 1) {
                State s = *nextStates.begin();
                states.insert (s);
                transitions[{currentState,c}] = s;
                if (!CONTAIN (unitStates, s)) {
                    unitStates[s] = { s };
                    openSet.push (s);
                }
            } else {

                bool foundOldState = false;
                for (int s = nfa.states.size(); s != iStateCount; ++s) {
                    if (unitStates[s] == nextStates) {
                        transitions[{currentState,c}] = s;
                        foundOldState = true;
                        break;
                    }
                }

                if (!foundOldState) {
                    states.insert (iStateCount);
                    transitions[{currentState,c}] = iStateCount;
                    unitStates[iStateCount] = std::move (nextStates);
                    openSet.push (iStateCount);
                    ++iStateCount;
                }
            }
        }
    }

_exit:
    
    if (goto_initState == 2)
        throw "Ambiguous";

    //
    // Finish states

    for (State s : states)
        if (CONTAIN (unitStates[s], initState))
            finishStates.insert (s);
}
