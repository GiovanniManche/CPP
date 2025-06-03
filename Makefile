CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g
INCLUDES = -Iincludes

TARGET = build/order_book
MATCHING_ENGINE_TEST_TARGET = build/tests/MatchingEngine/test_matching_engine
CSVREADER_TEST_TARGET = build/tests/CSVReader/test_csv_reader
OUTPUT_TEST_TARGET = build/tests/SimpleOutputs/test_outputs

SRC_DIR = src
BUILD_DIR = build
TEST_DIR = tests

# Sources séparées : main.cpp à la racine + sources dans src/
MAIN_SRC := main.cpp
SRC_FILES := $(shell find $(SRC_DIR) -name "*.cpp")
MAIN_OBJ := $(BUILD_DIR)/main.o
SRC_OBJS := $(SRC_FILES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
OBJS := $(MAIN_OBJ) $(SRC_OBJS)
OBJ_DIRS := $(sort $(dir $(OBJS)))

# Sources pour les tests (sans main.cpp)
TEST_SRCS := $(shell find $(SRC_DIR) -name "*.cpp" ! -name "main.cpp")
TEST_OBJS := $(TEST_SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

all: directories $(TARGET)

directories:
	@mkdir -p $(OBJ_DIRS)
	@mkdir -p build/tests/MatchingEngine
	@mkdir -p build/tests/CSVReader
	@mkdir -p build/tests/SimpleOutputs

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^

# Règle spéciale pour main.cpp à la racine
$(BUILD_DIR)/main.o: main.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Règle pour les sources dans src/
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Tests des cas limites du matching engine
$(MATCHING_ENGINE_TEST_TARGET): directories $(TEST_OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $(TEST_OBJS) $(TEST_DIR)/MatchingEngine/testsMatchingEngine.cpp

test_matching_engine: $(MATCHING_ENGINE_TEST_TARGET)
	./$(MATCHING_ENGINE_TEST_TARGET)

# Tests d'outputs simples
$(OUTPUT_TEST_TARGET): directories $(TEST_OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $(TEST_OBJS) $(TEST_DIR)/SimpleOutputs/testsOutputs.cpp

test_outputs: $(OUTPUT_TEST_TARGET)
	./$(OUTPUT_TEST_TARGET)

# Tests du CSV Reader
$(CSVREADER_TEST_TARGET): directories $(TEST_OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $(TEST_OBJS) $(TEST_DIR)/CSVReader/testsCSVReader.cpp

test_csv_reader: $(CSVREADER_TEST_TARGET)
	./$(CSVREADER_TEST_TARGET)

# Lancer tous les tests
test_all: test_matching_engine test_outputs test_csv_reader

clean:
	rm -rf $(BUILD_DIR)

re: clean all

run: all
	./$(TARGET)

.PHONY: all clean run test_matching_engine test_outputs test_csv_reader test_all directories re