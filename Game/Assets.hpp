#pragma once

#include <string>

#include "../Engine/Graphics/Texture.hpp"

/**
 * @brief Every sprite the game draws, loaded once
 */
struct Assets {
  te::TextureRef monsterWalk[2], monsterAttack, monsterPain, monsterDead;
  te::TextureRef fireball[2], puff[3];
  te::TextureRef medkit, ammo, shotgunPickup;
  te::TextureRef pistol, pistolFire, shotgun, shotgunFire;

  /**
   * @param directory Sprite folder, ending with `/`
   * @throw std::runtime_error A sprite failed to load
   */
  explicit Assets(const std::string &directory) {
    te::TextureCache cache(directory);

    monsterWalk[0] = cache.get("monster-walk1.png");
    monsterWalk[1] = cache.get("monster-walk2.png");
    monsterAttack = cache.get("monster-attack.png");
    monsterPain = cache.get("monster-pain.png");
    monsterDead = cache.get("monster-dead.png");

    fireball[0] = cache.get("fireball1.png");
    fireball[1] = cache.get("fireball2.png");

    for (int i = 0; i < 3; ++i) {
      puff[i] = cache.get("puff" + std::to_string(i + 1) + ".png");
    }

    medkit = cache.get("medkit.png");
    ammo = cache.get("ammo.png");
    shotgunPickup = cache.get("shotgun-pickup.png");

    pistol = cache.get("pistol.png");
    pistolFire = cache.get("pistol-fire.png");
    shotgun = cache.get("shotgun.png");
    shotgunFire = cache.get("shotgun-fire.png");
  }
};
