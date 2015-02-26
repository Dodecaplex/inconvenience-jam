/*!
 * @file engine.cpp
 * @date 2/20/2015
 * @author Tony Chiodo (http://dodecaplex.net)
 */
#include <iostream>
#include <fstream>
#include <string>
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
  Tile *tile;
  switch (step) {
    case Step::NONE:
    default: break;

    case Step::LEFT:
      tile = &ENGINE.current_level->get(x - 1, y);
      if (!tile->isSolid()) {
        x = (x - 1) % ENGINE.current_level->width;
      }
      else if (!ENGINE.current_level->get(x, y - 1).isSolid() &&
               !ENGINE.current_level->get(x - 1, y - 1). isSolid()) {
        x = (x - 1) % ENGINE.current_level->width;
        y = (y - 1) % ENGINE.current_level->height;
      }
      break;
    case Step::RIGHT:
      tile = &ENGINE.current_level->get(x + 1, y);
      if (!tile->isSolid()) {
        x = (x + 1) % ENGINE.current_level->width;
      }
      else if (!ENGINE.current_level->get(x, y - 1).isSolid() &&
               !ENGINE.current_level->get(x + 1, y - 1). isSolid()) {
        x = (x + 1) % ENGINE.current_level->width;
        y = (y - 1) % ENGINE.current_level->height;
      }
      break;
    case Step::UP:
      tile = &ENGINE.current_level->get(x, y - 1);
      if (!tile->isSolid()) {
        y = (y - 1) % ENGINE.current_level->height;
      }
      break;
    case Step::DOWN:
      tile = &ENGINE.current_level->get(x, y + 1);
      if (!tile->isSolid()) {
        y = (y + 1) % ENGINE.current_level->height;
      }
      break;
  }
  step = Step::NONE;
}

void Player::update(void) {
  // Apply gravity
  Tile &bel = ENGINE.current_level->get(x, y + 1);
  if (!bel.isSolid() && (bel.id != TileID::LADDER ||
      ENGINE.current_level->get(x, y).id != TileID::LADDER)) {
    std::cerr << "FALLING! ";
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
  if (!(x == prev_x && y == prev_y)) { // Spawn a wall tile behind the player
    //ENGINE.current_level->set(prev_x, prev_y, TileID::WALL);
  }
}

void Entity::draw(void) {
  if (!active) return;
  char c;
  switch (id) {
    case EntityID::NONE: c = '\0'; return;
    case EntityID::PLAYER: c = '@'; break;
    default: c = '?'; break;
  }

  // Because the camera view wraps around the edges of the level,
  // it may be necessary to draw entities multiple times, offset
  // by multiples of the level's width and height.
  unsigned i, j;
  for (unsigned ky = 0; true; ++ky) {
    j = y + VIEW_Y - ENGINE.cam_y + ky * ENGINE.current_level->height;
    if (j >= VIEW_Y + VIEW_H) break;

    for (unsigned kx = 0; true; ++kx) {
      i = x + VIEW_X - ENGINE.cam_x + kx * ENGINE.current_level->width;
      if (i >= VIEW_X + VIEW_W) break;
      TCODConsole::root->putChar(i, j, c);

      // Draw wrapped columns
      if (kx > 0) {
        i = x + VIEW_X - ENGINE.cam_x - kx * ENGINE.current_level->width;
        if (i >= VIEW_X + VIEW_W) continue;
        TCODConsole::root->putChar(i, j, c);
      }
    }

    // Draw wrapped rows
    if (ky > 0) {
      j = y + VIEW_Y - ENGINE.cam_y - ky * ENGINE.current_level->height;
      if (j >= VIEW_Y + VIEW_H) continue;

      for (unsigned kx = 0; true; ++kx) {
        i = x + VIEW_X - ENGINE.cam_x + kx * ENGINE.current_level->width;
        if (i >= VIEW_X + VIEW_W) break;

        TCODConsole::root->putChar(i, j, c);

        // Draw wrapped columns
        if (kx > 0) {
          i = x + VIEW_X - ENGINE.cam_x - kx * ENGINE.current_level->width;
          if (i >= VIEW_X + VIEW_W) continue;
          TCODConsole::root->putChar(i, j, c);
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

Level::Level(const char *fname) {
  std::ifstream fin(fname);
  if (fin.good()) {
    width = 1; height = 1;
    unsigned w, h;
    fin >> w >> h;
    for (unsigned i = 0; i < w; ++i) width *= 2;
    for (unsigned i = 0; i < h; ++i) height *= 2;
    size = width * height;
    tiles = new Tile[size];
    fin.ignore(256, '\n');

    std::string line;
    for (unsigned j = 0; j < height; ++j) {
      std::getline(fin, line);
      for (unsigned i = 0; i < width; ++i) {
        TileID id;
        if (i >= line.length()) {
          id = TileID::NONE;
        }
        else {
          switch (line[i]) {
            default:
            case ' ': id = TileID::NONE; break;
            case '#': id = TileID::WALL; break;
            case 'H': id = TileID::LADDER; break;
            case 'o': id = TileID::PILLOW; break;
            case 'x': id = TileID::SPIKE; break;
            case '@':
              id = TileID::NONE;
              ENGINE.player.x = i;
              ENGINE.player.y = j;
              break;
          }
        }
        tiles[i + width * j].id = id;
      }
    }
  }
  else {
    std::cerr << "Error loading level from " << fname << std::endl;
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
    yy = (j + ENGINE.cam_y - VIEW_Y);
    for (unsigned i = VIEW_X; i < VIEW_X + VIEW_W; ++i) {
      xx = (i + ENGINE.cam_x - VIEW_X);
      char c = '\0';
      Tile &tile = get(xx, yy);
      switch (tile.id) {
        case TileID::NONE: c = ' '; break;
        case TileID::WALL: c = '#'; break;
        case TileID::LADDER: c = 'H'; break;
        case TileID::PILLOW: c = 'o'; break;
        case TileID::SPIKE: c = 'x'; break;
        default: c = '?';
      }
      if (c != '\0')
        TCODConsole::root->putChar(i, j, c);
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
  state = EngineState::INTRO;
  player.id = EntityID::PLAYER;
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
  switch (state) { // Select update function according to Engine state
    case EngineState::INTRO: update_intro(); break;
    case EngineState::MENU:  update_menu(); break;
    case EngineState::GAME:  update_game(); break;
    case EngineState::QUIT:  update_quit(); break;
  }
}

void Engine::update_intro(void) {
  if (t++) {
    TCODSystem::sleepMilli(2000);
    state = EngineState::MENU;
    t = 0;
  }
}

void Engine::update_menu(void) {
  // Process input for menu navigation
  if (!getKeypress()) return;
  switch (lastkey.vk) {
    case TCODK_ENTER: // Confirm
      switch (menu_selection) {
        case MenuItem::NEW:
          // TODO: Load first level from file.
          current_level = new Level("level.txt");
          moveCamera();
          state = EngineState::GAME;
          break;
        case MenuItem::CONTINUE:
          // TODO: Load saved game. 
          break;
        case MenuItem::QUIT:
          state = EngineState::QUIT;
          return;
      }
      break;
    case TCODK_UP:
      if (menu_selection) {
        --menu_selection;
      }
      break;
    case TCODK_DOWN:
      if (menu_selection < MenuItem::QUIT) {
        ++menu_selection;
      }
      break;
    case TCODK_ESCAPE:
      state = EngineState::QUIT;
      return;
    default: break;
  }
}

void Engine::update_game(void) {
  // Process input if player is not falling
  if (!player.fall &&
     (current_level->get(player.x, player.y + 1).isSolid() ||
      current_level->get(player.x, player.y + 1).id == TileID::LADDER)) {
    if (!getKeypress()) return;
  }
  else { // Otherwise, pretend no keys were pressed and wait.
    lastkey.pressed = false;
    TCODSystem::sleepMilli(50);
  }

  if (lastkey.pressed) { // Process input
    Tile *tile;
    switch (lastkey.vk) {
      default: break;
      case TCODK_ESCAPE:
        state = EngineState::QUIT;
        return;
        break;
      case TCODK_ENTER:
        std::cerr << "ENTER" << std::endl;
        break;

      case TCODK_LEFT:
        tile = &current_level->get(player.x, player.y + 1);
        if (tile->isSolid() || tile->id == TileID::LADDER) {
          player.step = Step::LEFT;
        }
        break;
      case TCODK_RIGHT:
        tile = &current_level->get(player.x, player.y + 1);
        if (tile->isSolid() || tile->id == TileID::LADDER) {
          player.step = Step::RIGHT;
        }
        break;
      case TCODK_UP:
        tile = &current_level->get(player.x, player.y);
        if (tile->id == TileID::LADDER) {
          player.step = Step::UP;
        }
        break;
      case TCODK_DOWN:
        tile = &current_level->get(player.x, player.y + 1);
        if (tile->id == TileID::LADDER) {
          player.step = Step::DOWN;
        }
        break;
      case TCODK_SPACE:
        player.step = Step::NONE;
        break;
    }
  }

  // Update entities and camera
  player.update();
  for (auto &i: entities) { i.update(); }
  moveCamera();

  // Increment time
  ++t;
}

void Engine::update_quit(void) {
  quit = true;
}

void Engine::draw(void) {
  switch (state) { // Select draw function according to Engine state
    case EngineState::INTRO: draw_intro(); break;
    case EngineState::MENU:  draw_menu(); break;
    case EngineState::GAME:  draw_game(); break;
    case EngineState::QUIT:  draw_quit(); break;
    default: break;
  }

  TCODConsole::flush();
}

void Engine::draw_intro(void) {
  // Draw :: DODECAPLEX :: Logo
  unsigned i = WIN_W / 2 - 8, j = WIN_H / 2 - 10; // Top left corner
  TCODConsole::root->setDefaultBackground(TCODColor::red);
  TCODConsole::root->rect(i + 5, j + 1, 6, 4, 0, TCOD_BKGND_SET);
  TCODConsole::root->rect(i + 4, j + 2, 8, 2, 0, TCOD_BKGND_SET);
  TCODConsole::root->setDefaultBackground(TCODColor::yellow);
  TCODConsole::root->rect(i + 5, j + 5, 6, 7, 0, TCOD_BKGND_SET);
  TCODConsole::root->rect(i + 4, j + 7, 8, 4, 0, TCOD_BKGND_SET);
  TCODConsole::root->rect(i + 6, j + 12, 4, 1, 0, TCOD_BKGND_SET);
  TCODConsole::root->setDefaultBackground(TCODColor::fuchsia);
  TCODConsole::root->rect(i + 3, j + 2, 1, 2, 0, TCOD_BKGND_SET);
  TCODConsole::root->rect(i + 2, j + 3, 1, 2, 0, TCOD_BKGND_SET);
  TCODConsole::root->rect(i + 3, j + 4, 2, 3, 0, TCOD_BKGND_SET);
  TCODConsole::root->rect(i + 1, j + 5, 3, 6, 0, TCOD_BKGND_SET);
  TCODConsole::root->setDefaultBackground(TCODColor::green);
  TCODConsole::root->rect(i + 12, j + 2, 1, 2, 0, TCOD_BKGND_SET);
  TCODConsole::root->rect(i + 13, j + 3, 1, 2, 0, TCOD_BKGND_SET);
  TCODConsole::root->rect(i + 11, j + 4, 2, 3, 0, TCOD_BKGND_SET);
  TCODConsole::root->rect(i + 12, j + 5, 3, 6, 0, TCOD_BKGND_SET);
  TCODConsole::root->setDefaultBackground(TCODColor::blue);
  TCODConsole::root->rect(i + 2, j + 11, 3, 2, 0, TCOD_BKGND_SET);
  TCODConsole::root->rect(i + 3, j + 12, 3, 2, 0, TCOD_BKGND_SET);
  TCODConsole::root->rect(i + 5, j + 13, 3, 2, 0, TCOD_BKGND_SET);
  TCODConsole::root->setDefaultBackground(TCODColor::cyan);
  TCODConsole::root->rect(i + 11, j + 11, 3, 2, 0, TCOD_BKGND_SET);
  TCODConsole::root->rect(i + 10, j + 12, 3, 2, 0, TCOD_BKGND_SET);
  TCODConsole::root->rect(i + 8, j + 13, 3, 2, 0, TCOD_BKGND_SET);
  TCODConsole::root->setDefaultBackground(TCODColor::black);
  TCODConsole::root->printEx(WIN_W / 2, WIN_H / 2 + 6,
                             TCOD_BKGND_NONE, TCOD_CENTER,
                             ":: DODECAPLEX ::\npresents");
}

void Engine::draw_menu(void) {
  TCODConsole::root->clear();
  std::string menustr(":: TITLE ::\n\n"); // TODO: what's this game called?
  switch (menu_selection) {
    case MenuItem::NEW:
      menustr += "> NEW <\n\nCONTINUE\n\nQUIT"; break;
    case MenuItem::CONTINUE:
      menustr += "NEW\n\n> CONTINUE <\n\nQUIT"; break;
    case MenuItem::QUIT:
      menustr += "NEW\n\nCONTINUE\n\n> QUIT <"; break;
    default: break;
  }
  TCODConsole::root->printEx(WIN_W / 2, 8,
                             TCOD_BKGND_NONE, TCOD_CENTER,
                             menustr.c_str());
}

void Engine::draw_game(void) {
  TCODConsole::root->printFrame(VIEW_X - 1, VIEW_Y - 1,
                                VIEW_W + 2, VIEW_H + 2, false);
  current_level->draw();
  player.draw();
  for (auto &i: entities) { i.draw(); }
}

void Engine::draw_quit(void) {
}

bool Engine::getKeypress(void) {
  lastkey = TCODConsole::waitForKeypress(true);
  return lastkey.pressed;
}

void Engine::moveCamera(void) {
  cam_x = player.x - VIEW_W / 2;
  cam_y = player.y - VIEW_H / 2;
}
