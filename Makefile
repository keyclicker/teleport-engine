# Teleport Engine build. SFML 2 comes from pkg-config: see README.md.

# ---- Configuration -----------------------------------------------------

# Textures are loaded from here at runtime
RESOURCE_DIR ?= Resources

BUILD_DIR ?= build
TARGET    := $(BUILD_DIR)/teleport-engine

# -O0 crashes the renderer (modulo by an empty texture's zero width)
CXXFLAGS ?= -O2 -g

SFML := sfml-graphics >= 2.5 sfml-graphics < 3

# ---- Dependencies ------------------------------------------------------

ifeq ($(filter clean,$(MAKECMDGOALS)),)
  ifneq ($(shell pkg-config --exists '$(SFML)' 2>/dev/null && echo ok),ok)
    $(error SFML 2 not found by pkg-config. Run `nix develop`, see README.md)
  endif
endif

CPPFLAGS += -DRESOURCE_DIR='"$(RESOURCE_DIR)"'
CPPFLAGS += $(shell pkg-config --cflags sfml-graphics)
CXXFLAGS += -std=c++20 -Wall -Wextra -MMD -MP
LDLIBS   += $(shell pkg-config --libs sfml-graphics)

# ---- Targets -----------------------------------------------------------

SRCS := $(wildcard Game/*.cpp Engine/*.cpp Engine/Utils/*.cpp)
OBJS := $(SRCS:%.cpp=$(BUILD_DIR)/%.o)

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(BUILD_DIR)

-include $(OBJS:.o=.d)
