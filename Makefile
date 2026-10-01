CXX := c++
CXXFLAGS := -std=c++17 -Wall -Wextra -pedantic

TARGET := tsh
SOURCES := src/tsh.cpp src/parser.cpp src/command_runner.cpp

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET)

clean:
	rm -f $(TARGET)
