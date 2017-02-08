#include <stdio.h>
#include <stdlib.h>

#include "dungeon.h"

int bres_cells(int x1, int y1, int x2, int y2, int (*func)(cell *)) {
  int val = 0;
  /* if dx is negative, simply swap the coordinates so it isn't */
  if(x2 - x1 < 0) {
    int tmp = x1;
    x1 = x2;
    x2 = tmp;
    tmp = y1;
    y1 = y2;
    y2 = tmp;
  }
  int dx = x2 - x1, dy = y2 - y1, e = 0, i = y1, j = x1;
  if(dy >= 0) {
    if(dx >= dy) {
      while(j <= x2) {
        if(func(dungeon[i][j])) val = 1;
	if(2 * (e + dy) < dx) {
	  e += dy;
	} else {
	  e += dy - dx;
	  i++;
	}
	j++;
      }
    } else {
      while(i <= y2) {
	if(func(dungeon[i][j])) val = 1;
	if(2 * (e + dx) < dy) {
	  e += dx;
	} else {
	  e += dx - dy;
	  j++;
	}
	i++;
      }
    }
  } else if(dy < 0) {
    if(dx >= dy * -1) {
      while(j <= x2) {
	if(func(dungeon[i][j])) val = 1;
	if(2 * (e + dy) > -1 * dx) {
	  e += dy;
	} else {
	  e += dy + dx;
	  i--;
	}
	j++;
      }
    } else {
      while(i >= y2) {
	if(func(dungeon[i][j])) val = 1;
	if(2 * (e + dx) < -1 * dy) {
	  e += dx;
	} else {
	  e += dx + dy;
	  j++;
	}
	i--;
      }
    }
  }
  return val;
}

cell *get_cell_between(int x1, int y1, int x2, int y2) {
  if(x1 == x2 && y1 == y2) return dungeon[y1][x1];
  int swapped = 0;
  if(x2 - x1 < 0) {
    swapped = 1;
    int tmp = x1;
    x1 = x2;
    x2 = tmp;
    tmp = y1;
    y1 = y2;
    y2 = tmp;
  }
  int dx = x2 - x1, dy = y2 - y1, e = 0, i = 0, j = 0;
  if(dy >= 0) {
    if(dx >= dy) {
      if(2 * (e + dy) < dx) {
	e += dy;
      } else {
	e += dy - dx;
	i++;
      }
      j++;
    } else {
      if(2 * (e + dx) < dy) {
	e += dx;
      } else {
	e += dx - dy;
	j++;
      }
      i++;
    }
  } else if(dy < 0) {
    if(dx >= dy * -1) {
      if(2 * (e + dy) > -1 * dx) {
	e += dy;
      } else {
	e += dy + dx;
	i--;
      }
      j++;
    } else {
      if(2 * (e + dx) < -1 * dy) {
	e += dx;
      } else {
	e += dx + dy;
	j++;
      }
      i--;
    }
  }
  if(swapped) return dungeon[y2 - i][x2 - j];
  return dungeon[y1 + i][x1 + j];
}
