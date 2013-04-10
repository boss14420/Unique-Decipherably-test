
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
#include "automata.hh"

template <typename T> using Set = std::unordered_set<T>;

bool is_ud (Set<std::string> const &code) {

    //
    // construct DFA

    Set<char> alphabet;
    for (auto &w : code)
        for (auto &c : w)
            alphabet.insert (c);



    return true;
}

int main () {
    Set<std::string> code;
    std::string c;
    while ( ! (std::cin >> c).eof() )
        code.insert (c);

    std::cout << is_ud (code) << '\n';

    return 0;
}
