#include "game_logic.h"

void init_board(Board *board) {
  for (int i = 0; i < BOARD_SIZE; i++) {
    for (int j = 0; j < BOARD_SIZE; j++) {
      board->cells[i][j] = EMPTY;
    }
  }
}

int is_valid_placement(Board *board, int row, int col, int length,
                       int horizontal) {
  if (horizontal) {
    if (col + length > BOARD_SIZE) return 0;
  } else {
    if (row + length > BOARD_SIZE) return 0;
  }

  for (int i = 0; i < length; i++) {
    int r = horizontal ? row : row + i;
    int c = horizontal ? col + i : col;

    if (board->cells[r][c] != EMPTY) {
      return 0;
    }
  }

  if (horizontal) {
    if (col > 0) {
      for (int r = row - 1; r <= row + 1; r++) {
        if (r >= 0 && r < BOARD_SIZE) {
          if (board->cells[r][col - 1] != EMPTY) return 0;
        }
      }
    }

    if (col + length < BOARD_SIZE) {
      for (int r = row - 1; r <= row + 1; r++) {
        if (r >= 0 && r < BOARD_SIZE) {
          if (board->cells[r][col + length] != EMPTY) return 0;
        }
      }
    }

    if (row > 0) {
      for (int c = col - 1; c <= col + length; c++) {
        if (c >= 0 && c < BOARD_SIZE) {
          if (board->cells[row - 1][c] != EMPTY) return 0;
        }
      }
    }
    if (row < BOARD_SIZE - 1) {
      for (int c = col - 1; c <= col + length; c++) {
        if (c >= 0 && c < BOARD_SIZE) {
          if (board->cells[row + 1][c] != EMPTY) return 0;
        }
      }
    }
  } else {
    if (row > 0) {
      for (int c = col - 1; c <= col + 1; c++) {
        if (c >= 0 && c < BOARD_SIZE) {
          if (board->cells[row - 1][c] != EMPTY) return 0;
        }
      }
    }

    if (row + length < BOARD_SIZE) {
      for (int c = col - 1; c <= col + 1; c++) {
        if (c >= 0 && c < BOARD_SIZE) {
          if (board->cells[row + length][c] != EMPTY) return 0;
        }
      }
    }

    if (col > 0) {
      for (int r = row - 1; r <= row + length; r++) {
        if (r >= 0 && r < BOARD_SIZE) {
          if (board->cells[r][col - 1] != EMPTY) return 0;
        }
      }
    }
    if (col < BOARD_SIZE - 1) {
      for (int r = row - 1; r <= row + length; r++) {
        if (r >= 0 && r < BOARD_SIZE) {
          if (board->cells[r][col + 1] != EMPTY) return 0;
        }
      }
    }
  }

  return 1;
}

void place_ship(Board *board, int row, int col, int length, int horizontal) {
  if (!is_valid_placement(board, row, col, length, horizontal)) {
    return;
  }

  for (int i = 0; i < length; i++) {
    int r = horizontal ? row : row + i;
    int c = horizontal ? col + i : col;
    board->cells[r][c] = SHIP;
  }
}

void randomize_board(Board *board) {
  init_board(board);

  int ship_lengths[] = {4, 3, 3, 2, 2, 2, 1, 1, 1, 1};
  int num_ships = 10;

  for (int ship_idx = 0; ship_idx < num_ships; ship_idx++) {
    int length = ship_lengths[ship_idx];
    int placed = 0;
    int attempts = 0;

    while (!placed && attempts < MAX_ATTEMPTS) {
      int row = rand() % BOARD_SIZE;
      int col = rand() % BOARD_SIZE;
      int horizontal = rand() % 2;

      if (is_valid_placement(board, row, col, length, horizontal)) {
        place_ship(board, row, col, length, horizontal);
        placed = 1;
      }

      attempts++;
    }

    if (!placed) {
      ship_idx--;
      if (ship_idx < 0) ship_idx = 0;
      init_board(board);
    }
  }
}

int is_ship_sunk(Board *board, int row, int col) {
  if (board->cells[row][col] != HIT) {
    return 0;
  }

  int col_start = col;
  int col_end = col;

  while (col_start > 0 && (board->cells[row][col_start - 1] == SHIP ||
                           board->cells[row][col_start - 1] == HIT)) {
    col_start--;
  }

  while (col_end < BOARD_SIZE - 1 && (board->cells[row][col_end + 1] == SHIP ||
                                      board->cells[row][col_end + 1] == HIT)) {
    col_end++;
  }

  int horizontal_intact = 0;
  for (int c = col_start; c <= col_end; c++) {
    if (board->cells[row][c] == SHIP) {
      horizontal_intact = 1;
      break;
    }
  }

  if (!horizontal_intact && col_start < col_end) {
    return 1;
  }

  int row_start = row;
  int row_end = row;

  while (row_start > 0 && (board->cells[row_start - 1][col] == SHIP ||
                           board->cells[row_start - 1][col] == HIT)) {
    row_start--;
  }

  while (row_end < BOARD_SIZE - 1 && (board->cells[row_end + 1][col] == SHIP ||
                                      board->cells[row_end + 1][col] == HIT)) {
    row_end++;
  }

  int vertical_intact = 0;
  for (int r = row_start; r <= row_end; r++) {
    if (board->cells[r][col] == SHIP) {
      vertical_intact = 1;
      break;
    }
  }

  if (!vertical_intact && row_start < row_end) {
    return 1;
  }

  if (!horizontal_intact && col_start == col_end && !vertical_intact &&
      row_start == row_end) {
    return 1;
  }

  return 0;
}

int process_shot(Board *target_board, int row, int col) {
  CellState cell = target_board->cells[row][col];

  if (cell == EMPTY) {
    target_board->cells[row][col] = MISS;
    return 0;
  } else if (cell == SHIP) {
    target_board->cells[row][col] = HIT;
    if (is_ship_sunk(target_board, row, col)) {
      return 2;
    }
    return 1;
  } else if (cell == HIT || cell == MISS) {
    return -1;
  }

  return 0;
}

int all_ships_sunk(Board *board) {
  for (int i = 0; i < BOARD_SIZE; i++) {
    for (int j = 0; j < BOARD_SIZE; j++) {
      if (board->cells[i][j] == SHIP) {
        return 0;
      }
    }
  }
  return 1;
}

char cell_to_char(CellState state, int reveal_ships) {
  switch (state) {
    case EMPTY:
      return '.';
    case SHIP:
      return reveal_ships ? 'S' : '.';
    case HIT:
      return 'X';
    case MISS:
      return 'O';
    default:
      return '?';
  }
}
