# Teleport Engine build. Dependencies come from pkg-config: see README.md.

# ---- Configuration -----------------------------------------------------

# Textures are loaded from here at runtime
RESOURCE_DIR ?= Resources

BUILD_DIR ?= build
TARGET    := $(BUILD_DIR)/teleport-engine

CXXFLAGS ?= -O2 -g

# Window and input, GL function loader, image decoding
PKGS := glfw3 >= 3.3 epoxy stb

# ---- Dependencies ------------------------------------------------------

ifeq ($(filter clean,$(MAKECMDGOALS)),)
  ifneq ($(shell pkg-config --exists '$(PKGS)' 2>/dev/null && echo ok),ok)
    $(error $(PKGS) not found by pkg-config. Run `nix develop`, see README.md)
  endif
endif

CPPFLAGS += -DRESOURCE_DIR='"$(RESOURCE_DIR)"'
# -isystem: warnings in library headers aren't ours to fix
CPPFLAGS += $(patsubst -I%,-isystem %,$(shell pkg-config --cflags '$(PKGS)'))
CXXFLAGS += -std=c++20 -Wall -Wextra -MMD -MP
LDLIBS   += $(shell pkg-config --libs '$(PKGS)')

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
