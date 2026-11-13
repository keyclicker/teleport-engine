#include "Player.hpp"

#include <algorithm>
#include <cmath>

using namespace te;

// Turn speed for keyboard turning, radians per second
static constexpr float TURN_SPEED = 2;

// Head bob: cycles per second and height in world units
static constexpr float BOB_RATE = 2;
static constexpr float BOB_HEIGHT = 2.8f;

// How fast the view catches up with a step, per second
static constexpr float STEP_EASE = 12;

// Look limits, in screen heights of shearing
static constexpr float MAX_PITCH = 0.4f;

Player::Player() {
  radius = 16;
  height = 48;
  stepHeight = 24;
}

void Player::control(const Input &input, float dt) {
  cooldown -= dt;

  if (isDead()) {
    velocity = {};
    return;
  }

  // ---- Look ----

  Vec2 motion = input.getMotion();
  angle -= motion.x * mouseSensitivity;
  pitch = std::clamp(pitch - motion.y * mouseSensitivity * 0.6f,
                     -MAX_PITCH, MAX_PITCH);

  if (input.isDown(GLFW_KEY_LEFT) || input.isDown(GLFW_KEY_Q)) {
    angle += TURN_SPEED * dt;
  }
  if (input.isDown(GLFW_KEY_RIGHT) || input.isDown(GLFW_KEY_E)) {
    angle -= TURN_SPEED * dt;
  }

  // ---- Walk ----

  Vec2 forward = this->forward();
  Vec2 right = forward.right();
  Vec2 wish;

  if (input.isDown(GLFW_KEY_W) || input.isDown(GLFW_KEY_UP)) wish += forward;
  if (input.isDown(GLFW_KEY_S) || input.isDown(GLFW_KEY_DOWN)) wish -= forward;
  if (input.isDown(GLFW_KEY_D)) wish += right;
  if (input.isDown(GLFW_KEY_A)) wish -= right;

  velocity = wish.normalized() * walkSpeed;

  if (input.isPressed(GLFW_KEY_SPACE) && onGround) {
    verticalVelocity = jumpSpeed;
  }

  // ---- Bob ----

  bool walking = wish.lengthSquared() > 0 && onGround;

  if (walking) {
    bobPhase += 2 * PI * BOB_RATE * dt;
    bob = BOB_HEIGHT * std::sin(bobPhase);
  } else {
    bob *= std::max(0.0f, 1 - dt * 8);
  }
}

void Player::updateView(float dt) {
  flash -= dt;
  hurtFlash = std::max(0.0f, hurtFlash - dt * 2);
  pickupFlash = std::max(0.0f, pickupFlash - dt * 3);

  // Dead: sink to the floor
  if (isDead()) eyeHeight = std::max(8.0f, eyeHeight - dt * 60);

  float target = z + eyeHeight;

  // Ease up steps; falls and jumps are already smooth
  if (target > viewZ && onGround) {
    viewZ = std::min(target, viewZ + (target - viewZ) * STEP_EASE * dt
                             + 40 * dt);
  } else {
    viewZ = target;
  }
}

void Player::damage(World &world, int amount, Entity *source) {
  if (isDead()) return;

  Actor::damage(world, amount, source);
  hurtFlash = std::min(0.6f, hurtFlash + amount / 40.0f);
}

Camera Player::getCamera() const {
  Camera camera;
  camera.sector = sector;
  camera.pos = pos;
  camera.z = viewZ + bob;
  camera.angle = angle;
  camera.pitch = pitch;
  camera.self = this;

  return camera;
}
