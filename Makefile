CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g
INCLUDES = -Iincludes

TARGET = order_book
TEST_TARGET = test_edge_cases

SRC_DIR = src
BUILD_DIR = build
TEST_DIR = tests

SRCS := main.cpp $(shell find $(SRC_DIR) -name "*.cpp")
OBJS := $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
OBJ_DIRS := $(sort $(dir $(OBJS)))

# Sources pour les tests (sans main.cpp)
TEST_SRCS := $(shell find $(SRC_DIR) -name "*.cpp" ! -name "main.cpp")
TEST_OBJS := $(TEST_SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

all: directories $(TARGET)

directories:
	@mkdir -p $(OBJ_DIRS)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Nouvelle règle pour les tests
$(TEST_TARGET): directories $(TEST_OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $(TEST_OBJS) $(TEST_DIR)/MatchingEngine/testsMatchingEngine.cpp



test: $(TEST_TARGET)
	./$(TEST_TARGET)

clean:
	rm -rf $(BUILD_DIR) $(TARGET) $(TEST_TARGET)

re: clean all

run: all
	./$(TARGET)

.PHONY: all clean run test