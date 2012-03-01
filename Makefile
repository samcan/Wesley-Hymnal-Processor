CXX := g++
CXXFLAGS := -g -O2

all: wesley

wesley: wesley.o
	$(CXX) $(CXXFLAGS) wesley.o -lboost_program_options -o wesley

wesley.o: wesley.cpp
	$(CXX) $(CXXFLAGS) -c -I/usr/include/boost -o wesley.o wesley.cpp

clean:
	rm -r *.o
