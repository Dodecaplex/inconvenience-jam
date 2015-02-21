/*!
 * @file engine.h
 * @date 2/20/2015
 * @author Tony Chiodo (http://dodecaplex.net)
 */
#pragma once

#include "libtcod.hpp"

const unsigned ENTITY_MAX = 64;
const unsigned LEVEL_W = 16;
const unsigned LEVEL_H = 16;

const unsigned WIN_W = 80;
const unsigned WIN_H = 60;

const unsigned VIEW_W = 24;
const unsigned VIEW_H = 24;
const unsigned VIEW_X = 10;
const unsigned VIEW_Y = 5;

enum class TileID {
  NONE = 0,
  SOLID
};

struct Tile {
  TileID id;
  char flag;
};

enum class EntityID {
  NONE = 0,
  PLAYER
};

enum class Step {
  NONE = 0,
  LEFT, RIGHT, UP, DOWN
};

struct Entity {
  EntityID id;
  unsigned x;
  unsigned y;
  Step step;
  char flag;
  bool active;

  void update(void);
  void draw(void);
};

struct Level {
  Tile tiles[LEVEL_W * LEVEL_H];

  void draw(void);
};

struct Engine {
  Engine(void);
  void init(void);

  void run(void);
  void update(void);
  void draw(void);

  unsigned long t;
  bool quit;

  unsigned cam_x;
  unsigned cam_y;
  TCOD_key_t lastkey;

  Entity player;
  Entity entities[ENTITY_MAX];

  Level *current_level;

};

extern Engine ENGINE;
