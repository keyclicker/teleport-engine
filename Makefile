# Teleport Engine build. Dependencies come from pkg-config: see README.md.

# ---- Configuration -----------------------------------------------------

# `native`, or `web`: WebAssembly through Emscripten (`nix develop .#web`)
PLATFORM ?= native

ifeq ($(PLATFORM),web)
  CXX := em++

  # Textures are bundled into index.data and mounted here
  RESOURCE_DIR := /Resources

  BUILD_DIR ?= build-web
  TARGET    := $(BUILD_DIR)/index.html

  # `make PLATFORM=web run` serves the page here
  PORT ?= 8080

  # No -g: keeps DWARF out of the shipped .wasm
  CXXFLAGS ?= -O2

  # Emscripten brings GLFW and GLES, only image decoding is ours
  PKGS := stb

  # -O2 again: wasm-opt runs at link time
  LDFLAGS += -O2 -sUSE_GLFW=3 -sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2
  LDFLAGS += -sALLOW_MEMORY_GROWTH --shell-file web/shell.html
  LDFLAGS += --preload-file Resources@$(RESOURCE_DIR)
else ifeq ($(PLATFORM),native)
  # Textures are loaded from here at runtime
  RESOURCE_DIR ?= Resources

  BUILD_DIR ?= build
  TARGET    := $(BUILD_DIR)/teleport-engine

  CXXFLAGS ?= -O2 -g

  # Window and input, GL function loader, image decoding
  PKGS := glfw3 >= 3.3 epoxy stb
else
  $(error PLATFORM must be `native` or `web`, not `$(PLATFORM)`)
endif

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

# The web bundle embeds the textures and the page template
ifeq ($(PLATFORM),web)
$(TARGET): $(wildcard Resources/*/*) web/shell.html
endif

$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $@

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

ifeq ($(PLATFORM),web)
# Browsers don't fetch .wasm from file://, so serve it
run: $(TARGET)
	emrun --no_browser --port $(PORT) $(TARGET)
else
run: $(TARGET)
	./$(TARGET)
endif

clean:
	rm -rf $(BUILD_DIR)

-include $(OBJS:.o=.d)
