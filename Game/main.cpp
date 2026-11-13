#include <cstdio>
#include <exception>

#include "../Engine/Core/Engine.hpp"
#include "Shooter.hpp"

// Set by the build; the fallback works when run from the repo root
#ifndef RESOURCE_DIR
#define RESOURCE_DIR "Resources"
#endif

int main(int argc, char **argv) {
  try {
    te::EngineConfig config;
    config.title = "Teleport!";
    te::parseArgs(argc, argv, config);

    // Static: on the web run() leaves main while frames still run
    static te::Engine engine(config);
    static Shooter game(RESOURCE_DIR, "teleport.json");

    engine.run(game);
  } catch (const std::exception &e) {
    std::fprintf(stderr, "%s\n", e.what());
    return 1;
  }
}
