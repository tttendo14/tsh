CXX := c++
CXXFLAGS := -std=c++23 -Wall -Wextra -pedantic
# -MMD: while compiling foo.cpp, also write build/foo.d listing every header it includes.
# -MP:  add an empty rule per header, so deleting a header doesn't break the build.
DEPFLAGS := -MMD -MP

TARGET := tsh
BUILD_DIR := build
SOURCES := $(wildcard src/*.cpp)
OBJECTS := $(SOURCES:src/%.cpp=$(BUILD_DIR)/%.o)
DEPS := $(OBJECTS:.o=.d)

.PHONY: all clean

all: $(TARGET)

# Link: runs only when some object file is newer than the binary.
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Compile: one .cpp -> one .o (+ its .d). `| $(BUILD_DIR)` is an order-only
# prerequisite: the directory must exist, but its timestamp never triggers a rebuild.
$(BUILD_DIR)/%.o: src/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

# Pull in the generated header dependencies. The leading '-' ignores missing
# files, which is the case on the very first build.
-include $(DEPS)
