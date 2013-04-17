
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
bool FiniteAutomation<C>::recognizeEmptyString() const
{
    return finishStates.find (initState) != finishStates.end();
}

template <typename C>
NFA<C> NFA<C>::notRecogEmptyNFA() const
{
    if (!recognizeEmptyString())
        return *this;
    
    NFA neNfa (*this);

    std::deque<C> charList;
    for (C c : alphabet)
//        if (CONTAIN (transitions, std::make_pair (initState, c)))
        if (CONTAIN (transitions, std::make_pair (initState, c)))
            charList.push_back (c);
    
    if (charList.empty()) {
        neNfa.finishStates.erase (initState);
        return neNfa;
    }

    State newInitState (*std::max_element (states.begin(), states.end()) + 1);
    neNfa.initState = newInitState;
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


template <typename C>
DFA<C>::DFA (NFA<C> const &nfa)
{
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

    int maxStateVal = *std::max_element (nfa.states.begin(), nfa.states.end());
    int iStateCount = maxStateVal + 1;
    while (!openSet.empty()) {
        auto currentState = openSet.front();
        openSet.pop();

        for (auto &c : alphabet) {
            Set<State> nextStates;

            for (auto &s : unitStates[currentState]) {
                auto iNTranst = nfa.transitions.find ({s, c});
                if (iNTranst == nfa.transitions.end())
                    continue;
                auto &nextNfaStates = iNTranst->second;
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
                for (int s = maxStateVal + 1; s != iStateCount; ++s) {
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


    //
    // Finish states

    for (State s : states)
        for (State f : nfa.finishStates)
            if (CONTAIN (unitStates[s], f)) {
                finishStates.insert (s);
                break;
            }
}

template <typename C>
DFA<C>& DFA<C>::removeInAccessibleStates()
{
    Map<State, Set<State>> nextStates;
    for (auto &transt : transitions)
        nextStates[transt->first.first].insert (transitions.second);
    

    //
    // remove inaccessible states

    std::queue<State> statesQueue = { initState };
    Set<State> accessibleStates = { initState };
    while (!statesQueue.empty()) {
        State s = statesQueue.front();
        statesQueue.pop();
        accessibleStates.insert (s);
        for (State ns : nextStates[s])
            if (!CONTAIN (accessibleStates, ns))
                statesQueue.insert (ns);
    }

    // remove inaccessible path
    decltype(transitions) newTransitions;
    for (auto &transt : transitions) {
        if (CONTAIN (accessibleStates, transt->first.first)
             && CONTAIN (accessibleStates, transt->second))
            newTransitions.insert (transt);
    }

    // remove inaccessible finish states
    decltype(finishStates) newFinishStates;
    for (State s : finishStates)
        if (CONTAIN (accessibleStates, s))
            newFinishStates.insert (s);
    finishStates = std::move (newFinishStates);
    transitions = std::move (newTransitions);
    states = std::move (accessibleStates);

    return *this;
}

template <typename C>
DFA<C>& DFA<C>::removeNotCoaccessibleStates()
{
    Map<State, Set<State>> prevStates;
    for (auto &transt : transitions)
        prevStates[transt.second].insert (transt.first.first);
    

    //
    // remove not-coaccessible states

    std::queue<State> statesQueue;
    for (State f : finishStates) statesQueue.push (f);

    Set<State> coaccessibleStates = finishStates;
    while (!statesQueue.empty()) {
        State s = statesQueue.front();
        statesQueue.pop();
        coaccessibleStates.insert (s);
        for (State ps : prevStates[s])
            if (!CONTAIN (coaccessibleStates, ps))
                statesQueue.push (ps);
    }

    // remove not-coaccessible path
    decltype(transitions) newTransitions;
    for (auto &transt : transitions) {
        if (CONTAIN (coaccessibleStates, transt.first.first)
             && CONTAIN (coaccessibleStates, transt.second))
            newTransitions.insert (transt);
    }

    if (!CONTAIN (coaccessibleStates, initState))
        initState = invalid_state;

    transitions = std::move (newTransitions);
    states = std::move (coaccessibleStates);

    return *this;
}

template <typename C>
DFA<C>& DFA<C>::trim()
{
    return removeNotCoaccessibleStates (removeInAccessibleStates());
}

template <typename C>
bool operator== (DFA<C> const &dfa1, DFA<C> const &dfa2)
{
    DFA<C> td1 = dfa1; td1.removeNotCoaccessibleStates();
    DFA<C> td2 = dfa2; td2.removeNotCoaccessibleStates();

    typedef typename DFA<C>::State State;
    typedef std::pair<State, State> StatePair;
    std::queue<StatePair> pairsQueue;
    pairsQueue.push ({td1.initState, td2.initState});
    Set<StatePair> oldPairs;

    auto &transts1 = td1.transitions, &transts2 = td2.transitions;

    while (!pairsQueue.empty()) {
        auto p = pairsQueue.front();
        pairsQueue.pop();
        oldPairs.insert (p);

        if (   (CONTAIN (td1.finishStates, p.first)
                 && !CONTAIN (td2.finishStates, p.second))
            || (!CONTAIN (td1.finishStates, p.first)
                 && CONTAIN (td2.finishStates, p.second))
           )
            return false;

        for (C c : td1.alphabet) {
            auto t1 = transts1.find ({p.first, c});
            auto t2 = transts2.find ({p.second, c});
            if (t1 == transts1.end()) {
                if (t2 == transts2.end())
                    continue;
                else
                    return false;
            } else {
                if (t2 == transts2.end())
                    return false;
                else {
                    StatePair np = { t1->second, t2->second };
                    if (!CONTAIN (oldPairs, np))
                        pairsQueue.push (np);
                }
            }
        }
    }

    return true;
}
