
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
#include "automata.hh"

#define CONTAIN (c, k) (c.find (k) != c.end())

template <typename C>
NFA::NFA (Set<std::basic_string<C>> const &code)
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

    for (auto &w : code) {
        State prevState = 0;
        auto wi = w.begin();
        for (; wi + 1 != w.end(); ++w) {
            auto iTranst = transitions.find ({prevState, *wi});
            if (iTranst == transitions.end()) {
                transitions[{prevState, *wi}] = {prevState = ++iStateCount};
            } else {
                prevState = -1;
                for (auto s : iTranst->second)
                    if (s != 0) {
                        prevState = s;
                        break;
                    }

                if (prevState == -1) 
                    iTranst->second.insert (prevState = ++iStateCount);
            }
        }
        transitions[{prevState, *wi}].insert (0);
    }
}

template <typename C>
DFA::DFA (Set<std::basic_string<C>> const &code) 
{

    NFA nfa (code);


    // construct DFA
    //
    
    // DFA initState
    initState = nfa.initState;

    // DFA transitions && state
    std::queue<State> openSet;

    stateQueue.push (initState);

    Map<State, Set<State>> unitStates;
    unitStates[initState] = { initState };

    int iStateCount = nfa.states.size() - 1;
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
                unitStates[transitions[{currentState,c}]] = { s };
                openSet.push (s);
            } else {
                for (int s = nfa.states.size(); s != iStateCount; ++s) {
                    if (unitStates[s] != nextStates)
                        
                }
            }
        }
    }

_exit:
    
    if (goto_initState == 2)
        throw "Ambiguous";

    //
    // Finish states

    for (auto is = states.begin(); is != states.end(); ++is)
        if (CONTAIN (is->unitStates, 0))
            finishStates->insert (is);
}
