
/************************************************************************
        udtest-tail.cc

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

template <typename T> using Set = std::unordered_set<T>;

bool is_ud (Set<std::string> const &code) {
    std::deque<Set<std::string>> S (1);
    S[0] = code;

    for (int i = 1; !S[i-1].empty() ; ++i) {
        S.resize (i+1);
        for (auto &w1 : S[0]) {
            for (auto &w2 : S[i-1]) {
                if (w1.length() == w2.length())
                    continue;

                std::string v;
                if (w1.length() > w2.length() && 
                            !std::strncmp (w1.data(), w2.data(), w2.length()))
                    v = w1.substr (w2.length());
                else if (w1.length() < w2.length() &&
                            !std::strncmp (w1.data(), w2.data(), w1.length()))
                    v = w2.substr (w1.length());
                
                if (!v.empty()) {
                    if (code.find (v) != code.end())
                        return false;

                    bool old_tail = false;
                    for (int j = 1; j < i; ++j)
                        if (S[j].find (v) != S[j].end()) {
                            old_tail = true;
                            break;
                        }

                    if (!old_tail)
                        S[i].insert (v);
                }
            }
        }
    }

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