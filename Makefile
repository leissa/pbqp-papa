CXX ?= g++

# path #
SRC_PATH = src
BUILD_PATH = build
TEST_PATH = test
TEST_BUILD_PATH = test_build

GUROBI_PATH = ${GUROBI_HOME}

# extensions #
SRC_EXT = cpp

# code lists #
SOURCES = $(shell find $(SRC_PATH) -name '*.$(SRC_EXT)' | sort -k 1nr | cut -f2-)
TESTS = $(shell find $(TEST_PATH) -name '*.$(SRC_EXT)')
OBJECTS = $(SOURCES:$(SRC_PATH)/%.$(SRC_EXT)=$(BUILD_PATH)/%.o)
TEST_OBJECTS = $(TESTS:$(TEST_PATH)/%.$(SRC_EXT)=$(TEST_BUILD_PATH)/%.o)
DEPS = $(OBJECTS:.o=.d)

# flags #
COMPILE_FLAGS = -std=c++17 -Wall -Wextra -g
INCLUDES = -I include/ -I submodules/json/single_include/ -I submodules/doctest/

.PHONY: default_target
default_target: release

.PHONY: release
release: export CXXFLAGS := $(CXXFLAGS) $(COMPILE_FLAGS)
release: export PROJ_INCLUDES := $(INCLUDES)
release: export PROJ_LDFLAGS :=
release: dirs
	@$(MAKE) all_tests

.PHONY: all
all: export CXXFLAGS := $(CXXFLAGS) $(COMPILE_FLAGS) $(shell pkg-config libgvc --cflags 2>/dev/null)
all: export PROJ_INCLUDES := $(INCLUDES) -I $(GUROBI_PATH)/include/
all: export PROJ_LDFLAGS := $(shell pkg-config libgvc --libs 2>/dev/null) -L$(GUROBI_PATH)/lib -lgurobi_c++ -lgurobi81
all: dirs
	@$(MAKE) all_tests

.PHONY: dirs
dirs:
	@echo "Creating directories"
	@mkdir -p $(dir $(TEST_OBJECTS))
	@mkdir -p $(TEST_BUILD_PATH)
	@mkdir -p $(dir $(OBJECTS))

.PHONY: clean
clean:
	@echo "Deleting directories"
	@$(RM) -r $(BUILD_PATH)
	@$(RM) -r $(TEST_BUILD_PATH)

.PHONY: all_tests
all_tests: $(TEST_OBJECTS) $(addsuffix .run,$(TEST_OBJECTS))

# Run tests
$(TEST_BUILD_PATH)/%.o.run: $(TEST_BUILD_PATH)/%.o
	@echo "Running test: $<"
	@./$< > $<-report.txt

# Compile tests
$(TEST_BUILD_PATH)/%.o: $(TEST_PATH)/%.$(SRC_EXT) $(OBJECTS)
	@echo "Compiling: $< -> $@"
	$(CXX) $(CXXFLAGS) $(PROJ_INCLUDES) -o $@ $^ $(PROJ_LDFLAGS)

# Source file rules
$(BUILD_PATH)/%.o: $(SRC_PATH)/%.$(SRC_EXT)
	@echo "Compiling: $< -> $@"
	$(CXX) $(CXXFLAGS) $(PROJ_INCLUDES) -MP -MMD -c $< -o $@

# Add dependency files, if they exist
-include $(DEPS)

