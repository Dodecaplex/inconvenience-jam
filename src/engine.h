/*!
 * @file engine.h
 * @date 2/20/2015
 * @author Tony Chiodo (http://dodecaplex.net)
 */
#pragma once

#include "libtcod.hpp"

const unsigned ENTITY_MAX = 64;

const unsigned WIN_W = 32;
const unsigned WIN_H = 32;

const unsigned VIEW_W = 24;
const unsigned VIEW_H = 24;
const unsigned VIEW_X = 4;
const unsigned VIEW_Y = 4;

enum class TileID {
  NONE = 0,
  WALL,
  LADDER,
  PILLOW,
  SPIKE
};

struct Tile {
  TileID id;
  char flag;
  
  bool isSolid(void);
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
  Entity(void);
  Entity(EntityID id, unsigned x=0, unsigned y=0);
  void update(void);
  void draw(void);

  EntityID id;
  unsigned x;
  unsigned y;
  Step step;
  char flag;
  bool active;

};

struct Player : public Entity {
  void update(void);

  char fall;
  char length;

};

struct Level {
  Tile *tiles;
  unsigned width;
  unsigned height;
  unsigned size;

  Level(unsigned width, unsigned height);
  Level(const char *fname);
  ~Level(void);

  void draw(void);
  void set(unsigned x, unsigned y, TileID id);
  Tile &get(unsigned x, unsigned y);
};

struct Engine {
  unsigned long t;
  bool quit;

  unsigned cam_x;
  unsigned cam_y;
  TCOD_key_t lastkey;

  Player player;
  Entity entities[ENTITY_MAX];

  Level *current_level;

  Engine(void);
  void init(void);

  void run(void);
  void update(void);
  void draw(void);
};

extern Engine ENGINE;
