
/************************************************************************
        udtest-automata.cc

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

#include <deque>
#include <string>
#include <cstring>
#include <unordered_set>
#include <iostream>
#include <algorithm>
#include <fstream>
#include "automata.hh"

/* bool is_ud (Set<std::string> const &code) {
* 
*     try {
*         DFA<char> dfa (code);
*     } catch (...) {
*         return false;
*     }
* 
*     return true;
* }
* 
*/

bool is_ud (FiniteAutomaton const &code) {
    FiniteAutomaton fa1 = code;
    fa1.cutByPrefix (fa1);

    FiniteAutomaton fa2 = code;
    fa2.klene();
    fa2.removeEMoves();
    fa2.cutBySuffix (fa2);

    return intersectAutomata (fa1, fa2).recognizeOnlyEmptyString ();
}

bool is_ud_sardinas_patterson (FiniteAutomaton code) {
    std::deque<FiniteAutomaton> S, SS;
    code.removeEMoves();
    FiniteAutomaton fa = code;
    fa.cutByPrefix (code).excludeEmptyString();

    S.push_back (fa), SS.push_back(fa);

    while (!S.empty()) {

        //
        // calculate S1 = S^-1 code U code^-1 S
        std::deque<FiniteAutomaton> S1;
        for (auto fa : S) {
            if (fa.recognizeEmptyString())
                return false;

            FiniteAutomaton code0 = code;
            if (!code0.cutByPrefix (fa).isEmpty())
                S1.push_back (code0);
            if (!fa.cutByPrefix (code).isEmpty())
                S1.push_back (fa);
        }

        //
        // calculate S = S1 \ SS
        S.clear();
        for (auto &fa : S1)
            if (std::find (SS.begin(), SS.end(), fa) == SS.end())
                S.push_back (fa);

        //
        // calculate SS = SS U S
        for (auto &fa : S)
            SS.push_back (fa);
    }

    return true;
}

int main (int argc, char *argv[]) {
    
    // code = ab*
/*     FiniteAutomaton code (
 *         // alphabet
 *         {{ 'a', 'b' }},
 *         // states
 *         {{ 0, 1 }},
 *         // initState
 *         0,
 *         // finishStates
 *         {1},
 *         // transitions
 *         {
 *             {{0, 'a'}, {1}},
 *             {{1, 'b'}, {1}}
 *         },
 *         // flags
 *         FiniteAutomaton::FlagDFA | FiniteAutomaton::FlagAccessible
 *         | FiniteAutomaton::FlagCoaccessible
 *     );
 */

    // code = {a, ab, ba}
//    FiniteAutomaton code (
//        // alphabet
//        {{ 'a', 'b' }},
//        // states
//        {{ 0, 1, 2, 3 }},
//        // initState
//        0,
//        // finishStates
//        {1, 2},
//        // transitions
//        {
//            {{0, 'a'}, {1}},
//            {{1, 'b'}, {2}},
//            {{0, 'b'}, {3}},
//            {{3, 'a'}, {2}}
//        },
//        // flags
//        FiniteAutomaton::FlagDFA | FiniteAutomaton::FlagAccessible
//        | FiniteAutomaton::FlagCoaccessible
//    );


    FiniteAutomaton code (argv[1]);

    std::cout << is_ud (code) << is_ud_sardinas_patterson (code) << '\n';

    return 0;
}
