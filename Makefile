### compiler and flags ###
CXX=g++
CXXFLAGS=-O2 -std=c++14

### build output directories ###
BUILD_DIR=build
DIST_DIR=dist
LOG_DIR=log
### source directories ###
TEST_DIR=test
SRC_DIR=src/main
STEPS_DIR=src/steps

### .cpp and .o files ###
# wildcard keyword used to request the use of * cus it only gets automatically expanded when used in a rule/pre-requisite 
SRCS=$(SRC_DIR)/Core.cpp $(SRC_DIR)/Environment.cpp $(SRC_DIR)/MALtypes.cpp $(SRC_DIR)/myExceptions.cpp $(SRC_DIR)/printer.cpp $(SRC_DIR)/reader.cpp $(SRC_DIR)/repl.cpp
TEST_SRCS=$(SRC_DIR)/replNonTCO.cpp $(wildcard $(STEPS_DIR)/*.cpp)
# $(info $$SRCS is [${SRCS}])
# $(info $$TEST_SRCS is [${TEST_SRCS}])

REPL_MAIN=$(SRC_DIR)/mal.cpp
TEST_MAIN=$(TEST_DIR)/test.cpp
# $(patsubst pattern,replacement,text) Finds whitespace-separated words in text that match pattern and replaces them with replacement.
# 		‘%’ in pattern is a wildcard, in replacement it acts as a reference to the match in the pattern. 
#		Words that do not match the pattern are kept without change in the output. 
#		Only the first ‘%’ in the pattern and replacement is treated this way; any subsequent ‘%’ is unchanged.

# % matches the filename, turning every .cpp file in the SRCS list into a .o file in the OBJS list
OBJS=$(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))
# contains all the additional object files needed to compile the test executable
TEST_OBJS=$(BUILD_DIR)/replNonTCO.o $(patsubst $(STEPS_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(wildcard $(STEPS_DIR)/*.cpp))
# $(info $$TEST_SRCS is [${TEST_SRCS}])
# $(info $$TEST_OBJS is [${TEST_OBJS}])


### TARGETS ###

# .PHONY is a special target name that tells make that the target is not a file. Will only be run when explicitly called.
.PHONY: clean run test

## default target, copies and renames the REPL/malCPP executable to the dist directory
$(DIST_DIR)/REPL: $(BUILD_DIR)/malCPP | $(DIST_DIR)
	cp $(BUILD_DIR)/malCPP $(DIST_DIR)/REPL


## builds the REPL/malCPP executable in the /build directory from the OBJS list
$(BUILD_DIR)/malCPP:  $(REPL_MAIN) $(OBJS) | $(BUILD_DIR)
# $^ 			The names of all the prerequisites, with spaces between them.
# $@ 			The file name of the target of the rule.
	$(CXX) $(CXXFLAGS) -o $@ $^


# The prerequisites on the left of | are normal,
# the right side prerequisites are order-only, these do not cause the the target to be rebuilt if they are newer than the target.
# Thus the build directory gets created before we try to write to it, but the object files dont get rebuilt if the build dir changes timestamp.
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp  | $(BUILD_DIR)
# $< 			The name of the first prerequisite.
# -c 			compiles and assembles files but doesn’t link them. 
#				This is useful when building large projects to separate file compilation and minimize what is re-compiled.
	$(CXX) $(CXXFLAGS) -c -o $@ $<


# Creates the build and/or dist directory if it doesnt exist
$(BUILD_DIR) $(DIST_DIR) $(LOG_DIR):
# -p 			creates parent directories if they don’t exist.
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR) $(DIST_DIR)

# runs the REPL executable
run: $(DIST_DIR)/REPL
	./$(DIST_DIR)/REPL


### TESTS ###
# runs the test executable in the build directory
test: $(BUILD_DIR)/test | $(BUILD_DIR) $(LOG_DIR)
	./$(BUILD_DIR)/test

# builds the test executable in the build directory
$(BUILD_DIR)/test: $(TEST_MAIN) $(TEST_OBJS) $(OBJS)
	$(CXX) $(CXXFLAGS) -I $(SRC_DIR) -I$(STEPS_DIR) -o $(BUILD_DIR)/test $^

$(BUILD_DIR)/replNonTCO.o: $(SRC_DIR)/replNonTCO.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(filter-out %replNonTCO.o, $(TEST_OBJS)): $(BUILD_DIR)/%.o: $(STEPS_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -I $(SRC_DIR) -c -o $@ $<