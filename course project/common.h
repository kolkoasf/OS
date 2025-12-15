#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <string.h>

#include "os.h"
#include "sync.h"

#define BOARD_SIZE 10
#define MAX_GAMES 16
#define MAX_PLAYERS_PER_GAME 2
#define MAX_GAME_NAME 128
#define MAX_LOGIN 64
#define MAX_GAME_ID 32

typedef enum { EMPTY = 0, SHIP = 1, HIT = 2, MISS = 3 } CellState;

typedef enum {
  GAME_WAITING = 0,
  GAME_SETUP = 1,
  GAME_RUNNING = 2,
  GAME_FINISHED = 3
} GameStatus;

typedef struct {
  CellState cells[BOARD_SIZE][BOARD_SIZE];
} Board;

typedef struct {
  char login[MAX_LOGIN];
  int ready;
  Board board;
  Board shots;
} Player;

typedef struct {
  char game_id[MAX_GAME_ID];
  char game_name[MAX_GAME_NAME];
  Player players[MAX_PLAYERS_PER_GAME];
  int player_count;
  int current_turn;
  int status;
  int winner_idx;
  int stats_updated;

  struct {
    int active;
    int row;
    int col;
    int result;
    int processed_by_opponent;
  } last_shots[MAX_PLAYERS_PER_GAME];

  int last_update_version;

  OSSyncMutex game_mutex;
  OSSyncCondVar shot_changed;
} GameState;

typedef struct {
  GameState games[MAX_GAMES];
  int game_count;

  OSSyncMutex state_mutex;
  OSSyncCondVar state_changed;
} ServerState;

typedef struct {
  char login[MAX_LOGIN];
  int wins;
  int losses;
  int games_played;
} PlayerStats;

#endif  // COMMON_H