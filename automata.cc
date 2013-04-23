
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
#include <tuple>
#include "automata.hh"

#define CONTAIN(c,k) (c.find (k) != c.end())

const FiniteAutomation::C FiniteAutomation::empty_letter = C() - 2;

/* template <typename C>
 * NFA<C>::NFA (Set<std::basic_string<C>> const &code)
 * {
 *     // alphabet
 *     //
 *     Set<C> alphabet_set;
 *     for (auto &w : code)
 *         for (auto &c : w)
 *             alphabet_set.insert (c);
 *     alphabet.assign (alphabet_set.begin(), alphabet_set.end());
 * 
 * 
 *     // construct NFA
 *     //
 * 
 *     int iStateCount = 0;
 *     initState = iStateCount;
 *     states.insert (initState);
 *     finishStates.insert (initState);
 * 
 *     for (auto &w : code) {
 *         State prevState = 0;
 *         auto wi = w.begin();
 *         for (; wi + 1 != w.end(); ++wi) {
 *             auto iTranst = transitions.find ({prevState, *wi});
 *             if (iTranst == transitions.end()) {
 *                 transitions[{prevState, *wi}] = {++iStateCount};
 *                 prevState = iStateCount;
 *                 states.insert (iStateCount);
 *             } else {
 *                 prevState = -1;
 *                 for (auto s : iTranst->second)
 *                     if (s != 0) {
 *                         prevState = s;
 *                         break;
 *                     }
 * 
 *                 if (prevState == -1) {
 *                     iTranst->second.insert (prevState = ++iStateCount);
 *                     states.insert (iStateCount);
 *                 }
 *             }
 *         }
 *         transitions[{prevState, *wi}].insert (0);
 *     }
 * }
 * 
 */

bool FiniteAutomation::recognizeEmptyString() const
{
    bool initIsFinish = CONTAIN (finishStates, initState);

    if (!(flags & FlagHasEMove))
        return initIsFinish;

    if (initIsFinish)
        return true;

    std::queue<State> closure;
    closure.push (initState);

    std::vector<bool> accessedStates (states.size(), false);

    while (!closure.empty()) {
        State s = closure.front();

        if (CONTAIN (finishStates, s))
            return true;

        closure.pop();
        accessedStates[s] = true;
        
        auto iNextStates = transitions.find ({initState,empty_letter}); 
        if (iNextStates == transitions.end())
            continue;

        for (State ns : iNextStates->second)
            if (!accessedStates[ns])
                closure.push (ns);
    }

    return false;
}

bool FiniteAutomation::isEmpty() const
{
    // TODO when not coaccessible


    return (flags & FlagCoaccessible) && finishStates.empty();
}

FiniteAutomation& FiniteAutomation::excludeEmptyString() 
{
    if (!recognizeEmptyString())
        return *this;
    
    if (flags & FlagHasEMove)
        removeEMoves();

    std::deque<C> charList;
    for (C c : alphabet)
        if (CONTAIN (transitions, std::make_pair (initState, c)))
            charList.push_back (c);
    
    if (charList.empty()) {
        finishStates.erase (initState);
    } else {
        State newInitState (states.size() + 1);
        for (C c : charList)
            transitions[{newInitState,c}] = {initState};
        initState = newInitState;
    }

    return *this;
}


Set<FiniteAutomation::State> FiniteAutomation::eClosure (State s) const
{
    if (!(flags & FlagHasEMove))
        return {s};

    Set<State> result;

    std::queue<State> nextStates;
    nextStates.push (s);

    std::vector<bool> accessedStates (states.size(), false);

    while (!nextStates.empty()) {
        State ns = nextStates.front();
        nextStates.pop();
        accessedStates[ns] = true;
        result.insert (ns);

        auto iNext = transitions.find ({ns, empty_letter});
        if (iNext == transitions.end())
            continue;
        for (State nns : iNext->second) {
            if (!accessedStates[nns])
                nextStates.push (nns);
        }
    }

    return result;
}

Set<FiniteAutomation::State> 
FiniteAutomation::eClosure (Set<State> const &ss) const
{
    Set<State> result;
    for (State s : ss) {
        Set<State> ec = eClosure (s);
        result.insert (ec.begin(), ec.end());
    }
    return result;
}

FiniteAutomation& FiniteAutomation::removeEMoves()
{
    if (!(flags & FlagHasEMove))
        return *this;

    //
    // transtsFrom table

    std::vector<std::deque<std::pair<C, Set<State>>>> transtsFrom (states.size());
    for (auto &transt : transitions)
        transtsFrom[transt.first.first]
                .push_back ({transt.first.second, transt.second});


    typedef std::tuple<State, C, State> UnitTransition; 

    std::deque<UnitTransition> newTransitions;
    std::queue<UnitTransition> W;
    
    Set<UnitTransition> oldTransitions;

    //
    // add shortcuts

    State q1, q2;
    char c;
    while (!W.empty()) {
        auto t = W.front();
        W.pop();

        if (CONTAIN (oldTransitions, t))
            continue;
        oldTransitions.insert (t);

        std::tie (q1, c, q2) = t;

        if (c != empty_letter) {
            for (auto &tf : transtsFrom[q2]) {
                if (tf.first == empty_letter) {
                    // find all (q2, epsilon, q3)
                    for (auto q3 : tf.second) {
                        W.push (std::make_tuple (q1, c, q3));
                        transitions[{q1, c}].insert (q3);
                    } 

                } else {
                    // find all (q2, a, q3)
                    for (auto q3 : tf.second)
                        // old normal transitions, don't add to newTransitions
                        W.push (std::make_tuple (q1, c, q3));
                }
            } 

        } else {
            if (CONTAIN (finishStates, q2))
                finishStates.insert (q1);

            for (auto &tf : transtsFrom[q2]) {
                if (tf.first == empty_letter) {
                    for (auto q3 : tf.second)
                        W.push (std::make_tuple (q1, empty_letter, q3));
                } else {
                    for (auto q3 : tf.second) {
                        W.push (std::make_tuple (q1, tf.first, q3));
                        transitions[{q1, tf.first}].insert (q3);
                    }
                }
            }
        }
    }


    //
    // remove empty moves

    for (auto s : states) {
        auto iEMoves = transitions.find ({s, empty_letter});
        if (iEMoves != transitions.end())
            transitions.erase (iEMoves);
    }

    flags |= ~FlagHasEMove;

    return *this;
}


FiniteAutomation& FiniteAutomation::normalizeStateIndex()
{
    Map<State, State> newIndex;
    decltype(states) newStates;
    State ni = 0;
    for (State s : states) {
        newIndex[s] = ni;
        newStates.insert (ni++);
    }
    states = std::move (newStates);

    initState = newIndex[initState];

    decltype(finishStates) newFinishStates;
    for (State s : finishStates) 
        newFinishStates.insert (newIndex[s]);
    finishStates = std::move (newFinishStates);

    decltype(transitions) newTransitions;
    for (auto &transts : transitions) {
        Transition newTransts;
        newTransts.first.first = newIndex[transts.first.first];
        for (State ns : transts.second)
            newTransts.second.insert (newIndex[ns]);
        newTransitions.insert (newTransts);
    }
    transitions = std::move (newTransitions);

    return *this;
}

FiniteAutomation& FiniteAutomation::removeInAccessibleStates()
{
    if (flags & FlagCoaccessible)
        return *this;

    std::vector<Set<State>> nextStates (states.size());
    for (auto &transt : transitions)
        nextStates[transt.first.first]
            .insert (transt.second.begin(), transt.second.end());

    //
    // remove inaccessible states

    std::queue<State> statesQueue;
    for (State f : finishStates) statesQueue.push (f);

    Set<State> accessibleStates = { initState };
    while (!statesQueue.empty()) {
        State s = statesQueue.front();
        statesQueue.pop();
        accessibleStates.insert (s);
        for (State ps : nextStates[s])
            if (!CONTAIN (accessibleStates, ps))
                statesQueue.push (ps);
    }

    // remove inaccessible path
    decltype(transitions) newTransitions;
    for (auto &transt : transitions) {
        if (CONTAIN (accessibleStates, transt.first.first)) {
            Transition newTranst;
            newTranst.first = transt.first;
            for (State ns : transt.second)
                if (CONTAIN (accessibleStates, ns))
                    newTranst.second.insert (ns);
            if (!newTranst.second.empty())
                newTransitions.insert (newTranst);
        }
    }

    
    // remove inaccessible finish states
    decltype(finishStates) newFinishStates;
    for (State s : finishStates)
        if (CONTAIN (accessibleStates, s))
            newFinishStates.insert (s);
    finishStates = std::move (newFinishStates);
    transitions = std::move (newTransitions);
    states = std::move (accessibleStates);

    normalizeStateIndex();

    flags |= FlagAccessible;

    return *this;
}

FiniteAutomation& FiniteAutomation::removeNotCoaccessibleStates()
{
    if (flags & FlagCoaccessible)
        return *this;

    std::vector<Set<State>> prevStates (states.size());
    for (auto &transt : transitions)
        for (auto ps : transt.second)
            prevStates[ps].insert (transt.first.first);

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
        if (CONTAIN (coaccessibleStates, transt.first.first)) {
            Transition newTranst;
            newTranst.first = transt.first;
            for (State ns : transt.second)
                if (CONTAIN (coaccessibleStates, ns))
                    newTranst.second.insert (ns);
            if (!newTranst.second.empty())
                newTransitions.insert (newTranst);
        }
    }

    if (!CONTAIN (coaccessibleStates, initState))
        initState = invalid_state;

    transitions = std::move (newTransitions);
    states = std::move (coaccessibleStates);

    normalizeStateIndex();

    return *this;
}


/* template <typename C>
 * DFA<C>::DFA (Set<std::basic_string<C>> const &code) 
 * {
 * 
 *     NFA<C> nfa (code);
 * 
 * 
 *     // construct DFA
 *     //
 * 
 *     // DFA alphabet
 *     alphabet = nfa.alphabet;
 * 
 *     // DFA initState
 *     initState = nfa.initState;
 *     states.insert (initState);
 * 
 *     // DFA transitions && state
 *     std::queue<State> openSet;
 * 
 *     openSet.push (initState);
 * 
 *     Map<State, Set<State>> unitStates;
 *     unitStates[initState] = { initState };
 * 
 *     int iStateCount = nfa.states.size();
 *     int goto_initState = 0;
 *     while (!openSet.empty()) {
 *         auto currentState = openSet.front();
 *         openSet.pop();
 * 
 *         for (auto &c : alphabet) {
 *             goto_initState = 0;
 * 
 *             Set<State> nextStates;
 * 
 *             for (auto &s : unitStates[currentState]) {
 *                 auto iNTranst = nfa.transitions.find ({s, c});
 *                 if (iNTranst == nfa.transitions.end())
 *                     continue;
 *                 auto &nextNfaStates = iNTranst->second;
 *                 if (CONTAIN(nextNfaStates, 0))
 *                     if (++goto_initState == 2)
 *                         goto _exit;
 * 
 * //                if (!CONTAIN(transitions, {currentState,c})) {
 * //                    states.insert (++iStateCount);
 * //                    openSet.push (iStateCount);
 * //                    transitions[{currentState, c}] = iStateCount;
 * //                    unitStates [iStateCount] = {};
 * //                }
 * //                unitStates[transitions[{currentState,c}]]
 * //                                .insert (nextStates.begin(), nextStates.end());
 * 
 *                 nextStates.insert (nextNfaStates.begin(), nextNfaStates.end());
 *             }
 * 
 *             if (nextStates.empty())
 *                 continue;
 *             else if (nextStates.size() == 1) {
 *                 State s = *nextStates.begin();
 *                 states.insert (s);
 *                 transitions[{currentState,c}] = s;
 *                 if (!CONTAIN (unitStates, s)) {
 *                     unitStates[s] = { s };
 *                     openSet.push (s);
 *                 }
 *             } else {
 * 
 *                 bool foundOldState = false;
 *                 for (int s = nfa.states.size(); s != iStateCount; ++s) {
 *                     if (unitStates[s] == nextStates) {
 *                         transitions[{currentState,c}] = s;
 *                         foundOldState = true;
 *                         break;
 *                     }
 *                 }
 * 
 *                 if (!foundOldState) {
 *                     states.insert (iStateCount);
 *                     transitions[{currentState,c}] = iStateCount;
 *                     unitStates[iStateCount] = std::move (nextStates);
 *                     openSet.push (iStateCount);
 *                     ++iStateCount;
 *                 }
 *             }
 *         }
 *     }
 * 
 * _exit:
 *     
 *     if (goto_initState == 2)
 *         throw "Ambiguous";
 * 
 *     //
 *     // Finish states
 * 
 *     for (State s : states)
 *         if (CONTAIN (unitStates[s], initState))
 *             finishStates.insert (s);
 * }
 * 
 */

/* template <typename C>
 * DFA<C>::DFA (NFA<C> const &nfa)
 * {
 *     // construct DFA
 *     //
 * 
 *     // DFA alphabet
 *     alphabet = nfa.alphabet;
 * 
 *     // DFA initState
 *     initState = nfa.initState;
 *     states.insert (initState);
 * 
 *     // DFA transitions && state
 *     std::queue<State> openSet;
 * 
 *     openSet.push (initState);
 * 
 *     Map<State, Set<State>> unitStates;
 *     unitStates[initState] = { initState };
 * 
 *     int maxStateVal = *std::max_element (nfa.states.begin(), nfa.states.end());
 *     int iStateCount = maxStateVal + 1;
 *     while (!openSet.empty()) {
 *         auto currentState = openSet.front();
 *         openSet.pop();
 * 
 *         for (auto &c : alphabet) {
 *             Set<State> nextStates;
 * 
 *             for (auto &s : unitStates[currentState]) {
 *                 auto iNTranst = nfa.transitions.find ({s, c});
 *                 if (iNTranst == nfa.transitions.end())
 *                     continue;
 *                 auto &nextNfaStates = iNTranst->second;
 *                 nextStates.insert (nextNfaStates.begin(), nextNfaStates.end());
 *             }
 * 
 *             if (nextStates.empty())
 *                 continue;
 *             else if (nextStates.size() == 1) {
 *                 State s = *nextStates.begin();
 *                 states.insert (s);
 *                 transitions[{currentState,c}] = s;
 *                 if (!CONTAIN (unitStates, s)) {
 *                     unitStates[s] = { s };
 *                     openSet.push (s);
 *                 }
 *             } else {
 * 
 *                 bool foundOldState = false;
 *                 for (int s = maxStateVal + 1; s != iStateCount; ++s) {
 *                     if (unitStates[s] == nextStates) {
 *                         transitions[{currentState,c}] = s;
 *                         foundOldState = true;
 *                         break;
 *                     }
 *                 }
 * 
 *                 if (!foundOldState) {
 *                     states.insert (iStateCount);
 *                     transitions[{currentState,c}] = iStateCount;
 *                     unitStates[iStateCount] = std::move (nextStates);
 *                     openSet.push (iStateCount);
 *                     ++iStateCount;
 *                 }
 *             }
 *         }
 *     }
 * 
 * 
 *     //
 *     // Finish states
 * 
 *     for (State s : states)
 *         for (State f : nfa.finishStates)
 *             if (CONTAIN (unitStates[s], f)) {
 *                 finishStates.insert (s);
 *                 break;
 *             }
 * }
 * 
 */

FiniteAutomation& FiniteAutomation::trim()
{
    return removeNotCoaccessibleStates().removeInAccessibleStates();
}


FiniteAutomation& FiniteAutomation::cutByPrefix (FiniteAutomation const &prefix)
{
    removeEMoves();

    typedef std::pair<State, State> StatePair;
    std::queue<StatePair> pairsQueue;
    pairsQueue.push ({initState, prefix.initState});
    Set<StatePair> oldPairs;
    std::deque<State> cutPoints;

    auto &pTransitions = prefix.transitions;

    while (!pairsQueue.empty()) {
        auto p = pairsQueue.front();
        pairsQueue.pop();
        oldPairs.insert (p);

        if (CONTAIN (prefix.finishStates, p.second))
            cutPoints.push_back (p.first);

        for (C c : prefix.alphabet) {
            auto t1 = transitions.find ({p.first, c}); 
            auto t2 = pTransitions.find ({p.second, c});

            if (t1 == transitions.end() || t2 == pTransitions.end())
                continue;
            
            for (State ns1 : t1->second)
                for (State ns2 : t2->second) {
                    StatePair np {ns1, ns2};
                    if (!CONTAIN (oldPairs, np))
                        pairsQueue.push (np);
                }
        }
    }

    if (cutPoints.empty())
        return (*this = FiniteAutomation());
    
    if (cutPoints.size() == 1) {
        if (*cutPoints.begin() != initState) {
            initState = *cutPoints.begin();
            flags &= ~FlagAccessible;
        }
        return *this;
    }

    //
    // if >= 2 cutPoints, add new initState

    State newInitState (states.size());
    states.insert (newInitState);
    for (State s : cutPoints) {
        if (CONTAIN (finishStates, s))
            finishStates.insert (newInitState);

        for (C c : alphabet) {
            auto iNextStates = transitions.find ({s, c});
            if (iNextStates != transitions.end())
                for (State ns : iNextStates->second)
                    transitions[{newInitState, c}].insert (ns);
        }
    }
    initState = newInitState;
    flags = (flags & ~FlagAccessible & ~FlagDFA) | FlagNFA;
    return *this;
}

bool operator== (FiniteAutomation const &dfa1, FiniteAutomation const &dfa2)
{

    FiniteAutomation td1 = dfa1; 
    td1.removeNotCoaccessibleStates();
    td1.removeEMoves();
    FiniteAutomation td2 = dfa2; 
    td2.removeNotCoaccessibleStates();
    td2.removeEMoves();

    typedef FiniteAutomation::State State;
    typedef FiniteAutomation::C C;

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
                    for (State ns1 : t1->second)
                        for (State ns2 : t2->second) {
                            StatePair np = { ns1, ns2 };
                            if (!CONTAIN (oldPairs, np))
                                pairsQueue.push (np);
                        }
                }
            }
        }
    }

    return true;
}
