#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common.h"
#include "game_logic.h"
#include "server.h"
#include "stats.h"
#include "sync_init.h"

static int keep_running = 1;

void signal_handler(int sig) {
  (void)sig;
  keep_running = 0;
}

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;

  printf("═══════════════════════════════════════\n");
  printf(" BATTLESHIP - SERVER \n");
  printf("═══════════════════════════════════════\n\n");

  os_signal_register_int(signal_handler);

  OSMmapHandle state_handle;
  ServerState *state = NULL;

  if (!server_init(&state, state_handle)) {
    fprintf(stderr, "Error initializing server\n");
    return 1;
  }

  printf("Initializing synchronization primitives...\n");
  if (!init_server_mutexes(state)) {
    fprintf(stderr, "Error initializing mutexes\n");
    server_cleanup(state, state_handle);
    return 1;
  }

  printf("Synchronization primitives initialized\n\n");

  stats_init();
  printf("Statistics system initialized\n\n");

  printf("Server is running and waiting for players...\n");
  printf("Clients can connect\n\n");

  int last_game_count = 0;

  while (keep_running) {
    os_usleep(500000);

    OSMmapHandle handle = mmap_open(STATE_FILE, sizeof(ServerState));
    if (handle.addr == NULL) continue;

    mmap_read(handle.addr, state, sizeof(ServerState));

    os_mutex_lock(&state->state_mutex);

    for (int i = 0; i < state->game_count; i++) {
      GameState *game = &state->games[i];
      os_mutex_lock(&game->game_mutex);

      for (int shooter_idx = 0; shooter_idx < 2; shooter_idx++) {
        if (game->last_shots[shooter_idx].active &&
            game->last_shots[shooter_idx].result == -2) {
          int row = game->last_shots[shooter_idx].row;
          int col = game->last_shots[shooter_idx].col;

          printf("\n[GAME: %s] Processing shot from player %d\n",
                 game->game_name, shooter_idx);

          if (row < 0 || row >= BOARD_SIZE || col < 0 || col >= BOARD_SIZE) {
            printf("Error: coordinates out of bounds\n");
            game->last_shots[shooter_idx].result = -1;
          } else {
            int result = process_server_shot(game, shooter_idx, row, col);
            game->last_shots[shooter_idx].result = result;

            switch (result) {
              case 0:
                printf("MISS [%d,%d]\n", row, col);
                break;
              case 1:
                printf("HIT [%d,%d]\n", row, col);
                break;
              case 2:
                printf("SUNK [%d,%d]\n", row, col);
                break;
              case 3:
                printf("*** VICTORY! Player %d won!\n", shooter_idx);
                game->status = GAME_FINISHED;
                game->winner_idx = shooter_idx;
                break;
              case -1:
                printf("ERROR during processing\n");
                break;
            }

            game->last_update_version++;
            os_condvar_broadcast(&game->shot_changed);
            printf("Result sent (version %d)\n", game->last_update_version);
          }
        }
      }

      if (game->status == GAME_FINISHED && game->winner_idx >= 0 &&
          !game->stats_updated) {
        printf("\n[GAME %s] Finished!\n", game->game_name);
        printf(" Winner: %s\n", game->players[game->winner_idx].login);
        printf(" Loser: %s\n", game->players[1 - game->winner_idx].login);

        update_player_stats(game->players[game->winner_idx].login, 1, 0);
        int loser_idx = 1 - game->winner_idx;
        update_player_stats(game->players[loser_idx].login, 0, 1);

        game->stats_updated = 1;
        game->last_shots[0].active = 0;
        game->last_shots[1].active = 0;
        os_condvar_broadcast(&game->shot_changed);
      }

      os_mutex_unlock(&game->game_mutex);
    }

    os_mutex_unlock(&state->state_mutex);
    mmap_close(handle, sizeof(ServerState));

    if (state->game_count != last_game_count) {
      printf("\n[SERVER] Active games: %d/%d\n", state->game_count, MAX_GAMES);
      last_game_count = state->game_count;
    }
  }

  printf("\n[SERVER] Shutting down...\n");

  for (int i = 0; i < state->game_count; i++) {
    GameState *game = &state->games[i];

    os_mutex_lock(&game->game_mutex);
    game->status = GAME_FINISHED;
    game->winner_idx = -1;
    os_condvar_broadcast(&game->shot_changed);
    os_mutex_unlock(&game->game_mutex);
  }

  os_usleep(5000000);
  cleanup_server_mutexes(state);
  server_cleanup(state, state_handle);
  printf("Server stopped\n");

  return 0;
}
