/*!
 * @file engine.cpp
 * @date 2/20/2015
 * @author Tony Chiodo (http://dodecaplex.net)
 */
#include <iostream>
#include "engine.h"

Engine ENGINE;
////////////////////////////////////////////////////////////////////////////////
// Tile
bool Tile::isSolid(void) {
  return id == TileID::WALL ||
         id == TileID::PILLOW ||
         id == TileID::SPIKE;
}

////////////////////////////////////////////////////////////////////////////////
// Entity
Entity::Entity(void) {
  active = true;
}

Entity::Entity(EntityID id, unsigned x, unsigned y) {
  this->id = id;
  this->x = x;
  this->y = y;
  active = true;
}

void Entity::update(void) {
  if (!active) return;

  // Update position
  switch (step) {
    case Step::NONE:
    default: break;

    case Step::LEFT:
      if (!ENGINE.current_level->get(x - 1, y).isSolid()) {
        x = (x - 1) % ENGINE.current_level->width;
      }
      else if (!ENGINE.current_level->get(x, y - 1).isSolid()) {
        if (!ENGINE.current_level->get(x - 1, y - 1). isSolid()) {
          x = (x - 1) % ENGINE.current_level->width;
          y = (y - 1) % ENGINE.current_level->height;
        }
      }
      break;
    case Step::RIGHT:
      if (!ENGINE.current_level->get(x + 1, y).isSolid()) {
        x = (x + 1) % ENGINE.current_level->width;
      }
      else if (!ENGINE.current_level->get(x, y - 1).isSolid()) {
        if (!ENGINE.current_level->get(x + 1, y - 1). isSolid()) {
          x = (x + 1) % ENGINE.current_level->width;
          y = (y - 1) % ENGINE.current_level->height;
        }
      }
      break;
    case Step::UP:
      if (!ENGINE.current_level->get(x, y - 1).isSolid()) {
        y = (y - 1) % ENGINE.current_level->height;
      }
      break;
    case Step::DOWN:
      if (!ENGINE.current_level->get(x, y + 1).isSolid()) {
        y = (y + 1) % ENGINE.current_level->height;
      }
      break;
  }
  step = Step::NONE;
}

void Player::update(void) {
  // Apply gravity
  Tile &bel = ENGINE.current_level->get(x, y + 1);
  if (!(bel.isSolid() || bel.id == TileID::LADDER)) {
    step = Step::DOWN;
    if (fall != 0xFF) {
      ++fall;
    }
  }
  else {
    fall = 0;
  }

  unsigned prev_x = x;
  unsigned prev_y = y;
  std::cerr << "updating player: ";
  Entity::update();
  std::cerr << "x = " << x << "; y = " << y << ";" << std::endl;
  if (!(x == prev_x && y == prev_y)) {
    ENGINE.current_level->set(prev_x, prev_y, TileID::WALL);
  }
}

void Entity::draw(void) {
  if (!active) return;

  unsigned i, j;
  for (unsigned ky = 0; true; ++ky) {
    j = y + VIEW_Y - ENGINE.cam_y + ky * ENGINE.current_level->height;
    if (j >= VIEW_Y + VIEW_H) break;

    for (unsigned kx = 0; true; ++kx) {
      i = x + VIEW_X - ENGINE.cam_x + kx * ENGINE.current_level->width;
      if (i >= VIEW_X + VIEW_W) break;

      char c;
      switch (id) {
        case EntityID::NONE: c = '\0'; break;
        case EntityID::PLAYER: c = '@'; break;
        default: c = '?'; break;
      }

      if (c != '\0') {
        TCODConsole::root->putChar(i, j, c);
      }

      if (kx > 0) {
        i = x + VIEW_X - ENGINE.cam_x - kx * ENGINE.current_level->width;
        if (i >= VIEW_X + VIEW_W) continue;

        if (c != '\0') {
          TCODConsole::root->putChar(i, j, c);
        }
      }
    }

    if (ky > 0) {
      j = y + VIEW_Y - ENGINE.cam_y - ky * ENGINE.current_level->height;
      if (j >= VIEW_Y + VIEW_H) continue;

      for (unsigned kx = 0; true; ++kx) {
        i = x + VIEW_X - ENGINE.cam_x + kx * ENGINE.current_level->width;
        if (i >= VIEW_X + VIEW_W) break;

        char c;
        switch (id) {
          case EntityID::NONE: c = '\0'; break;
          case EntityID::PLAYER: c = '@'; break;
          default: c = '?'; break;
        }

        if (c != '\0') {
          TCODConsole::root->putChar(i, j, c);
        }

        if (kx > 0) {
          i = x + VIEW_X - ENGINE.cam_x - kx * ENGINE.current_level->width;
          if (i >= VIEW_X + VIEW_W) continue;

          if (c != '\0') {
            TCODConsole::root->putChar(i, j, c);
          }
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
// Level
Level::Level(unsigned width, unsigned height) {
  this->width = width;
  this->height = height;
  size = width * height;
  tiles = new Tile[size];
  for (unsigned i = 0; i < size; ++i) {
    tiles[i].id = TileID::NONE;
  }
}

Level::~Level(void) {
  for (unsigned i = 0; i < size; ++i) {
    delete &(tiles[i]);
  }
  delete [] tiles;
}

void Level::draw(void) {
  unsigned xx, yy;
  for (unsigned j = VIEW_Y; j < VIEW_Y + VIEW_H; ++j) {
    yy = (j + ENGINE.cam_y - VIEW_Y) % height;
    for (unsigned i = VIEW_X; i < VIEW_X + VIEW_W; ++i) {
      xx = (i + ENGINE.cam_x - VIEW_X) % width;
      Tile &tile = get(xx, yy);
      switch (tile.id) {
        case TileID::NONE:
        default:
          TCODConsole::root->putChar(i, j, ' ');
          break;

        case TileID::WALL:
          TCODConsole::root->putChar(i, j, '#');
          break;
      }
    }
  }
}

void Level::set(unsigned x, unsigned y, TileID id) {
  x %= width;
  y %= height;
  tiles[x + width * y].id = id;
}

Tile &Level::get(unsigned x, unsigned y) {
  x %= width;
  y %= height;
  return tiles[x + width * y];
}

////////////////////////////////////////////////////////////////////////////////
// Engine
Engine::Engine(void) {
  init();
}

void Engine::init(void) {
  player.id = EntityID::PLAYER;

  current_level = new Level(64, 64);
  for (unsigned j = 0; j < current_level->height; ++j) {
    for (unsigned i = 0; i < current_level->width; ++i) {
      // Make a floor for now.
      current_level->set(i, j, (j==i+1 || j==63)?TileID::WALL:TileID::NONE);
    }
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
  // Process input if player is not falling
  if (!player.fall && current_level->get(player.x, player.y + 1).isSolid()) {
    lastkey = TCODConsole::waitForKeypress(true);
    if (!lastkey.pressed) return; // Do not update on key release
  }
  else { // Otherwise, pretend no keys were pressed and wait.
    lastkey.pressed = false;
    TCODSystem::sleepMilli(50);
  }

  // std::cerr << t << std::endl;

  // Process input
  if (lastkey.pressed) {
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
        if (current_level->get(player.x, player.y + 1).isSolid()) {
          player.step = Step::LEFT;
        }
        break;
      case TCODK_RIGHT:
        if (current_level->get(player.x, player.y + 1).isSolid()) {
          player.step = Step::RIGHT;
        }
        break;
      case TCODK_UP:
        if (current_level->get(player.x, player.y).id == TileID::LADDER) {
          player.step = Step::UP;
        }
        break;
      case TCODK_DOWN:
        if (current_level->get(player.x, player.y + 1).id == TileID::LADDER) {
          player.step = Step::DOWN;
        }
        break;
      case TCODK_SPACE:
        player.step = Step::NONE;
        break;
    }
  }

  // Update entities
  player.update();
  for (auto &i: entities) { i.update(); }

  // Update camera position
  /*
  if (player.x >= cam_x + VIEW_W) {
    cam_x += VIEW_W;
  }
  else if (player.x < cam_x) {
    cam_x -= VIEW_W;
  }

  if (player.y >= cam_y + VIEW_H) {
    cam_y += VIEW_H;
  }
  else if (player.y < cam_y) {
    cam_y -= VIEW_H;
  }
  */
  cam_x = player.x - VIEW_W / 2;
  cam_y = player.y - VIEW_H / 2;

  // Increment time
  ++t;
}

void Engine::draw(void) {
  TCODConsole::root->clear();

  current_level->draw();
  player.draw();
  for (auto &i: entities) { i.draw(); }

  TCODConsole::flush();
}
