#include "../Engine/Game.hpp"
#include "../Engine/Map.hpp"



int main() {

  auto map = new Map();

  auto portalSide = std::make_shared<Map::Side >();


  portalSide->upper.loadFromFile("../Resources/Textures/brick.png");
  portalSide->lower.loadFromFile("../Resources/Textures/brick.png");

  auto side1 = std::make_shared<Map::Side >();
  side1->middle.loadFromFile("../Resources/Textures/wall.gif");

  auto sec1 = new Map::Sector(map);

  sec1->ceiling.loadFromFile("../Resources/Textures/ceiling.gif");
  sec1->floor.loadFromFile("../Resources/Textures/floor.gif");

  sec1->ceilingheight = 200;
  sec1->floorheight = 0;
  sec1->lightlevel = 60;


  new Map::Line(sec1, {400, 400}, {400, -400}, side1);
  new Map::Line(sec1, {400, -400}, {0, -600}, side1);
  new Map::Line(sec1, {0, -600}, {-400, -400}, side1);
  new Map::Line(sec1,  {-400, -400}, {-400, 400}, side1);
  new Map::Line(sec1, {-400, 400}, {0, 600}, side1);

  auto portal1 = new Map::Line(sec1, {0, 600}, {400, 400}, portalSide);

  auto side2 = std::make_shared<Map::Side >();
  side2->middle = sf::Image();
  side2->middle.loadFromFile("../Resources/Textures/ostap.jpg");

  auto sec2 = new Map::Sector(map);

  sec2->ceiling.loadFromFile("../Resources/Textures/misha.jpg");
  sec2->floor.loadFromFile("../Resources/Textures/misha.jpg");

  sec2->ceilingheight = 150;
  sec2->floorheight = 20;
  sec2->lightlevel = 60;


  auto portal2 = new Map::Line(sec2,{400, 400}, {0, 600}, portalSide);
  new Map::Line(sec2, {0, 600}, {400, 1000}, side2);
  new Map::Line(sec2, {400, 1000}, {800, 800}, side2);
  new Map::Line(sec2, {800, 800}, {400, 400}, side2);


  portal1->portal = portal2;
  portal2->portal = portal1;



  Game game(640, 480, "Teleport!");

  game.setMap(map);
  game.gameLoop();
}