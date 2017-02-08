#ifndef BRES_H
# define BRES_H

#include "dungeon.h"

int bres_cells(int x1, int y1, int x2, int y2, int (*func)(cell *));
cell *get_cell_between(int x1, int y1, int x2, int y2);

#endif
