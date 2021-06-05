# Ensures all is the default
all:
	@echo "Please specify which project you want to build (client/server)"

# Compiler alternatives, options and flags
LD_FLAGS := -pthread
DEBUG_FLAGS := -O0 -ggdb

CXX := g++
CXX_FLAGS := $(DEBUG_FLAGS) $(LD_FLAGS) 
#

# Binaries and it's dependencies
RULES := server client
SERVER_OBJS := server.o socket.o
CLIENT_OBJS := client.o socket.o
#

# Project structure
SRC_DIR := ./src
OBJ_DIR := ./objs
BIN_DIR := ./bin
DEP_DIR := ./deps
#

# Project structure auxiliary variables
SUBDIRS := $(SRC_DIR) $(OBJ_DIR) $(BIN_DIR) $(DEP_DIR)

SOURCES := $(shell find . -name "*.cpp")
DEPS := $(SOURCES:$(SRC_DIR)/%.cpp=$(DEP_DIR)/%.d)

BINARIES := $(addprefix $(BIN_DIR)/,$(RULES))

SERVER_OBJS := $(addprefix $(OBJ_DIR)/,$(SERVER_OBJS))
CLIENT_OBJS := $(addprefix $(OBJ_DIR)/,$(CLIENT_OBJS))
#

# [GLOBAL] Assure subdirectories exist
$(shell mkdir -p $(SUBDIRS))

# Aliases to bin/server and bin/client
.PHONY: server client
server: .EXTRA_PREREQS = ./bin/server
client: .EXTRA_PREREQS = ./bin/client

# Inform which objects are used by each binary
./bin/server: $(SERVER_OBJS)
./bin/client: $(CLIENT_OBJS)

# Create binary out of objects
$(BINARIES):
	$(CXX) $^ -o $@ $(CXX_FLAGS)

# Convert cpp to obj files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp 
	$(CXX) $(SRC_DIR)/$(<F) -c $(CXX_FLAGS) -o $@

# Make obj be recompiled after .hpp changed (include all .d files)
ifeq (,$(filter clean,$(MAKECMDGOALS)))

include $(DEPS)

endif
# Rule to create/update .d files
$(DEP_DIR)/%.d: $(SRC_DIR)/%.cpp
#	Gets all includes of a .cpp file
	$(CXX) -MM -MT '$@ $(OBJ_DIR)/$(@F:.d=.o)' $< > $@
	

# Delete output subfolders
.PHONY: clean
clean: 
	rm -rf $(OBJ_DIR) $(DEP_DIR) $(BIN_DIR)