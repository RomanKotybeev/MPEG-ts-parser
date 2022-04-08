CXX = g++
CXXFLAGS = -Wall -g
OBJMODULES = Parser.o StreamSections.o TS_Packet.o

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

parser: main.cpp $(OBJMODULES)
	$(CXX) $(CXXFLAGS) $^ -o $@

clean:
	rm -f *.o parser
