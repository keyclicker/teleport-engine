#include "../Engine/Game.hpp"
#include "../Engine/Map.hpp"

// This file inits map
// There is definitely need in map format and map editor

int main() {
  auto map = new Map();

  auto portalSide = std::make_shared<Map::Side >();

  portalSide->upper.loadFromFile("../Resources/Textures/brick.png");
  portalSide->lower.loadFromFile("../Resources/Textures/brick.png");

  auto side1 = std::make_shared<Map::Side >();
  side1->middle.loadFromFile("../Resources/Textures/wall.gif");

  auto sec1 = new Map::Sector(map);

  sec1->ceiling.loadFromFile("../Resources/Textures/ceiling.gif");
  sec1->floor.loadFromFile("../Resources/Textures/stone.png");

  sec1->ceilingheight = 200;
  sec1->floorheight = 0;
  sec1->lightlevel = 60;

  new Map::Line(sec1, {400, 400}, {400, 0}, side1);
  auto loop1 = new Map::Line(sec1, {400, 0}, {400, -400}, portalSide);

  new Map::Line(sec1, {400, -400}, {0, -600}, side1);
  new Map::Line(sec1, {0, -600}, {-400, -400}, side1);
  new Map::Line(sec1, {-400, -400}, {-400, 0}, side1);

  auto loop0 = new Map::Line(sec1, {-400, 0}, {-400, 400}, portalSide);

  new Map::Line(sec1, {-400, 400}, {0, 600}, side1);

  auto portal1 = new Map::Line(sec1, {0, 600}, {400, 400}, portalSide);

  auto side2 = std::make_shared<Map::Side >();
  side2->middle = sf::Image();
  side2->middle.loadFromFile("../Resources/Textures/wall.gif");

  auto sec2 = new Map::Sector(map);

  sec2->ceiling.loadFromFile("../Resources/Textures/ceiling.gif");
  sec2->floor.loadFromFile("../Resources/Textures/floor.gif");

  sec2->ceilingheight = 150;
  sec2->floorheight = 20;
  sec2->lightlevel = 60;

  auto portal2 = new Map::Line(sec2,{400, 400}, {0, 600}, portalSide);
  new Map::Line(sec2, {0, 600}, {400, 1000}, side2);
  new Map::Line(sec2, {400, 1000}, {800, 800}, side2);
  auto portal3 = new Map::Line(sec2, {800, 800}, {400, 400}, portalSide);

  portal1->portal = portal2;
  portal2->portal = portal1;

  auto sec3 = new Map::Sector(map);
  sec3->ceiling.loadFromFile("../Resources/Textures/ceiling.gif");
  sec3->floor.loadFromFile("../Resources/Textures/floor.gif");

  sec3->ceilingheight = 300;
  sec3->floorheight = 40;

  new Map::Line(sec3, {800, 800}, {800, 400}, side2);
  auto sp0 = new Map::Line(sec3, {800, 400}, {400, 400}, portalSide);
  auto portal4 = new Map::Line(sec3, {400, 400}, {800, 800}, portalSide);

  portal3->portal = portal4;
  portal4->portal = portal3;

  auto stair1 = new Map::Sector(map);
  stair1->ceiling.loadFromFile("../Resources/Textures/ceiling.gif");
  stair1->floor.loadFromFile("../Resources/Textures/floor.gif");

  stair1->ceilingheight = 300;
  stair1->floorheight = 60;

  auto sp1 = new Map::Line(stair1, {400, 400}, {800, 400}, portalSide);
  new Map::Line(stair1, {800, 400}, {800, 300}, side2);
  auto sp2 = new Map::Line(stair1, {800, 300}, {400, 300}, portalSide);
  new Map::Line(stair1, {400, 300}, {400, 400}, side2);

  sp0->portal = sp1;
  sp1->portal = sp0;

  auto stair2 = new Map::Sector(map);
  stair2->ceiling.loadFromFile("../Resources/Textures/ceiling.gif");
  stair2->floor.loadFromFile("../Resources/Textures/floor.gif");

  stair2->ceilingheight = 300;
  stair2->floorheight = 80;

  auto sp3 = new Map::Line(stair2, {400, 300}, {800, 300}, portalSide);
  new Map::Line(stair2, {800, 300}, {800, 200}, side2);
  auto sp4 = new Map::Line(stair2, {800, 200}, {400, 200}, portalSide);
  new Map::Line(stair2, {400, 200}, {400, 300}, side2);

  sp2->portal = sp3;
  sp3->portal = sp2;

  auto stair3 = new Map::Sector(map);
  stair3->ceiling.loadFromFile("../Resources/Textures/ceiling.gif");
  stair3->floor.loadFromFile("../Resources/Textures/floor.gif");

  stair3->ceilingheight = 300;
  stair3->floorheight = 100;

  auto sp5 = new Map::Line(stair3, {400, 200}, {800, 200}, portalSide);
  new Map::Line(stair3, {800, 200}, {800, 100}, side2);
  auto sp6 = new Map::Line(stair3, {800, 100}, {400, 100}, portalSide);
  new Map::Line(stair3, {400, 100}, {400, 200}, side2);

  sp4->portal = sp5;
  sp5->portal = sp4;

  auto stair4 = new Map::Sector(map);
  stair4->ceiling.loadFromFile("../Resources/Textures/ceiling.gif");
  stair4->floor.loadFromFile("../Resources/Textures/floor.gif");

  stair4->ceilingheight = 300;
  stair4->floorheight = 120;

  auto sp7 = new Map::Line(stair4, {400, 100}, {800, 100}, portalSide);
  new Map::Line(stair4, {800, 100}, {800, 0}, side2);
  auto sp8 = new Map::Line(stair4, {800, 0}, {400, 0}, portalSide);
  new Map::Line(stair4, {400, 0}, {400, 100}, side2);

  sp6->portal = sp7;
  sp7->portal = sp6;

  auto secret = new Map::Sector(map);
  secret->ceiling.loadFromFile("../Resources/Textures/ceiling.gif");
  secret->floor.loadFromFile("../Resources/Textures/floor.gif");

  secret->ceilingheight = 300;
  secret->floorheight = 140;

  auto sideHi = std::make_shared<Map::Side >();
  sideHi->middle.loadFromFile("../Resources/Textures/hitler.png");

  auto sideX = std::make_shared<Map::Side >();
  sideX->middle.loadFromFile("../Resources/Textures/sw.png");

  auto sp9 = new Map::Line(secret, {400, 0}, {800, 0}, portalSide);
  new Map::Line(secret, {800, 0}, {800, -400}, sideX);
  new Map::Line(secret, {800, -400}, {400, -400}, sideHi);
  new Map::Line(secret, {400, -400}, {400, 0}, sideX);

  sp8->portal = sp9;
  sp9->portal = sp8;

  auto tunel = new Map::Sector(map);
  tunel->ceiling.loadFromFile("../Resources/Textures/ceiling.gif");
  tunel->floor.loadFromFile("../Resources/Textures/floor.gif");

  tunel->ceilingheight = 150;
  tunel->floorheight = 0;

  auto loop2 = new Map::Line(tunel, {400, -400}, {400, 0}, portalSide);
  new Map::Line(tunel, {400, 0}, {600, 0}, side1);
  new Map::Line(tunel, {600, 0}, {800, -200}, side1);
  new Map::Line(tunel, {800, -200}, {800, -600}, side1);
  auto loop3 = new Map::Line(tunel, {800, -600}, {800, -1000}, portalSide);
  new Map::Line(tunel, {800, -1000}, {600, -1000}, side1);
  new Map::Line(tunel, {600, -1000}, {400, -800}, side1);
  new Map::Line(tunel, {400, -800}, {400, -400}, side1);

  loop1->portal = loop2;
  loop2->portal = loop1;

  auto tunel1 = new Map::Sector(map);
  tunel1->ceiling.loadFromFile("../Resources/Textures/ceiling.gif");
  tunel1->floor.loadFromFile("../Resources/Textures/floor.gif");

  tunel1->ceilingheight = 150;
  tunel1->floorheight = 0;

  auto loop4 = new Map::Line(tunel1, {800, -1000}, {800, -600}, portalSide);
  new Map::Line(tunel1, {800, -600}, {1400, -600}, side1);
  auto loop5 = new Map::Line(tunel1, {1400, -600}, {1400, -1000}, portalSide);
  new Map::Line(tunel1, {1400, -1000}, {800, -1000}, side1);

  loop3->portal = loop4;
  loop4->portal = loop3;

  loop5->portal = loop0;
  loop0->portal = loop5;

  Game game(1920 , 1080, "Teleport!");

  game.setMap(map);
  game.gameLoop();
}