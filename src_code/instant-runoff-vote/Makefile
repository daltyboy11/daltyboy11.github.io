CXX = g++
CXXFLAGS = -std=c++17 -Wall
OBJECTS = main.o instant-runoff-votes.o
TARGET = main

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJECTS)

main.o: main.cc instant-runoff-votes.hpp
	$(CXX) $(CXXFLAGS) -c main.cc instant-runoff-votes.hpp

instant-runoff-votes.o: instant-runoff-votes.cc instant-runoff-votes.hpp
	$(CXX) $(CXXFLAGS) -c instant-runoff-votes.cc instant-runoff-votes.hpp

clean:
	rm $(OBJECTS) $(TARGET) *.gch


