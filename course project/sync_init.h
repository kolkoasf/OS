#ifndef MUTEX_INIT_H
#define MUTEX_INIT_H

#include "common.h"

int init_server_mutexes(ServerState *state);

int init_game_mutexes(GameState *game);

void cleanup_server_mutexes(ServerState *state);

#endif
