all: udtest-automata udtest-tail automata-test

CXX = g++
CXXFLAGS = -g -Wall -std=c++11
INCLUDE = -I.

udtest-tail: udtest-tail.cc
	$(CXX) $(CXXFLAGS) $(INCLUDE) -o $@ $<

udtest-automata: udtest-automata.cc automata.hh automata.cc
	$(CXX) $(CXXFLAGS) $(INCLUDE) -o $@ $<

automata-test: automata-test.cc automata.hh automata.cc
	$(CXX) $(CXXFLAGS) $(INCLUDE) -o $@ $<


clean: clean-tail clean-automata clean-automata-test

clean-tail:
	rm udtest-tail

clean-automata:
	rm udtest-automata

clean-automata-test:
	rm automata-test
