CXX = g++
CXXFLAGS = -std=c++20 -O3 -Wall -Wextra -Iheaders

SRC = $(wildcard src/*.cpp)
OBJ = $(patsubst src/%.cpp, build/%.o, $(SRC))
BUILD_DIR = build
SETUP_DIR = /usr/local/bin/
TARGET = hedgehog

all: $(TARGET)

setup: $(TARGET)
	sudo cp -r $(BUILD_DIR)/$(TARGET) $(SETUP_DIR)

run: $(TARGET)
	./$(BUILD_DIR)/$(TARGET)

clean:
	rm -rf $(BUILD_DIR)

$(TARGET): $(OBJ) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $(BUILD_DIR)/$(TARGET) $(OBJ)

$(BUILD_DIR)/%.o: src/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)


