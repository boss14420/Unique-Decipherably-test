all: udtest-automata udtest-tail automata-test
#all: automata-test

CXX = g++
CXXFLAGS = -g -Wall -Wno-mismatched-tags -std=c++11
INCLUDE = -I.

udtest-tail: udtest-tail.cc
	$(CXX) $(CXXFLAGS) $(INCLUDE) -o $@ $<

udtest-automata: udtest-automata.cc automata.cc.o
	$(CXX) $(CXXFLAGS) $(INCLUDE) -o $@ $^

automata-test: automata-test.cc automata.cc.o
	$(CXX) $(CXXFLAGS) $(INCLUDE) -o $@ $^

automata.cc.o: automata.cc automata.hh util.hpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

clean: clean-tail clean-automata clean-automata-test

clean-tail:
	rm udtest-tail

clean-automata:
	rm *.o

clean-automata-test:
	rm *.o
