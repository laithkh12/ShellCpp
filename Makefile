CXX = g++
CXXFLAGS = -std=c++11 -Wall

TARGET = shell

OBJS = main.o Shell.o

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

main.o: main.cpp Shell.h
	$(CXX) $(CXXFLAGS) -c main.cpp

Shell.o: Shell.cpp Shell.h
	$(CXX) $(CXXFLAGS) -c Shell.cpp

clean:
	rm -f $(TARGET) $(OBJS)