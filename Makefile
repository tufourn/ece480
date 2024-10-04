CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++11

SRC = main.cpp
TARGET = build/main
BUILD_DIR = build

all: $(BUILD_DIR) $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
