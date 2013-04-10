all: udtest-automata udtest-tail

CXX = g++
CXXFLAGS = -g -Wall -std=c++11
INCLUDE = -I.

udtest-tail: udtest-tail.cc
	$(CXX) $(CXXFLAGS) $(INCLUDE) -o $@ $<

udtest-automata: udtest-automata.cc automata.hh automata.cc
	$(CXX) $(CXXFLAGS) $(INCLUDE) -o $@ $<

clean: clean-tail clean-automata

clean-tail:
	rm udtest-tail

clean-automata:
	rm udtest-automata
