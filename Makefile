# Teleport Engine build. Dependencies come from pkg-config: see README.md.

# ---- Configuration -----------------------------------------------------

# `native`, or `web`: WebAssembly through Emscripten (`nix develop .#web`)
PLATFORM ?= native

SRCS := $(wildcard Game/*.cpp Engine/*/*.cpp)

ifeq ($(PLATFORM),web)
  CXX := em++

  # Textures and maps are bundled into .data files and mounted here
  RESOURCE_DIR := /Resources

  BUILD_DIR ?= build-web

  # Two builds, index.html loads one: threads need a cross-origin
  # isolated page (COOP/COEP headers), the other runs anywhere
  TARGET  := $(BUILD_DIR)/index.html
  BUNDLES := $(BUILD_DIR)/teleport.js $(BUILD_DIR)/teleport-mt.js

  # `make PLATFORM=web run` serves the page here
  PORT ?= 8080

  # No -g: keeps DWARF out of the shipped .wasm
  CXXFLAGS ?= -O2

  # Emscripten brings GLFW and GLES; image decoding and JSON are ours
  PKGS := stb nlohmann_json

  # -O2 again: wasm-opt runs at link time
  LDFLAGS += -O2 -sUSE_GLFW=3 -sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2
  LDFLAGS += -sALLOW_MEMORY_GROWTH -sINITIAL_MEMORY=64MB
  LDFLAGS += --preload-file Resources@$(RESOURCE_DIR)

  # A worker per hardware thread; ThreadPool never asks for more
  MT_LDFLAGS := -pthread -sPTHREAD_POOL_SIZE=navigator.hardwareConcurrency
else ifeq ($(PLATFORM),native)
  # Textures are loaded from here at runtime
  RESOURCE_DIR ?= Resources

  BUILD_DIR ?= build
  TARGET    := $(BUILD_DIR)/teleport-engine

  CXXFLAGS ?= -O2 -g

  # Window and input, GL function loader, image decoding, JSON maps
  PKGS := glfw3 >= 3.3 epoxy stb nlohmann_json
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

# ---- Object variants ---------------------------------------------------

# Objects for one build flavor, in $(BUILD_DIR)/<name>/
#   $(1): name, $(2): extra compiler flags
define VARIANT
$(1)_OBJS := $$(SRCS:%.cpp=$$(BUILD_DIR)/$(1)/%.o)

$$(BUILD_DIR)/$(1)/%.o: %.cpp
	@mkdir -p $$(@D)
	$$(CXX) $$(CPPFLAGS) $$(CXXFLAGS) $(2) -c $$< -o $$@

-include $$(wildcard $$(BUILD_DIR)/$(1)/*/*.d $$(BUILD_DIR)/$(1)/*/*/*.d)
endef

ifeq ($(PLATFORM),web)
  $(eval $(call VARIANT,st,))
  $(eval $(call VARIANT,mt,-pthread))
else
  # The renderer splits frames across threads
  $(eval $(call VARIANT,obj,-pthread))
endif

# ---- Targets -----------------------------------------------------------

.PHONY: all run test clean

all: $(TARGET)

ifeq ($(PLATFORM),web)

$(TARGET): web/index.html $(BUNDLES)
	cp web/index.html $@

# Each bundle embeds the resources
$(BUNDLES): $(wildcard Resources/*/*)

$(BUILD_DIR)/teleport.js: $(st_OBJS)
	$(CXX) $(LDFLAGS) $(st_OBJS) $(LDLIBS) -o $@

$(BUILD_DIR)/teleport-mt.js: $(mt_OBJS)
	$(CXX) $(LDFLAGS) $(MT_LDFLAGS) $(mt_OBJS) $(LDLIBS) -o $@

# Browsers don't fetch .wasm from file://, so serve it. emrun sends the
# COOP/COEP headers, so this gets the threaded build
run: $(TARGET)
	emrun --no_browser --port $(PORT) $(TARGET)

else

$(TARGET): $(obj_OBJS)
	$(CXX) $(LDFLAGS) -pthread $(obj_OBJS) $(LDLIBS) -o $@

run: $(TARGET)
	./$(TARGET)

# Headless engine checks: maps, movement, portals, traces
TEST_OBJS := $(BUILD_DIR)/obj/Tests/WorldTest.o \
             $(filter $(BUILD_DIR)/obj/Engine/World/%,$(obj_OBJS)) \
             $(BUILD_DIR)/obj/Engine/Graphics/Texture.o

$(BUILD_DIR)/world-test: $(TEST_OBJS)
	$(CXX) $(LDFLAGS) -pthread $^ -o $@

test: $(BUILD_DIR)/world-test
	./$(BUILD_DIR)/world-test

endif

clean:
	rm -rf $(BUILD_DIR)
