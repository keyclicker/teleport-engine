#pragma once

#include <memory>
#include <random>
#include <string>
#include <vector>

#include "../Engine/Core/Engine.hpp"
#include "../Engine/World/World.hpp"
#include "Actors.hpp"
#include "Assets.hpp"
#include "Player.hpp"

/**
 * @brief The demo: clear the map of monsters, through its portals
 *
 * Map things it knows: `player`, `monster`, `medkit`, `ammo`,
 * `shotgun`.
 */
class Shooter : public te::App {
private:
  std::string mapPath;
  te::TextureCache textures;
  Assets assets;

  std::unique_ptr<te::World> world;
  std::shared_ptr<Player> player;
  std::vector<std::shared_ptr<Monster>> monsters;

  std::mt19937 rng{1};

  // Short note in the middle of the screen, e.g. "Picked up a shotgun"
  std::string message;
  float messageTime = 0;

  /**
   * @brief (Re)load the map and spawn its things
   */
  void start();

  void fire(te::Engine &engine);
  void collectPickups();
  void showMessage(const std::string &text);

  int countKills() const;

  void drawWeapon(te::Canvas &canvas);
  void drawCrosshair(te::Canvas &canvas);

public:
  /**
   * @param resourceDir Folder with Textures/, Sprites/ and Maps/
   * @param mapName Map file in Maps/
   * @throw std::runtime_error The map or a sprite failed to load
   */
  Shooter(const std::string &resourceDir, const std::string &mapName);

  void update(te::Engine &engine, float dt) override;
  te::Camera getCamera() const override;
  void drawHud(te::Engine &engine, te::Hud &hud) override;
};
