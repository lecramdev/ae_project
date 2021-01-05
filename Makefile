# Tools
MKDIR_P = mkdir -p
CXX=g++-9

# Flags
CXXFLAGS += -MMD -MP -I/opt/gurobi911/linux64/include
LDFLAGS += -L/opt/gurobi911/linux64/lib/ -lgurobi_g++5.2 -lgurobi91

# Defines
BUILD_DIR = $(CURDIR)/build
RELEASE_DIR = $(BUILD_DIR)/Release
DEBUG_DIR = $(BUILD_DIR)/Debug

APP_NAME = ae_project
RELEASE_APP = $(CURDIR)/$(APP_NAME).exe
DEBUG_APP = $(CURDIR)/$(APP_NAME)_dbg.exe

# Sources
SRCS = $(wildcard src/*.cpp)

RELEASE_OBJS = $(SRCS:%.cpp=$(RELEASE_DIR)/%.o)
RELEASE_DEPS = $(RELEASE_OBJS:.o=.d)

DEBUG_OBJS = $(SRCS:%.cpp=$(DEBUG_DIR)/%.o)
DEBUG_DEPS = $(DEBUG_OBJS:.o=.d)

# Rules
$(RELEASE_APP): $(RELEASE_OBJS)
	$(CXX) -O2 $(RELEASE_OBJS) -o $@ $(LDFLAGS)

$(DEBUG_APP): $(DEBUG_OBJS)
	$(CXX) -g -pg $(DEBUG_OBJS) -o $@ $(LDFLAGS)

$(RELEASE_DIR)/%.o: %.cpp
	@$(MKDIR_P) $(dir $@)
	$(CXX) -O2 $(CXXFLAGS) -c $< -o $@

$(DEBUG_DIR)/%.o: %.cpp
	@$(MKDIR_P) $(dir $@)
	$(CXX) -g -pg $(CXXFLAGS) -c $< -o $@

.PHONY: all clean

all: $(RELEASE_APP) $(DEBUG_APP)

clean:
	rm -rf $(BUILD_DIR) $(RELEASE_APP) $(DEBUG_APP)

-include $(RELEASE_DEPS)
-include $(DEBUG_DEPS)