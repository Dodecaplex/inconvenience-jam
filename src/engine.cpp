/*!
 * @file engine.cpp
 * @date 2/20/2015
 * @author Tony Chiodo (http://dodecaplex.net)
 */
#include <iostream>
#include "engine.h"

Engine ENGINE;

////////////////////////////////////////////////////////////////////////////////
// Entity
void Entity::update(void) {
  if (!active) return;
  switch (step) {
    case Step::NONE:
    default: break;

    case Step::LEFT:
      x = (x - 1) % LEVEL_W;
      break;
    case Step::RIGHT:
      x = (x + 1) % LEVEL_W;
      break;
    case Step::UP:
      y = (y - 1) % LEVEL_H;
      break;
    case Step::DOWN:
      y = (y + 1) % LEVEL_H;
      break;
  }
}

void Entity::draw(void) {
  if (!active) return;
  int xx = x + VIEW_X;
  int yy = y + VIEW_Y;

  switch (id) {
    case EntityID::NONE:
    default: break;

    case EntityID::PLAYER:
      TCODConsole::root->putChar(xx, yy, '@');
  }
}

////////////////////////////////////////////////////////////////////////////////
// Level
void Level::draw(void) {
  unsigned xx, yy;
  for (unsigned j = VIEW_Y; j < VIEW_Y + VIEW_H; ++j) {
    yy = (j + ENGINE.cam_y) % LEVEL_H;
    for (unsigned i = VIEW_X; i < VIEW_X + VIEW_W; ++i) {
      xx = (i + ENGINE.cam_x) % LEVEL_W;
      Tile &tile = tiles[xx + yy * LEVEL_W];
      switch (tile.id) {
        case TileID::NONE:
        default:
          TCODConsole::root->putChar(i, j, (xx % 4 && yy % 4)?' ':'.');
          break;

        case TileID::SOLID:
          TCODConsole::root->putChar(i, j, '#');
          break;
      }
    }
  }

}

////////////////////////////////////////////////////////////////////////////////
// Engine
Engine::Engine(void) {
  init();
}

void Engine::init(void) {
  player.id = EntityID::PLAYER;

  current_level = new Level;
  for (auto &i: current_level->tiles) {
    i.id = TileID::NONE;
    i.flag = 0;
  }
}

void Engine::run(void) {
  TCODConsole::initRoot(WIN_W, WIN_H, ":: INCONVENIENCE-JAM ::", false);
  TCODConsole::setKeyboardRepeat(300, 50);

  while (!(quit || TCODConsole::isWindowClosed())) {
    update();
    draw();
  }
}

void Engine::update(void) {
  // TCODSystem::checkForEvent(TCOD_EVENT_KEY_PRESS, NULL, NULL);
  lastkey = TCODConsole::waitForKeypress(true);
  if (!lastkey.pressed) return;

  std::cerr << t << std::endl;

  switch (lastkey.vk) {
    default: break;
    case TCODK_ESCAPE:
      quit = true;
      return;
      break;
    case TCODK_ENTER:
      std::cerr << "ENTER" << std::endl;
      break;

    case TCODK_LEFT:
      ENGINE.cam_x = (ENGINE.cam_x - 1) % LEVEL_W;
      break;
    case TCODK_RIGHT:
      ENGINE.cam_x = (ENGINE.cam_x + 1) % LEVEL_W;
      break;
    case TCODK_UP:
      ENGINE.cam_y = (ENGINE.cam_y - 1) % LEVEL_H;
      break;
    case TCODK_DOWN:
      ENGINE.cam_y = (ENGINE.cam_y + 1) % LEVEL_H;
      break;
  }

  for (auto &i: entities) { i.update(); }
  ++t;
}

void Engine::draw(void) {
  TCODConsole::root->clear();

  current_level->draw();
  for (auto &i: entities) { i.draw(); }

  TCODConsole::flush();
}
