#ifndef SERVER_H
#define SERVER_H

#include "common.h"

#define STATE_FILE "server_state.mmap"

int server_init(ServerState **state, OSMmapHandle state_handle);

void server_cleanup(ServerState *state, OSMmapHandle state_handle);

int create_game(ServerState *state, const char *game_name,
                const char *player_login, char *out_game_id);

int join_game(ServerState *state, const char *game_id,
              const char *player_login);

GameState *get_game(ServerState *state, const char *game_id);

void list_games(ServerState *state);

int check_game_ready(GameState *game);

void finish_game(GameState *game, int winner_idx);

int process_server_shot(GameState *game, int shooter_idx, int row, int col);

#endif  // SERVER_H
