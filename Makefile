CXX := g++
CXXFLAGS := -g -O2

all: wesley

wesley: wesley.o
	$(CXX) $(CXXFLAGS) wesley.o -o wesley

wesley.o: wesley.cpp
	$(CXX) $(CXXFLAGS) -c wesley.cpp -o wesley.o

clean:
	rm -r *.o
