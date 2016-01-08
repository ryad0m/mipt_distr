CXX= g++
CXX_FLAGS= -O2 -Wall -fopenmp
LD_FLAGS= -lpthread

all: main

main: main.cpp
	$(CXX) $(CXX_FLAGS) $(LD_FLAGS) -o $@ $^

run: main
	./main

clean:
	rm -f main

.PHONY: main run
