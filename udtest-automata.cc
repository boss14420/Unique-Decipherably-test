
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

bool is_ud (FiniteAutomation const &code) {
    std::deque<FiniteAutomation> S, SS;
    FiniteAutomation fa = code;
    fa.cutByPrefix (code).excludeEmptyString();

    S.push_back (fa), SS.push_back(fa);

    while (!S.empty()) {

        //
        // calculate S1 = S^-1 code U code^-1 S
        std::deque<FiniteAutomation> S1;
        for (auto fa : S) {
            if (fa.recognizeEmptyString())
                return false;

            FiniteAutomation code0 = code;
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

int main () {
    
    // code = ab*
/*     FiniteAutomation code (
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
 *         FiniteAutomation::FlagDFA | FiniteAutomation::FlagAccessible
 *         | FiniteAutomation::FlagCoaccessible
 *     );
 */


    
    // code = a*ba*
    FiniteAutomation code (
        // alphabet
        {{ 'a', 'b' }},
        // states
        {{ 0, 1 }},
        // initState
        0,
        // finishStates
        {1},
        // transitions
        {
            {{0, 'a'}, {0}},
            {{0, 'b'}, {1}},
            {{1, 'a'}, {1}}
        },
        // flags
        FiniteAutomation::FlagDFA | FiniteAutomation::FlagAccessible
        | FiniteAutomation::FlagCoaccessible
    );


    std::cout << is_ud (code) << '\n';

    return 0;
}
