
/************************************************************************
        automata-test.cc

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


#include <iostream>
#include "automata.hh"

int main (int argc, char *argv[])
{
    
//    FiniteAutomation<char> fa {
//        // alphabet
//        {{ 'a', 'b' }},
//        // states
//        { 0, 1, 2},
//        //initStates
//        0,
//        // finishStates
//        {2}
//    };

    NFA<char> nfa1 (
        // alphabet
        {{ 'a', 'b' }},
        // states
        {{ 0, 1, 2}},
        //initStates
        0,
        // finishStates
        {{2}},
        // transitions
        {{
            {{0, 'a'}, {1}},
            {{1, 'a'}, {1,2}},
            {{1, 'b'}, {1}},
            {{2, 'b'}, {2}}
        }}
    );
    
    NFA<char> nfa2 (
        // alphabet
        { 'a', 'b' },
        // states
        { 0, 1, 2},
        //initStates
        0,
        // finishStates
        {2},
        // transitions
        {
            {{0, 'a'}, {1}},
            {{1, 'a'}, {1}},
            {{1, 'b'}, {1,2}},
        }
    );

    std::cout << nfa1.recognizeEmptyString() << '\n';
    std::cout << nfa2.recognizeEmptyString() << '\n';

    DFA<char> dfa1 (nfa1), dfa2 (nfa2);

    std::cout << (dfa1 == dfa2) << '\n';

    return 0;
}
