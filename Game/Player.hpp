#pragma once

#include "../Engine/Core/Input.hpp"
#include "../Engine/Render/Renderer.hpp"
#include "../Engine/World/Entity.hpp"

/**
 * @brief First-person body: walks, jumps, looks with the mouse
 */
class Player : public te::Entity {
private:
  // Eye height that eases up stairs instead of snapping
  float viewZ = 0;

  // Head bob phase, advances while walking
  float bobPhase = 0;
  float bob = 0;

  float pitch = 0;

public:
  float eyeHeight = 36;
  float walkSpeed = 300;
  float jumpSpeed = 260;

  // Radians per mouse pixel
  float mouseSensitivity = 0.0025f;

  Player();

  /**
   * @brief Turn input into velocity and facing, call before World::update
   */
  void control(const te::Input &input, float dt);

  /**
   * @brief Ease the view toward the eye, call after World::update
   */
  void updateView(float dt);

  te::Camera getCamera() const;
};
