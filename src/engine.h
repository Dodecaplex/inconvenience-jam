/*!
 * @file engine.h
 * @date 2/20/2015
 * @author Tony Chiodo (http://dodecaplex.net)
 */
#pragma once

#include "libtcod.hpp"

const unsigned WIN_W = 41;
const unsigned WIN_H = 32;

const unsigned ENTITY_MAX = 64;
const unsigned LEVEL_MAX = 8;

const unsigned VIEW_W = 33;
const unsigned VIEW_H = 24;
const unsigned VIEW_X = 4;
const unsigned VIEW_Y = 4;

const unsigned char CHAR_WALL = 219; // ASCII solid block

enum class TileID {
  NONE = 0,
  WALL,
  PLAYER_WALL,
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
  PLAYER,
  GEM,
  EXIT,
  KEY,
  LOCK
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
  unsigned init_x;
  unsigned init_y;
  unsigned x;
  unsigned y;
  Step step;
  char flag;
  bool active;
};

struct Player : public Entity {
  void update(void);

  char fall;
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

enum class EngineState {
  INTRO = 0,
  MENU,
  GAME,
  QUIT
};

enum MenuItem { // Non-class enum for casting to unsigned
  NEW = 0,
  CONTINUE,
  QUIT
};

struct Engine {
  EngineState state;
  unsigned menu_selection;
  unsigned long t;
  bool quit;

  unsigned cam_x;
  unsigned cam_y;
  TCOD_key_t lastkey;

  Player player;
  Entity entities[ENTITY_MAX];
  unsigned entity_count;
  unsigned gems;
  unsigned keys;

  char levelfname[LEVEL_MAX][16];
  unsigned level_index;
  Level *current_level;

  Engine(void);
  void init(void);

  void run(void);

  void update(void);
  void update_intro(void);
  void update_menu(void);
  void update_game(void);
  void update_quit(void);

  void draw(void);
  void draw_intro(void);
  void draw_menu(void);
  void draw_game(void);
  void draw_quit(void);

  bool getKeypress(void);
  void moveCamera(void);
  void save(void);
  void load(void);
  void levelReset(void);
};

extern Engine ENGINE;
