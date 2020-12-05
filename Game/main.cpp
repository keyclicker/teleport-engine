#include "../Engine/Game.hpp"
#include "../Engine/Map.hpp"



int main() {

  Map map;

  Map::Vertex vers[] = {
    {400, 400}, {400, -400}, {0, -600},
    {-400, -400}, {-400, 400}, {0, 600}
  };

  Map::Side side;
  side.middle = Color(255, 0, 0);
  side.top = Color(0, 255, 0);
  side.bottom = Color(0, 0, 255);

  Map::Sector sec;

  sec.ceiling = Color(20, 20, 20);
  sec.floor = Color(40, 40, 40);
  sec.ceilingheight = 112;
  sec.floorheight = -56;
  sec.lightlevel = 60;

  map.sectors.push_back(sec);

  for (int i = 0; i < 6; ++i) {
    Map::Line line;
    line.side = side;
    line.sector = map.sectors.end() - 1;

    line.v1 = vers[i];
    line.v2 = vers[(i+1)%6];

    map.sectors.back().lines.push_back(line);
  }

  Game game(1280, 720, "Teleport!");
  game.setMap(map);

  game.gameLoop();
}