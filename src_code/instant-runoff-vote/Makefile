CXX = g++
CXXFLAGS = -std=c++17 -Wall
OBJECTS = main.o instant-runoff-votes.o
TARGET = main

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJECTS)

main.o: main.cc instant-runoff-votes.h
	$(CXX) $(CXXFLAGS) -c main.cc instant-runoff-votes.h

instant-runoff-votes.o: instant-runoff-votes.cc instant-runoff-votes.h
	$(CXX) $(CXXFLAGS) -c instant-runoff-votes.cc instant-runoff-votes.h

clean:
	rm $(OBJECTS) $(TARGET) *.gch


