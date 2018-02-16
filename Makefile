TARGET_EXEC ?= libargparser
BUILD_DIR ?= ./build
SRC_DIRS ?= ./src
UNIT_TESTS_DIR ?= ./unit-tests

SRCS := $(shell find $(SRC_DIRS) -maxdepth 1 -name *.cpp -or -name *.c -or -name *.s)
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find . -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))
LDDIRS := $(shell find . -type d)
LDPATHS := $(addprefix -L,$(LDDIRS))
#LDFLAGS := -fPIC #add libraries here

CC=gcc
CXX=g++
CPPFLAGS ?= $(INC_FLAGS) -std=c++11 -Wall -Wextra -pedantic-errors -fPIC

ifdef DEBUG
CPPFLAGS += -g -O0
else
CPPFLAGS += -O3
endif


.PHONY: clean all

all: $(BUILD_DIR)/$(TARGET_EXEC) #tests

$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	# build static library
	ar rcs $@.a $(OBJS)
	# build shared library
	$(CXX) $(LDPATHS) -shared $(OBJS) -o $@.so $(LDFLAGS)

# uncomment the following for C++ source
$(BUILD_DIR)/%.cpp.o: %.cpp
	$(MKDIR_P) $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

clean:
	$(RM) -r $(BUILD_DIR)
#	$(MAKE) -C $(UNIT_TESTS_DIR) clean

tests:
	$(MAKE) -C $(UNIT_TESTS_DIR)

-include $(DEPS)

MKDIR_P ?= mkdir -p

