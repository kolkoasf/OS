#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include "common.h"

#define MAX_ATTEMPTS 100

void place_ship(Board *board, int row, int col, int length, int horizontal);

int is_valid_placement(Board *board, int row, int col, int length,
                       int horizontal);

void randomize_board(Board *board);

void init_board(Board *board);

int process_shot(Board *target_board, int row, int col);

int is_ship_sunk(Board *board, int row, int col);

int all_ships_sunk(Board *board);

char cell_to_char(CellState state, int reveal_ships);

#endif  // GAME_LOGIC_H
