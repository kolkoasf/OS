#include "stats.h"

typedef struct {
  PlayerStats players[MAX_PLAYERS_STATS];
  int count;
} StatsDatabase;

static StatsDatabase db;
static int db_initialized = 0;

int stats_init() {
  memset(&db, 0, sizeof(StatsDatabase));

  FILE *f = fopen(STATS_FILE, "rb");
  if (f) {
    fread(&db, sizeof(StatsDatabase), 1, f);
    fclose(f);
  }

  db_initialized = 1;
  return 1;
}

static int find_or_create_player(const char *login) {
  for (int i = 0; i < db.count; i++) {
    if (strcmp(db.players[i].login, login) == 0) {
      return i;
    }
  }

  if (db.count >= MAX_PLAYERS_STATS) {
    fprintf(stderr, "Error: statistics database is full\n");
    return -1;
  }

  int idx = db.count;
  memset(&db.players[idx], 0, sizeof(PlayerStats));
  strcpy(db.players[idx].login, login);
  db.count++;
  return idx;
}

int load_player_stats(const char *login, PlayerStats *stats) {
  if (!db_initialized) stats_init();

  int idx = find_or_create_player(login);
  if (idx < 0) return 0;

  *stats = db.players[idx];
  return 1;
}

int save_player_stats(PlayerStats *stats) {
  if (!db_initialized) stats_init();

  int idx = find_or_create_player(stats->login);
  if (idx < 0) return 0;

  db.players[idx] = *stats;

  FILE *f = fopen(STATS_FILE, "wb");
  if (!f) {
    perror("fopen");
    return 0;
  }

  fwrite(&db, sizeof(StatsDatabase), 1, f);
  fclose(f);
  return 1;
}

int update_player_stats(const char *login, int is_win, int is_loss) {
  if (!db_initialized) stats_init();

  PlayerStats stats;
  load_player_stats(login, &stats);

  if (is_win) {
    stats.wins++;
    stats.games_played++;
  } else if (is_loss) {
    stats.losses++;
    stats.games_played++;
  }

  return save_player_stats(&stats);
}

void display_player_stats(const char *login) {
  if (!db_initialized) stats_init();

  PlayerStats stats;
  load_player_stats(login, &stats);

  printf("\n========== PLAYER STATISTICS ==========\n");
  printf("Login: %s\n", stats.login);
  printf("Wins: %d\n", stats.wins);
  printf("Loses: %d\n", stats.losses);
  printf("Total games: %d\n", stats.games_played);

  if (stats.games_played > 0) {
    double win_rate = (double)stats.wins / stats.games_played * 100;
    printf("Win rate: %.1f%%\n", win_rate);
  }
}
