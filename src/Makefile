CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17
TARGET = server

all: $(TARGET)

$(TARGET): server.cpp
	$(CXX) $(CXXFLAGS) server.cpp -o $(TARGET)

clean:
	rm -f $(TARGET)