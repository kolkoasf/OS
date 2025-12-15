#include "server.h"

#include "game_logic.h"
#include "stats.h"
#include "sync_init.h"

int server_init(ServerState **state, OSMmapHandle state_handle) {
  os_file_unlink(STATE_FILE);
  state_handle = mmap_create(STATE_FILE, sizeof(ServerState));
  *state = (ServerState *)state_handle.addr;

  if (*state == NULL) {
    fprintf(stderr, "Error: failed to create state file\n");
    return 0;
  }

  (*state)->game_count = 0;
  printf("Server initialized\n");
  return 1;
}

void server_cleanup(ServerState *state, OSMmapHandle handle) {
  mmap_close(handle, sizeof(ServerState));
}

int create_game(ServerState *state, const char *game_name,
                const char *player_login, char *out_game_id) {
  os_mutex_lock(&state->state_mutex);

  if (state->game_count >= MAX_GAMES) {
    fprintf(stderr, "Error: maximum number of games reached\n");
    os_mutex_unlock(&state->state_mutex);
    return 0;
  }

  GameState *game = &state->games[state->game_count];
  snprintf(out_game_id, MAX_GAME_ID, "%d", state->game_count);
  strcpy(game->game_id, out_game_id);
  strcpy(game->game_name, game_name);

  strcpy(game->players[0].login, player_login);
  game->players[0].ready = 0;
  init_board(&game->players[0].board);
  init_board(&game->players[0].shots);

  game->player_count = 1;
  game->current_turn = 0;
  game->status = GAME_WAITING;
  game->winner_idx = -1;
  game->stats_updated = 0;

  for (int i = 0; i < 2; i++) {
    game->last_shots[i].result = -1;
    game->last_shots[i].processed_by_opponent = 0;
  }

  if (!init_game_mutexes(game)) {
    fprintf(stderr, "Error initializing game mutexes\n");
    os_mutex_unlock(&state->state_mutex);
    return 0;
  }

  game->last_update_version = 0;
  state->game_count++;
  os_condvar_broadcast(&state->state_changed);
  os_mutex_unlock(&state->state_mutex);

  printf("Game '%s' created (ID: %s)\n", game_name, out_game_id);
  return 1;
}

int join_game(ServerState *state, const char *game_id,
              const char *player_login) {
  os_mutex_lock(&state->state_mutex);

  for (int i = 0; i < state->game_count; i++) {
    if (strcmp(state->games[i].game_id, game_id) == 0) {
      GameState *game = &state->games[i];

      if (game->player_count >= MAX_PLAYERS_PER_GAME) {
        fprintf(stderr, "Error: game is full\n");
        os_mutex_unlock(&state->state_mutex);
        return 0;
      }

      if (game->status != GAME_WAITING) {
        fprintf(stderr, "Error: game already started\n");
        os_mutex_unlock(&state->state_mutex);
        return 0;
      }

      os_mutex_lock(&game->game_mutex);

      if (game->player_count >= MAX_PLAYERS_PER_GAME) {
        fprintf(stderr, "Error: game is full\n");
        os_mutex_unlock(&game->game_mutex);
        os_mutex_unlock(&state->state_mutex);
        return 0;
      }

      strcpy(game->players[1].login, player_login);
      game->players[1].ready = 0;
      init_board(&game->players[1].board);
      init_board(&game->players[1].shots);
      game->player_count++;
      game->status = GAME_SETUP;

      os_condvar_broadcast(&game->shot_changed);
      os_condvar_broadcast(&state->state_changed);
      os_mutex_unlock(&game->game_mutex);
      os_mutex_unlock(&state->state_mutex);

      printf("Player '%s' joined the game\n", player_login);
      return 1;
    }
  }

  os_mutex_unlock(&state->state_mutex);
  fprintf(stderr, "Error: game not found\n");
  return 0;
}

GameState *get_game(ServerState *state, const char *game_id) {
  for (int i = 0; i < state->game_count; i++) {
    if (strcmp(state->games[i].game_id, game_id) == 0) {
      return &state->games[i];
    }
  }
  return NULL;
}

void list_games(ServerState *state) {
  printf("\n========== AVAILABLE GAMES ==========\n");

  if (state->game_count == 0) {
    printf("No available games\n");
    return;
  }

  for (int i = 0; i < state->game_count; i++) {
    GameState *game = &state->games[i];

    if (game->status == GAME_FINISHED) {
      continue;
    }

    printf("%d. [%s] %s - %d/%d players (status: ", i + 1, game->game_id,
           game->game_name, game->player_count, MAX_PLAYERS_PER_GAME);

    switch (game->status) {
      case GAME_WAITING:
        printf("waiting");
        break;
      case GAME_SETUP:
        printf("setup");
        break;
      case GAME_RUNNING:
        printf("running");
        break;
      case GAME_FINISHED:
        printf("finished");
        break;
      default:
        printf("?");
        break;
    }
    printf(")\n");
  }
}

int check_game_ready(GameState *game) {
  if (game->player_count != 2) {
    return 0;
  }

  if (game->players[0].ready && game->players[1].ready) {
    game->status = GAME_RUNNING;
    return 1;
  }

  return 0;
}

void finish_game(GameState *game, int winner_idx) {
  if (game->status != GAME_RUNNING) {
    return;
  }

  game->status = GAME_FINISHED;
  game->winner_idx = winner_idx;

  if (winner_idx >= 0 && winner_idx < 2) {
    update_player_stats(game->players[winner_idx].login, 1, 0);
    int loser_idx = 1 - winner_idx;
    update_player_stats(game->players[loser_idx].login, 0, 1);
    printf("Game finished! Winner: %s\n", game->players[winner_idx].login);
  }
}

int process_server_shot(GameState *game, int shooter_idx, int row, int col) {
  if (game->status != GAME_RUNNING) {
    return -1;
  }

  int opponent_idx = 1 - shooter_idx;

  if (row < 0 || row >= BOARD_SIZE || col < 0 || col >= BOARD_SIZE) {
    return -1;
  }

  Board *shooter_shots = &game->players[shooter_idx].shots;

  if (shooter_shots->cells[row][col] != EMPTY) {
    return -1;
  }

  Board *opponent_board = &game->players[opponent_idx].board;
  int result = process_shot(opponent_board, row, col);

  if (result == -1) {
    return -1;
  }

  if (result == 0) {
    shooter_shots->cells[row][col] = MISS;
  } else {
    shooter_shots->cells[row][col] = HIT;
  }

  game->players[opponent_idx].board = *opponent_board;
  game->players[shooter_idx].shots = *shooter_shots;

  printf("[SHOT] Player %d shoots [%d,%d] -> ", shooter_idx, row, col);

  if (result == 0) {
    game->current_turn = opponent_idx;
    printf("MISS. Turn goes to player %d\n", opponent_idx);
  } else if (result == 1) {
    game->current_turn = shooter_idx;
    printf("HIT! Turn stays with player %d\n", shooter_idx);
  } else if (result == 2) {
    game->current_turn = shooter_idx;
    printf("SUNK! Turn stays with player %d\n", shooter_idx);
  }

  if (all_ships_sunk(opponent_board)) {
    game->status = GAME_FINISHED;
    game->winner_idx = shooter_idx;
    printf("[VICTORY] Player %d won!\n", shooter_idx);
    return 3;
  }

  return result;
}
