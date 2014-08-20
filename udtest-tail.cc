
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

namespace std {
    template<>
    struct hash<Set<std::string>> {
        size_t operator() (Set<std::string> const &ss) const {
            size_t seed = 0;
            auto h = std::hash<std::string>();
            for (auto const &s : ss)
                // boost::hash_combine()
                seed ^= h(s) + 0x9e3779b9 + (seed << 6) + (seed >> 2);

            return seed;
        }
    };
}

bool is_ud (Set<std::string> const &code) {
    Set<Set<std::string>> S;
    S.insert(code);
    auto oldSuffix = code;
    Set<std::string> newSuffix;

    while(!oldSuffix.empty()) {
        for (auto const &w1 : code) {
            for (auto const &w2 : oldSuffix) {
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
                    else
                        newSuffix.insert(v);
                }
            }
        }

        Set<Set<std::string>>::iterator si;
        bool inserted;
        std::tie(si, inserted) = S.insert(newSuffix);
        if (!inserted) return true;
        oldSuffix = std::move(newSuffix);  
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
