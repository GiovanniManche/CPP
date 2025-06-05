CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g
INCLUDES = -Iincludes

# Targets
TARGET = build/order_book
MATCHING_ENGINE_TEST_TARGET = build/tests/MatchingEngine/test_matching_engine
CSVREADER_TEST_TARGET = build/tests/CSVReader/test_csv_reader
OUTPUT_TEST_TARGET = build/tests/SimpleOutputs/test_outputs
PERF_TEST_TARGET = build/tests/Performance/test_performance

# Directories
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

# Default target
all: directories $(TARGET)

# Create necessary directories
directories:
	@mkdir -p $(OBJ_DIRS)
	@mkdir -p build/tests/MatchingEngine
	@mkdir -p build/tests/CSVReader
	@mkdir -p build/tests/SimpleOutputs
	@mkdir -p build/tests/Performance

# Main executable
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

###########################################################################################################
# TESTS UNITAIRES
###########################################################################################################

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

# Lancer tous les tests unitaires (SANS les tests de performance)
test_all: test_matching_engine test_outputs test_csv_reader

# ###########################################################################################################
# TESTS DE PERFORMANCE 
# ###########################################################################################################

# Tests de performance du matching engine
$(PERF_TEST_TARGET): directories $(TEST_OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $(TEST_OBJS) $(TEST_DIR)/performance/performanceMetrics.cpp

test_performance: $(PERF_TEST_TARGET)
	./$(PERF_TEST_TARGET)

# ========================================
# UTILITAIRES
# ========================================

# Nettoyage
clean:
	rm -rf $(BUILD_DIR)

# Rebuild complet
re: clean all

# Lancer le programme principal
run: all
	./$(TARGET)

# Tests + Performance (si vous voulez tout lancer d'un coup)
test_complete: test_all test_performance

.PHONY: all clean run test_matching_engine test_outputs test_csv_reader test_all test_performance test_complete directories re help