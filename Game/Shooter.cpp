#include "Shooter.hpp"

#include <algorithm>
#include <cmath>
#include <format>

#include "../Engine/World/MapLoader.hpp"

using namespace te;

// ---- Weapons -------------------------------------------------------------

struct WeaponSpec {
  int pellets;
  int damage;

  // Random aim error per pellet, radians
  float spread;

  // Seconds between shots, ammo per shot
  float cooldown;
  int ammoCost;
};

static constexpr WeaponSpec PISTOL = {1, 15, 0.01f, 0.35f, 1};
static constexpr WeaponSpec SHOTGUN = {7, 10, 0.09f, 0.85f, 2};

static constexpr float SHOT_RANGE = 4000;
static constexpr float MUZZLE_FLASH = 0.08f;

// Gun barrel below the eye, world units
static constexpr float GUN_DROP = 4;

static const WeaponSpec &specOf(Player::Weapon weapon) {
  return weapon == Player::Weapon::Shotgun ? SHOTGUN : PISTOL;
}

// ---- Setup ---------------------------------------------------------------

Shooter::Shooter(const std::string &resourceDir, const std::string &mapName):
  mapPath(resourceDir + "/Maps/" + mapName),
  textures(resourceDir + "/Textures/"),
  assets(resourceDir + "/Sprites/") {
  start();
}

void Shooter::start() {
  world = std::make_unique<World>(loadMap(mapPath, textures));
  player = nullptr;
  monsters.clear();

  // The player first: monsters need their target
  for (const Thing &thing : world->getMap().things) {
    if (thing.type == "player") {
      player = world->spawn<Player>(thing.sector, thing.pos, thing.angle);
    }
  }

  if (!player) throw std::runtime_error("Map has no player start");

  unsigned seed = 1;

  for (const Thing &thing : world->getMap().things) {
    auto spawnPickup = [&](Pickup::Kind kind) {
      world->spawn<Pickup>(thing.sector, thing.pos, 0, assets, kind);
    };

    if (thing.type == "monster") {
      monsters.push_back(world->spawn<Monster>(
        thing.sector, thing.pos, thing.angle, assets, *player, seed++));
    } else if (thing.type == "medkit") {
      spawnPickup(Pickup::Kind::Medkit);
    } else if (thing.type == "ammo") {
      spawnPickup(Pickup::Kind::Ammo);
    } else if (thing.type == "shotgun") {
      spawnPickup(Pickup::Kind::Shotgun);
    }
  }

  player->updateView(0);
  message.clear();
}

// ---- Tick ----------------------------------------------------------------

void Shooter::update(Engine &engine, float dt) {
  const Input &input = engine.getInput();
  messageTime -= dt;

  bool won = countKills() == int(monsters.size());

  if (input.isPressed(GLFW_KEY_R) && (player->isDead() || won)) {
    start();
    return;
  }

  // Dead: a click restarts too
  if (player->isDead() &&
      input.isButtonPressed(GLFW_MOUSE_BUTTON_LEFT) && player->eyeHeight <= 8) {
    start();
    return;
  }

  // Paused until the mouse is captured: click to play, Esc to pause
  if (!input.isCaptured()) return;

  player->control(input, dt);

  if (!player->isDead()) {
    if (input.isPressed(GLFW_KEY_1)) player->weapon = Player::Weapon::Pistol;
    if (input.isPressed(GLFW_KEY_2) && player->hasShotgun) {
      player->weapon = Player::Weapon::Shotgun;
    }

    bool trigger = input.isButtonDown(GLFW_MOUSE_BUTTON_LEFT) ||
                   input.isDown(GLFW_KEY_LEFT_CONTROL);

    if (trigger) fire(engine);
  }

  world->update(dt);
  player->updateView(dt);

  collectPickups();
}

void Shooter::fire(Engine &engine) {
  const WeaponSpec &spec = specOf(player->weapon);

  if (player->cooldown > 0) return;

  // Out of shells: fall back to the pistol
  if (player->ammo < spec.ammoCost) {
    if (player->weapon == Player::Weapon::Shotgun) {
      player->weapon = Player::Weapon::Pistol;
    }
    return;
  }

  player->ammo -= spec.ammoCost;
  player->cooldown = spec.cooldown;
  player->flash = MUZZLE_FLASH;

  Camera camera = player->getCamera();
  const Frame &frame = engine.getFrame();
  float slope = camera.centerSlope(float(frame.getWidth()) / frame.getHeight());

  std::uniform_real_distribution<float> spread(-spec.spread, spec.spread);

  for (int i = 0; i < spec.pellets; ++i) {
    Vec2 dir = Vec2::fromAngle(camera.angle + spread(rng));
    float pelletSlope = slope + spread(rng) * 0.5f;

    TraceHit hit = world->trace(player->sector, player->pos,
                                camera.z - GUN_DROP, dir, SHOT_RANGE,
                                player.get(), pelletSlope);

    if (auto actor = dynamic_cast<Actor *>(hit.entity)) {
      actor->damage(*world, spec.damage, player.get());
    }

    // A puff where it landed, pulled back so it isn't inside the wall
    if (hit.hit) {
      spawnPuff(*world, assets, hit.sector, hit.pos - hit.dir * 2, hit.z,
                0.4f);
    }
  }
}

void Shooter::collectPickups() {
  if (player->isDead()) return;

  for (Entity *entity : player->sector->entities) {
    auto pickup = dynamic_cast<Pickup *>(entity);
    if (!pickup || pickup->isRemoved()) continue;

    float reach = player->radius + pickup->radius;
    if ((pickup->pos - player->pos).length() > reach) continue;

    switch (pickup->kind) {
    case Pickup::Kind::Medkit:
      // Leave it for later when healthy
      if (player->health >= 100) continue;
      player->health = std::min(100, player->health + 25);
      showMessage("Picked up a medkit");
      break;

    case Pickup::Kind::Ammo:
      player->ammo += 20;
      showMessage("Picked up ammo");
      break;

    case Pickup::Kind::Shotgun:
      player->hasShotgun = true;
      player->weapon = Player::Weapon::Shotgun;
      player->ammo += 10;
      showMessage("You got the shotgun!  [2]");
      break;
    }

    player->pickupFlash = 0.4f;
    pickup->remove();
  }
}

void Shooter::showMessage(const std::string &text) {
  message = text;
  messageTime = 2;
}

int Shooter::countKills() const {
  return int(std::count_if(monsters.begin(), monsters.end(),
                           [](const auto &m) { return m->isDead(); }));
}

Camera Shooter::getCamera() const {
  return player->getCamera();
}

// ---- HUD -----------------------------------------------------------------

void Shooter::drawWeapon(Canvas &canvas) {
  bool firing = player->flash > 0;
  bool shotgun = player->weapon == Player::Weapon::Shotgun;

  const Texture &texture = shotgun
    ? *(firing ? assets.shotgunFire : assets.shotgun)
    : *(firing ? assets.pistolFire : assets.pistol);

  // About a third of the screen tall, at any render scale
  float scale = canvas.getHeight() * 0.38f / texture.getHeight();

  float bob = player->getBob();
  float swayX = bob * scale * 1.5f;
  float dropY = std::abs(bob) * scale + (firing ? 3 * scale : 0);

  int x = int(canvas.getWidth() / 2 - texture.getWidth() * scale / 2 + swayX);
  int y = int(canvas.getHeight() - texture.getHeight() * scale + dropY);

  canvas.drawTexture(texture, x, y, scale);
}

void Shooter::drawCrosshair(Canvas &canvas) {
  int unit = std::max(1, canvas.getHeight() / 240);
  int cx = canvas.getWidth() / 2, cy = canvas.getHeight() / 2;
  Color color = {120, 255, 120};

  canvas.fillRect(cx - 4 * unit, cy, 3 * unit, unit, color);
  canvas.fillRect(cx + 2 * unit, cy, 3 * unit, unit, color);
  canvas.fillRect(cx, cy - 4 * unit, unit, 3 * unit, color);
  canvas.fillRect(cx, cy + 2 * unit, unit, 3 * unit, color);
}

void Shooter::drawHud(Engine &engine, Hud &hud) {
  Canvas &canvas = hud.canvas;

  if (!player->isDead()) {
    drawWeapon(canvas);
    drawCrosshair(canvas);
  }

  canvas.tint({200, 0, 0}, player->hurtFlash);
  canvas.tint({255, 200, 60}, player->pickupFlash * 0.5f);

  if (player->isDead()) canvas.tint({90, 0, 0}, 0.4f);

  // ---- Status line ----

  float scale = hud.textScale * 2;
  int kills = countKills();
  int total = int(monsters.size());

  std::string weapons = player->hasShotgun ? "[1] pistol  [2] shotgun"
                                           : "[1] pistol";
  auto status = std::format("HEALTH {}   AMMO {}   KILLS {}/{}   {}",
                            std::max(0, player->health), player->ammo,
                            kills, total, weapons);

  Color statusColor = player->health > 25 ? Color{230, 230, 230}
                                          : Color{255, 80, 60};

  hud.text.print(status, 8 * hud.textScale,
                 hud.screenHeight - 16 * scale, scale, statusColor);

  // ---- Center messages ----

  auto center = [&](const std::string &text, float y, float size,
                    Color color) {
    float width = TextRenderer::measure(text, size);
    hud.text.print(text, (hud.screenWidth - width) / 2, y, size, color);
  };

  float middle = hud.screenHeight * 0.3f;

  if (player->isDead()) {
    center("YOU DIED", middle, scale * 2, {255, 60, 40});
    center("click or R to try again", middle + 30 * scale, scale,
           {230, 230, 230});
  } else if (kills == total) {
    center("MAP CLEAR", middle, scale * 2, {255, 220, 60});
    center("R to play again", middle + 30 * scale, scale, {230, 230, 230});
  } else if (!engine.getInput().isCaptured()) {
    center("PAUSED  click to play", middle, scale * 1.5f, {230, 230, 230});
    center("WASD move  mouse look  click shoot  space jump  1 2 weapons",
           middle + 24 * scale, hud.textScale * 1.5f, {200, 200, 200});
  }

  if (messageTime > 0) {
    center(message, middle - 20 * scale, scale, {255, 220, 120});
  }
}
