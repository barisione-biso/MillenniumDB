TARGET := $(notdir $(CURDIR))

BIN_DIR := bin
OBJ_DIR := obj
SRC_DIR := src

OUT_DIRS := $(OBJ_DIR)
OUT_DIRS += $(BIN_DIR)

# create needed folders
$(shell mkdir $(subst /,\,$(OUT_DIRS)) >makefile_out 2>&1 || (exit 0))

# collect source files
SRCS := $(wildcard src/*.cc src/*/*.cc src/*/*/*.cc src/*/*/*/*.cc src/*/*/*/*/*.cc)

# collect header file folders
INC_DIRS := $(wildcard **/*.h)

# prepare inc flags
INC_DIRS := $(dir $(INC_DIRS))
INC_FLAGS = $(addprefix -I,$(INC_DIRS))

# prepare obj file paths
NO_DIRS := $(notdir $(SRCS))
OBJS := $(NO_DIRS:%=$(OBJ_DIR)/%.o)

# prepare dependency file paths
DEPS := $(OBJS:.o=.d)

# compiler flags
# CPPFLAGS := $(INC_FLAGS) -MMD -MP -Wall -Wextra
CPPFLAGS := -std=c++1z -I $(SRC_DIR) -MMD -MP -Wall -Wextra -O2

# call make as "make DEBUG=1"
ifdef DEBUG
	CPPFLAGS += -g
endif

# VPATH built-in variable specifies a list of directories that make should search for both prerequisites and targets of rules
VPATH = $(dir $(SRCS))

 # link
$(BIN_DIR)/$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS) -lboost_filesystem -lboost_iostreams -lboost_system -lssl -lcrypto

# compile cpp
$(OBJ_DIR)/%.cc.o: %.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

# clean
DELS := $(OBJS)
DELS += $(DEPS)
DELS += $(BIN_DIR)/$(TARGET)

# clean target, calling "make clean" cleans the project
clean:
	$(shell $(RM) $(DELS))

cleanfiles:
	$(shell $(RM) test_files/*.*)
