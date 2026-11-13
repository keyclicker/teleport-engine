#include <cstdio>
#include <exception>
#include <memory>
#include <string>

#include "../Engine/Core/Engine.hpp"
#include "../Engine/World/MapLoader.hpp"
#include "../Engine/World/World.hpp"
#include "Player.hpp"

// Set by the build; the fallback works when run from the repo root
#ifndef RESOURCE_DIR
#define RESOURCE_DIR "Resources"
#endif

using namespace te;

/**
 * @brief Walk around the map
 */
class Walker : public App {
private:
  TextureCache textures{RESOURCE_DIR "/Textures/"};
  World world;
  std::shared_ptr<Player> player;

public:
  explicit Walker(const std::string &map):
    world(loadMap(map, textures)) {
    for (const auto &thing : world.getMap().things) {
      if (thing.type == "player") {
        player = world.spawn<Player>(thing.sector, thing.pos, thing.angle);
      }
    }

    player->updateView(0);
  }

  void update(Engine &engine, float dt) override {
    player->control(engine.getInput(), dt);
    world.update(dt);
    player->updateView(dt);
  }

  Camera getCamera() const override {
    return player->getCamera();
  }
};

int main(int argc, char **argv) {
  try {
    EngineConfig config;
    config.title = "Teleport!";
    parseArgs(argc, argv, config);

    // Static: on the web run() leaves main while frames still run
    static Engine engine(config);
    static Walker game(RESOURCE_DIR "/Maps/teleport.json");

    engine.run(game);
  } catch (const std::exception &e) {
    std::fprintf(stderr, "%s\n", e.what());
    return 1;
  }
}
