#pragma once

#include "../Engine/Core/Input.hpp"
#include "../Engine/Render/Renderer.hpp"
#include "Actors.hpp"

/**
 * @brief First-person body: walks, jumps, looks, carries weapons
 */
class Player : public Actor {
private:
  // Eye height that eases up stairs instead of snapping
  float viewZ = 0;

  // Head bob phase, advances while walking
  float bobPhase = 0;
  float bob = 0;

  float pitch = 0;

public:
  enum class Weapon { Pistol, Shotgun };

  float eyeHeight = 36;
  float walkSpeed = 300;
  float jumpSpeed = 260;

  // Radians per mouse pixel
  float mouseSensitivity = 0.0025f;

  int ammo = 40;
  bool hasShotgun = false;
  Weapon weapon = Weapon::Pistol;

  // Seconds until the weapon can fire again, and the muzzle flash shows
  float cooldown = 0;
  float flash = 0;

  // Screen tints fading out: red when hurt, gold on pickups
  float hurtFlash = 0;
  float pickupFlash = 0;

  Player();

  /**
   * @brief Turn input into velocity and facing, call before World::update
   */
  void control(const te::Input &input, float dt);

  /**
   * @brief Ease the view and fade effects, call after World::update
   */
  void updateView(float dt);

  void damage(te::World &world, int amount, te::Entity *source) override;

  /**
   * @brief Vertical view offset from walking, for the weapon sprite
   */
  float getBob() const { return bob; }

  te::Camera getCamera() const;
};
