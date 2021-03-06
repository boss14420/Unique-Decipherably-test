
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
#include <stack>
#include <algorithm>
#include <tuple>
#include <rapidxml.hpp>
#include <cstring>
#include <fstream>
#include "automata.hh"
#include "util.hpp"

#define CONTAIN(c,k) (c.find (k) != c.end())

const FiniteAutomaton::C FiniteAutomaton::empty_letter = C() - 2;

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

char *readTextFile (char const *filename) {
    std::ifstream file (filename, std::ios::in | std::ios::binary | std::ios::ate);
    if (file.is_open()) {
        std::size_t size = file.tellg();
        char *contents = new char[size];
        file.seekg (0, std::ios::beg);
        file.read (contents, size);
        file.close ();

        return contents;
    }

    return NULL;
}

FiniteAutomaton::FiniteAutomaton (char const *filename)
{
    rapidxml::xml_document<C> xml;
    char *text = readTextFile (filename);
    xml.parse<0> (text);

    rapidxml::xml_node<C>* first_node = xml.first_node ("structure");
    if (!first_node)
        throw XmlParseFailed();

    // check fa
    auto type_node = first_node->first_node();
    if (!type_node || std::strcmp (type_node->name(), "type")
            || std::strcmp (type_node->value(), "fa"))
        throw XmlParseFailed();


    //
    // construct
    auto automaton = type_node->next_sibling();
    if (!automaton || std::strcmp (automaton->name(), "automaton"))
        throw XmlParseFailed();

    // states
    initState = invalid_state;
    Map <int, State> stateIds;
    State currentStateId = 0;
    auto node = automaton->first_node();
    for (;node && !std::strcmp (node->name(), "state"); 
                                    node = node->next_sibling()) 
    {
        if (std::strcmp (node->first_attribute()->name(), "id"))
            throw XmlParseFailed();

        stateIds[std::atoi (node->first_attribute()->value())] = currentStateId;
        states.insert (currentStateId);

        if (node->first_node ("initial")) {
            if (initState != invalid_state)
                throw XmlParseFailed ("Too many init states");
            initState = currentStateId;
        } 
        if (node->first_node ("final"))
            finishStates.insert (currentStateId);

        ++currentStateId;
    }

    // transitions
    flags = FlagDFA;
    for (;node && !std::strcmp (node->name(), "transition");
                                            node = node->next_sibling())
    {
        State from, to;
        C c;
        
        rapidxml::xml_node<> *cnode;

        if ( !(cnode = node->first_node ("from")) || !cnode->value() )
            throw XmlParseFailed();
        from = stateIds[std::atoi (cnode->value())];

        if ( !(cnode = node->first_node ("to")) || !cnode->value() )
            throw XmlParseFailed();
        to = stateIds[std::atoi (cnode->value())];

        if ( !(cnode = node->first_node ("read")) || !cnode->value() )
            throw XmlParseFailed();
        c = *cnode->value();
        if (c == '\0') {
            c = empty_letter;
            flags |= FlagHasEMove;
        } else
            alphabet.insert (c);
        
        transitions[{from, c}].insert (to);
        if (transitions[{from, c}].size() > 1)
            flags = (flags & ~FlagDFA) | FlagNFA;
    }

    delete[] text;
}

bool FiniteAutomaton::recognizeEmptyString() const
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

bool FiniteAutomaton::recognizeOnlyEmptyString() const
{
    if (!recognizeEmptyString()) return false;

    if (!(flags & FlagHasEMove)) {
        Set<State> accessibleStates {initState};
        std::stack<State> statesStack;
        statesStack.push (initState);

        while (!statesStack.empty()) {
            State s = statesStack.top();
            statesStack.pop();

            for (C c : alphabet) {
                auto iNext = transitions.find ({s, c});
                if (iNext != transitions.end())
                    for (State ns : iNext->second) {
                        if (CONTAIN (finishStates, ns))
                            return false;
                        if (!CONTAIN (accessibleStates, ns)) {
                            accessibleStates.insert (ns);
                            statesStack.push (ns);
                        }
                    }
            }
        }

        return true;

    } else {
        // TODO: when has e-moves
        throw MethodNotYetImplement ("FiniteAutomaton::recognizeOnlyEmptyString()");
    }
}

bool FiniteAutomaton::isEmpty() const
{

    if (finishStates.empty())
        return true;

    if (flags & FlagCoaccessible)
        return false;

    std::vector<Set<State>> prevStates (states.size());
    for (auto &transt : transitions)
        for (auto ps : transt.second)
            prevStates[ps].insert (transt.first.first);

    Set<State> coaccessibleStates = finishStates;
    std::stack<State> statesStack;
    for (State f : coaccessibleStates) statesStack.push (f);

    while (!statesStack.empty()) {
        State s = statesStack.top();
        statesStack.pop();

        if (s == initState)
            return false;

        for (State ns : prevStates[s])
            if (!CONTAIN (coaccessibleStates, ns)) {
                coaccessibleStates.insert (ns);
                statesStack.push (ns);
            }
    }

    return true;
}

FiniteAutomaton& FiniteAutomaton::excludeEmptyString() 
{
    if (!recognizeEmptyString())
        return *this;
    
    if (flags & FlagHasEMove)
        removeEMoves();

    std::deque<C> charsToInitState;
    for (C c : alphabet)
        if (CONTAIN (transitions, std::make_pair (initState, c))
                && CONTAIN ((transitions[{initState, c}]), initState))
            charsToInitState.push_back (c);
    
    if (charsToInitState.empty()) {
        finishStates.erase (initState);
    } else {
        State newInitState (states.size());
        for (C c : charsToInitState)
            transitions[{newInitState,c}] = {initState};
        initState = newInitState;
        states.insert (newInitState);
    }

    return *this;
}


Set<FiniteAutomaton::State> FiniteAutomaton::eClosure (State s) const
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

Set<FiniteAutomaton::State> 
FiniteAutomaton::eClosure (Set<State> const &ss) const
{
    Set<State> result;
    for (State s : ss) {
        Set<State> ec = eClosure (s);
        result.insert (ec.begin(), ec.end());
    }
    return result;
}

FiniteAutomaton& FiniteAutomaton::removeEMoves()
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
    for (auto &tf : transtsFrom[initState])
        for (State ns : tf.second)
            W.push (std::make_tuple (initState, tf.first, ns));

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

    flags &= ~FlagHasEMove;

    return *this;
}


FiniteAutomaton& FiniteAutomaton::normalizeStateIndex()
{
    State maxIndex = *std::max_element (states.begin(), states.end());
    if (maxIndex == states.size())
        return *this;

    std::vector<State> newIndex (maxIndex);
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

FiniteAutomaton& FiniteAutomaton::removeInAccessibleStates()
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
        for (State ps : nextStates[s])
            if (!CONTAIN (accessibleStates, ps)) {
                statesQueue.push (ps);
                accessibleStates.insert (ps);
            }
    }

    // remove inaccessible states' paths
    auto iTranst = transitions.begin();
    while (iTranst != transitions.end()) {
        if (!CONTAIN (accessibleStates, iTranst->first.first)) {
            iTranst = transitions.erase (iTranst);
        } else {
            auto &nextStates = iTranst->second;
            auto iNextStates = nextStates.begin();
            while (iNextStates != nextStates.end()) {
                if (!CONTAIN (accessibleStates, *iNextStates))
                    iNextStates = nextStates.erase (iNextStates);
                else
                    ++iNextStates;
            }
            ++iTranst;
        }
    }


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

FiniteAutomaton& FiniteAutomaton::removeNotCoaccessibleStates()
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
        for (State ps : prevStates[s])
            if (!CONTAIN (coaccessibleStates, ps)) {
                statesQueue.push (ps);
                coaccessibleStates.insert (ps);
            }
    }

    if (coaccessibleStates.size() == states.size()) {
        flags |= FlagCoaccessible;
        return *this;
    }

    // remove not-coaccessible states' paths
    auto iTranst = transitions.begin();
    while (iTranst != transitions.end()) {
        if (!CONTAIN (coaccessibleStates, iTranst->first.first)) {
            iTranst = transitions.erase (iTranst);
        } else {
            auto &nextStates = iTranst->second;
            auto iNextStates = nextStates.begin();
            while (iNextStates != nextStates.end()) {
                if (!CONTAIN (coaccessibleStates, *iNextStates))
                    iNextStates = nextStates.erase (iNextStates);
                else
                    ++iNextStates;
            }
            ++iTranst;
        }
    }

    if (!CONTAIN (coaccessibleStates, initState))
        initState = invalid_state;

    states = std::move (coaccessibleStates);

    normalizeStateIndex();
    
    flags |= FlagCoaccessible;
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

FiniteAutomaton& FiniteAutomaton::trim()
{
    return removeNotCoaccessibleStates().removeInAccessibleStates();
}

FiniteAutomaton& FiniteAutomaton::klene()
{
    for (State f : finishStates) {
        transitions[{f, empty_letter}].insert (initState);
    }
    flags |= FlagHasEMove;

    return *this;
}

FiniteAutomaton& FiniteAutomaton::cutBySuffix (FiniteAutomaton suffix)
{
    removeEMoves();
    suffix.removeEMoves();
        
    typedef std::pair<State, State> StatePair;
    Set<State> cutPoints;

    auto &sTransitions = suffix.transitions;

    for (State s : states) {
        Set<StatePair> oldPairs;
        std::stack<StatePair> pairsStack;
        pairsStack.push ({s, suffix.initState});

        while (!pairsStack.empty()) {
            auto p = pairsStack.top();
            pairsStack.pop();

            for (C c : suffix.alphabet) {
                auto t1 = transitions.find ({p.first, c});
                auto t2 = sTransitions.find ({p.second, c});

                if (t1 == transitions.end() || t2 == sTransitions.end())
                    continue;

                for (State ns1 : t1->second)
                    for (State ns2 : t2->second) {
                        StatePair np {ns1, ns2};
                        if (!CONTAIN (oldPairs, np)) {
                            if (CONTAIN (finishStates, ns1)
                                && CONTAIN (suffix.finishStates, ns2)) 
                            {
                                cutPoints.insert (s);
                                goto test_next_state;
                            }
                            oldPairs.insert (np);
                            pairsStack.push (np);
                        }
                    }
            }
        }
test_next_state:
        ++s; 
    }

    if (cutPoints.empty())
        return (*this = FiniteAutomaton());
    
    finishStates = std::move (cutPoints);
    flags &= ~FlagCoaccessible;
    return *this;
}

FiniteAutomaton& FiniteAutomaton::cutByPrefix (FiniteAutomaton prefix)
{
    removeEMoves();
    prefix.removeEMoves();

    typedef std::pair<State, State> StatePair;
    std::queue<StatePair> pairsQueue;
    pairsQueue.push ({initState, prefix.initState});
    Set<StatePair> oldPairs { {initState, initState} };
    Set<State> cutPoints;

    auto &pTransitions = prefix.transitions;

    while (!pairsQueue.empty()) {
        auto p = pairsQueue.front();
        pairsQueue.pop();

        if (CONTAIN (prefix.finishStates, p.second))
            cutPoints.insert (p.first);

        for (C c : prefix.alphabet) {
            auto t1 = transitions.find ({p.first, c}); 
            auto t2 = pTransitions.find ({p.second, c});

            if (t1 == transitions.end() || t2 == pTransitions.end())
                continue;
            
            for (State ns1 : t1->second)
                for (State ns2 : t2->second) {
                    StatePair np {ns1, ns2};
                    if (!CONTAIN (oldPairs, np)) {
                        pairsQueue.push (np);
                        oldPairs.insert (np);
                    }
                }
        }
    }

    if (cutPoints.empty())
        return (*this = FiniteAutomaton());
    
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

Set<FiniteAutomaton::State>
FiniteAutomaton::nextStates (Set<FiniteAutomaton::State> const &ss, C c) const
{
    Set<State> res;
    for (State s : ss) {
        auto iNext = transitions.find ({s, c});
        if (iNext != transitions.end())
            res.insert (iNext->second.begin(), iNext->second.end());
    } 

    return res;
}

bool operator== (FiniteAutomaton const &dfa1, FiniteAutomaton const &dfa2)
{

    // TODO when use different alphabet

    if (dfa1.alphabet != dfa2.alphabet)
        throw MethodNotYetImplement ("operator==");

    FiniteAutomaton td1 = dfa1; 
    td1.removeNotCoaccessibleStates();
    td1.removeEMoves();
    FiniteAutomaton td2 = dfa2; 
    td2.removeNotCoaccessibleStates();
    td2.removeEMoves();


    typedef FiniteAutomaton::State State;
    typedef FiniteAutomaton::C C;

    typedef std::pair<Set<State>, Set<State>> StatePair;
    std::queue<StatePair> pairsQueue;
    pairsQueue.push ({{td1.initState}, {td2.initState}});
    Set<StatePair> oldPairs;

    while (!pairsQueue.empty()) {
        auto p = pairsQueue.front();
        pairsQueue.pop();

        int finish1 = 0, finish2 = 0;
        
        for (State f : td1.finishStates)
            if (CONTAIN (p.first, f)) {
                finish1 = 1;
                break;
            }

        for (State f : td2.finishStates)
            if (CONTAIN (p.second, f)) {
                finish2 = 1;
                break;
            }

        if ((finish1 + finish2) == 1)
            return false;

        for (C c : td1.alphabet) {
            Set<State> ns1 = td1.nextStates (p.first, c);
            Set<State> ns2 = td2.nextStates (p.second, c);

            if (  (ns1.size() > 0 && ns2.size() == 0)
                ||(ns1.size() == 0 && ns2.size() > 0))
                return false;

            StatePair np { std::move (ns1), std::move (ns2) };

            if (!CONTAIN (oldPairs, np)) {
                oldPairs.insert (np);
                pairsQueue.push (std::move (np)); 
            }
        }
    }

    return true;
}


void FiniteAutomaton::writeToXmlFile (char const *filename) 
{
    std::ofstream file (filename, std::ios::out | std::ios::trunc);
    if (!file.is_open())
        return;

    file << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n"
            "<structure>\n\t<type>fa</type>\n\t<automaton>\n";
    
    float width = 600, x = 0, y = 100;
    float gap = width / (states.size() + 2);
    for (State s : states) {
        file << "\t\t<state id=\"" << s << "\" name=\"q" << s << "\">\n"
                "\t\t\t<x>" << (x+=gap) << "</x>\n"
                "\t\t\t<y>" << y << "</y>\n";
        if (s == initState)
            file << "\t\t\t<initial/>\n";
        if (CONTAIN (finishStates, s))
            file << "\t\t\t<final/>\n";
        file << "\t\t</state>\n";
    }

    for (auto &transts : transitions) {
        State q1 = transts.first.first;
        C c = transts.first.second;
        for (State q2 : transts.second) {
            file << "\t\t<transition>\n"
                    "\t\t\t<from>" << q1 << "</from>\n"
                    "\t\t\t<to>" << q2 << "</to>\n";
            if (c == empty_letter)
                file << "\t\t\t<read/>\n";
            else
                file << "\t\t\t<read>" << c << "</read>\n";
            file << "\t\t</transition>\n";
        }
    }

    file << "\t</automaton>\n</structure>";

    file.close();
}


FiniteAutomaton intersectAutomata (FiniteAutomaton fa1, FiniteAutomaton fa2)
{
    // TODO: different alphabet

    if (fa1.alphabet != fa2.alphabet)
        throw MethodNotYetImplement("intersectAutomata()");

    fa1.removeEMoves();
    fa2.removeEMoves();

    FiniteAutomaton res;
    res.flags = FiniteAutomaton::FlagNFA;
    res.alphabet = fa1.alphabet;

    typedef FiniteAutomaton::State State;
    typedef std::pair<State, State> StatePair;
    
    State iStateCount = 0;
    Map<StatePair, State> pairIndex;
    pairIndex[{fa1.initState, fa2.initState}] = iStateCount;
    res.initState = iStateCount;

    std::queue<StatePair> pairsQueue;
    pairsQueue.push ({fa1.initState, fa2.initState});

    while (!pairsQueue.empty()) {
        auto p = pairsQueue.front();
        pairsQueue.pop();
        State id = pairIndex[p];

        if (CONTAIN (fa1.finishStates, p.first) 
                && CONTAIN (fa2.finishStates, p.second))
        {
            res.finishStates.insert (id);
        }

        for (FiniteAutomaton::C c : fa1.alphabet) {
            auto t1 = fa1.transitions.find ({p.first, c});
            auto t2 = fa2.transitions.find ({p.second, c});
            
            if (t1 == fa1.transitions.end() || t2 == fa2.transitions.end())
                continue;

            for (State ns1 : t1->second)
                for (State ns2 : t2->second) {
                    StatePair np { ns1, ns2 };
                    if (!CONTAIN (pairIndex, np)) {
                        pairsQueue.push (np);
                        pairIndex[np] = ++iStateCount;
                    }
                    res.transitions[{id, c}].insert (pairIndex[np]);
                }
        }
    }

    return res;
}
