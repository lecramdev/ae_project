# Tools
MKDIR_P = mkdir -p
CXX=g++-9

# Flags
CXXFLAGS += -MMD -MP

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
	$(CXX) -O2 $(RELEASE_OBJS) -o $@

$(DEBUG_APP): $(DEBUG_OBJS)
	$(CXX) -O2 $(DEBUG_OBJS) -o $@

$(RELEASE_DIR)/%.o: %.cpp
	@$(MKDIR_P) $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(DEBUG_DIR)/%.o: %.cpp
	@$(MKDIR_P) $(dir $@)
	$(CXX) -g $(CXXFLAGS) -c $< -o $@

.PHONY: all clean

all: $(RELEASE_APP) $(DEBUG_APP)

clean:
	rm -rf $(BUILD_DIR) $(RELEASE_APP) $(DEBUG_APP)

-include $(RELEASE_DEPS)
-include $(DEBUG_DEPS)