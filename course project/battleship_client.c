#include <errno.h>
#include <pthread.h>
#include <time.h>

#include "common.h"
#include "game_logic.h"
#include "server.h"
#include "stats.h"

#define SHOT_TIMEOUT 300

static char player_login[MAX_LOGIN];
static char current_game_id[MAX_GAME_ID];
static int player_index = -1;

static ServerState *get_server_state(OSMmapHandle *handle) {
  *handle = mmap_open("server_state.mmap", sizeof(ServerState));
  return (ServerState *)handle->addr;
}

static void clear_input_buffer(void) {
  int c;
  while ((c = getchar()) != '\n' && c != EOF) {
  }
}

static int login_menu() {
  printf("\n==========================================\n");
  printf("=  BATTLESHIP - CLIENT                  =\n");
  printf("==========================================\n\n");

  printf("Enter your login (up to 63 characters): ");
  if (fgets(player_login, sizeof(player_login), stdin) == NULL) {
    return 0;
  }
  player_login[strcspn(player_login, "\n")] = 0;

  if (strlen(player_login) == 0) {
    printf("Error: login cannot be empty\n");
    return 0;
  }

  printf("Welcome, %s!\n", player_login);
  stats_init();

  return 1;
}

static int main_menu() {
  printf("\n========== MAIN MENU ==========\n");
  printf("1. Create new game\n");
  printf("2. Join a game\n");
  printf("3. View my statistics\n");
  printf("4. Exit\n");
  printf("Choose action (1-4): ");

  char choice[10];
  if (fgets(choice, sizeof(choice), stdin) == NULL) {
    return 4;
  }

  return atoi(choice);
}

static int create_game_dialog() {
  char game_name[MAX_GAME_NAME];

  printf("\nEnter name of new game: ");
  if (fgets(game_name, sizeof(game_name), stdin) == NULL) {
    return 0;
  }
  game_name[strcspn(game_name, "\n")] = 0;

  if (strlen(game_name) == 0) {
    printf("Error: game name cannot be empty\n");
    return 0;
  }

  OSMmapHandle fd;
  ServerState *state = get_server_state(&fd);
  if (state == NULL) {
    fprintf(stderr, "Error: could not connect to server\n");
    return 0;
  }

  if (create_game(state, game_name, player_login, current_game_id)) {
    mmap_close(fd, sizeof(ServerState));
    player_index = 0;
    printf("Game created! ID: %s\n", current_game_id);
    printf("Waiting for second player...\n");
    return 1;
  }

  mmap_close(fd, sizeof(ServerState));
  return 0;
}

static int join_game_dialog() {
  OSMmapHandle fd;
  ServerState *state = get_server_state(&fd);
  if (state == NULL) {
    fprintf(stderr, "Error: could not connect to server\n");
    return 0;
  }

  list_games(state);

  printf("\nEnter game ID to join (or empty to cancel): ");
  char game_id[MAX_GAME_ID];
  if (fgets(game_id, sizeof(game_id), stdin) == NULL) {
    mmap_close(fd, sizeof(ServerState));
    return 0;
  }
  game_id[strcspn(game_id, "\n")] = 0;

  if (strlen(game_id) == 0) {
    mmap_close(fd, sizeof(ServerState));
    return 0;
  }

  if (join_game(state, game_id, player_login)) {
    strcpy(current_game_id, game_id);
    mmap_close(fd, sizeof(ServerState));
    player_index = 1;
    printf("Joined the game!\n");
    return 1;
  }

  mmap_close(fd, sizeof(ServerState));
  return 0;
}

static void setup_ships(Board *board) {
  printf("\n========== SHIP PLACEMENT ==========\n");
  printf("Generating random ship placement...\n");
  printf("- 1 four-deck ship (1x4)\n");
  printf("- 2 three-deck ships (2x3)\n");
  printf("- 3 two-deck ships (3x2)\n");
  printf("- 4 one-deck ships (4x1)\n\n");

  randomize_board(board);

  printf("\n%s\n", "Ships placed randomly!");
  printf("   0 1 2 3 4 5 6 7 8 9\n");
  for (int i = 0; i < BOARD_SIZE; i++) {
    printf(" %d ", i);
    for (int j = 0; j < BOARD_SIZE; j++) {
      printf("%c ", cell_to_char(board->cells[i][j], 1));
    }
    printf("\n");
  }
}

static int get_opponent_index() { return (player_index == 0) ? 1 : 0; }

static void display_boards(const Board *my_board, const Board *my_shots) {
  printf("\nYour board:              Your shots:\n");
  printf("   0 1 2 3 4 5 6 7 8 9    0 1 2 3 4 5 6 7 8 9\n");

  for (int i = 0; i < BOARD_SIZE; i++) {
    printf(" %d ", i);
    for (int j = 0; j < BOARD_SIZE; j++) {
      printf("%c ", cell_to_char(my_board->cells[i][j], 1));
    }
    printf("  %d ", i);
    for (int j = 0; j < BOARD_SIZE; j++) {
      printf("%c ", cell_to_char(my_shots->cells[i][j], 0));
    }
    printf("\n");
  }
}

int wait_for_shot_processing(GameState *game, int player_idx, int max_seconds) {
  printf("...Waiting for server to process shot...\n");

  os_mutex_lock(&game->game_mutex);

  while (game->last_shots[player_idx].result == -2) {
    int wait_result = os_condvar_timedwait(
        &game->shot_changed, &game->game_mutex, max_seconds * 1000);

    if (wait_result == OS_WAIT_TIMEOUT) {
      printf("Timeout: server did not respond in %d seconds\n", max_seconds);
      os_mutex_unlock(&game->game_mutex);
      return 0;
    }

    if (wait_result == OS_WAIT_ERROR) {
      printf("Wait error\n");
      os_mutex_unlock(&game->game_mutex);
      return 0;
    }
  }

  os_mutex_unlock(&game->game_mutex);

  return 1;
}

static void play_game() {
  printf("\n========== GAME START ==========\n");
  OSMmapHandle fd;
  ServerState *state = get_server_state(&fd);

  if (state == NULL) {
    fprintf(stderr, "Error: could not connect to server\n");
    return;
  }

  GameState *game = get_game(state, current_game_id);
  if (game == NULL) {
    fprintf(stderr, "Error: game not found\n");
    mmap_close(fd, sizeof(ServerState));
    return;
  }

  int opponent_idx = get_opponent_index();
  int game_over = 0;
  int last_opponent_shot_version = -1;

  while (!game_over) {
    mmap_read(state, state, sizeof(ServerState));
    game = get_game(state, current_game_id);

    if (game == NULL) {
      fprintf(stderr, "Error: game lost\n");
      break;
    }

    if (game->last_shots[opponent_idx].result == 3) {
      printf("\n~~~ YOU LOST! ~~~\n");
      game->status = GAME_FINISHED;
      game->winner_idx = opponent_idx;
      game_over = 1;
      break;
    }

    Board my_board = game->players[player_index].board;
    Board my_shots = game->players[player_index].shots;

    printf("\n========== CURRENT STATUS ==========\n");

    display_boards(&my_board, &my_shots);

    if (game->current_turn == player_index) {
      printf("\n=== YOUR TURN ===\n");
      printf("Enter shot coordinates (row column): ");

      int row, col;
      if (scanf("%d %d", &row, &col) != 2) {
        clear_input_buffer();
        printf("Error: enter two numbers!\n");
        continue;
      }

      clear_input_buffer();

      if (row < 0 || row >= BOARD_SIZE || col < 0 || col >= BOARD_SIZE) {
        printf("Error: coordinates out of bounds (0-%d)\n", BOARD_SIZE - 1);
        continue;
      }

      if (my_shots.cells[row][col] != EMPTY) {
        printf("Error: you already shot here!\n");
        continue;
      }

      printf("...Sending shot to server...\n");

      os_mutex_lock(&game->game_mutex);

      game->last_shots[player_index].active = 1;
      game->last_shots[player_index].row = row;
      game->last_shots[player_index].col = col;
      game->last_shots[player_index].result = -2;
      game->last_shots[player_index].processed_by_opponent = 0;
      game->last_update_version++;

      for (int i = 0; i < state->game_count; i++) {
        if (strcmp(state->games[i].game_id, current_game_id) == 0) {
          state->games[i] = *game;
          mmap_write(&state->games[i], &state->games[i], sizeof(GameState));
          break;
        }
      }

      mmap_write(state, state, sizeof(ServerState));

      os_mutex_unlock(&game->game_mutex);

      if (!wait_for_shot_processing(game, player_index, 10)) {
        printf("Error: server did not process shot\n");
        continue;
      }

      mmap_read(state, state, sizeof(ServerState));
      game = get_game(state, current_game_id);
      if (game == NULL) break;

      int result = game->last_shots[player_index].result;

      if (result == -1) {
        printf("Error: invalid shot\n");
      } else if (result == 0) {
        printf("MISS! Turn goes to opponent.\n");
      } else if (result == 1) {
        printf("HIT! You shoot again!\n");
      } else if (result == 2) {
        printf("SUNK! You shoot again!\n");
      } else if (result == 3) {
        printf("*** YOU WON! ***\n");
        game_over = 1;
        break;
      }

      os_mutex_lock(&game->game_mutex);
      game->last_shots[player_index].active = 0;
      os_mutex_unlock(&game->game_mutex);

      for (int i = 0; i < state->game_count; i++) {
        if (strcmp(state->games[i].game_id, current_game_id) == 0) {
          state->games[i] = *game;
          mmap_write(&state->games[i], &state->games[i], sizeof(GameState));
          break;
        }
      }
      mmap_write(state, state, sizeof(ServerState));

    } else {
      printf("\n=== WAITING FOR OPPONENT'S TURN ===\n");

      int opponent_made_move = 0;

      os_mutex_lock(&game->game_mutex);

      while (game->current_turn == opponent_idx &&
             game->status != GAME_FINISHED && !opponent_made_move) {
        int wait_result = os_condvar_timedwait(&game->shot_changed,
                                               &game->game_mutex, 120000);

        if (wait_result == OS_WAIT_TIMEOUT) {
          printf("Opponent is taking too long\n");
          os_mutex_unlock(&game->game_mutex);
          break;
        }

        if (wait_result == OS_WAIT_ERROR) {
          os_mutex_unlock(&game->game_mutex);
          break;
        }

        if (game->last_update_version != last_opponent_shot_version) {
          opponent_made_move = 1;
        }
      }

      os_mutex_unlock(&game->game_mutex);

      mmap_read(state, state, sizeof(ServerState));
      game = get_game(state, current_game_id);
      if (game == NULL) break;

      my_board = game->players[player_index].board;
      my_shots = game->players[player_index].shots;

      if (game->last_shots[opponent_idx].result >= -1 &&
          last_opponent_shot_version != game->last_update_version) {
        last_opponent_shot_version = game->last_update_version;

        printf("\n*** OPPONENT SHOT! ***\n");
        int row = game->last_shots[opponent_idx].row;
        int col = game->last_shots[opponent_idx].col;
        int result = game->last_shots[opponent_idx].result;

        printf("Opponent shot [%d,%d]: ", row, col);

        if (result == 0) {
          printf("MISS\n");
          my_board.cells[row][col] = MISS;
        } else if (result == 1) {
          printf("HIT your cell!\n");
          my_board.cells[row][col] = HIT;
        } else if (result == 2) {
          printf("SUNK your ship!\n");
          my_board.cells[row][col] = HIT;
        } else if (result == 3) {
          printf("~~~YOU LOST!~~~\n");
        }

        game->last_shots[opponent_idx].processed_by_opponent = 1;

        for (int i = 0; i < state->game_count; i++) {
          if (strcmp(state->games[i].game_id, current_game_id) == 0) {
            state->games[i] = *game;
            mmap_write(&state->games[i], &state->games[i], sizeof(GameState));
            break;
          }
        }
        mmap_write(state, state, sizeof(ServerState));
      }
    }
  }

  mmap_close(fd, sizeof(ServerState));

  stats_init();
  printf("\nStatistics updated from server\n");
  clear_input_buffer;
}

int main() {
  init_random();

  if (!login_menu()) {
    printf("Error logging in\n");
    return 1;
  }

  int running = 1;

  while (running) {
    int choice = main_menu();

    switch (choice) {
      case 1:
        if (create_game_dialog()) {
          OSMmapHandle fd;
          ServerState *state = get_server_state(&fd);
          GameState *game = get_game(state, current_game_id);

          setup_ships(&game->players[player_index].board);
          game->players[player_index].ready = 1;
          mmap_write(game, game, sizeof(GameState));

          printf("\nWaiting for second player...\n");
          printf("(When second player connects, ship placement will begin)\n");

          int waiting = 1;
          while (waiting && game->player_count < 2) {
            os_usleep(500000);
            mmap_read(state, state, sizeof(ServerState));
            game = get_game(state, current_game_id);
            printf(".");
            fflush(stdout);
          }
          printf("\nSecond player connected!\n");

          mmap_close(fd, sizeof(ServerState));
          play_game();
        }
        break;

      case 2:
        if (join_game_dialog()) {
          OSMmapHandle fd;
          ServerState *state = get_server_state(&fd);
          GameState *game = get_game(state, current_game_id);

          setup_ships(&game->players[player_index].board);
          game->players[player_index].ready = 1;
          mmap_write(game, game, sizeof(GameState));

          printf("Waiting for first player...\n");
          int waiting = 1;
          while (waiting && !game->players[0].ready) {
            os_usleep(500000);
            mmap_read(state, state, sizeof(ServerState));
            game = get_game(state, current_game_id);
          }
          printf("Both players ready! Game starts!\n");

          game->status = GAME_RUNNING;
          mmap_write(game, game, sizeof(GameState));
          mmap_close(fd, sizeof(ServerState));
          play_game();
        }
        break;

      case 3:
        stats_init();
        display_player_stats(player_login);
        break;

      case 4:
        running = 0;
        printf("Goodbye!\n");
        break;

      default:
        printf("Invalid choice\n");
    }
  }

  return 0;
}
