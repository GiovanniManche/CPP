CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g
INCLUDES = -Iincludes

TARGET = order book

SRC_DIR = src
BUILD_DIR = build

SRCS := $(shell find $(SRC_DIR) -name "*.cpp")
OBJS := $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
OBJ_DIRS := $(sort $(dir $(OBJS)))

all: directories $(TARGET)

directories:
	@mkdir -p $(OBJ_DIRS)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

re: clean all

run: all
	./$(TARGET)

.PHONY: all clean run