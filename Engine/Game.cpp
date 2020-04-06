#include "Game.hpp"

void Game::gameLoop() {
  while (bf.isOpen()) {
    renderSector(map.sectors.back());
  }
}

void Game::renderSector(const Map::Sector &sec) {
  for (auto a : sec.lines) {
    
  }
}
