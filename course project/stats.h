#ifndef STATS_H
#define STATS_H

#include "common.h"

#define STATS_FILE "players_stats.db"
#define MAX_PLAYERS_STATS 1000

int stats_init();

int load_player_stats(const char *login, PlayerStats *stats);

int save_player_stats(PlayerStats *stats);

int update_player_stats(const char *login, int is_win, int is_loss);

void display_player_stats(const char *login);

#endif  // STATS_H
