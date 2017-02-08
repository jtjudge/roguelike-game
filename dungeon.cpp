#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <ncurses.h>

#include "dungeon.h"
#include "bres.h"
#include "object.h"

int num_rooms;
room **rooms;
cell *dungeon[D_HEIGHT][D_WIDTH];

room *create_room(int x, int y, int w, int h) {
  room *r = new room();
  r->x = x; r->y = y;
  r->w = w; r->h = h;
  r->connected = 0;
  int i, j;
  for(i = y + 1; i < y + h - 1; i++) {
    for(j = x + 1; j < x + w - 1; j++) {
      dungeon[i][j]->norm_map = D_FLOOR;
      dungeon[i][j]->hardness = 0;
    }
  }
  return r;
}

void destroy_room(room *r) {
  delete r;
}

cell *create_cell(int x, int y) {
  cell *c = new cell();
  c->x = x;
  c->y = y;
  if(x == 0 || x == D_WIDTH - 1 ||
     y == 0 || y == D_HEIGHT - 1) {
    c->hardness = 255;
  } else {
    c->hardness = 1 + rand() % 254;
  }
  c->norm_map = c->pc_view = D_ROCK;
  c->actor = 0;
  c->obj = 0;
  c->cursor = 0;
  return c;
}

void destroy_cell(cell *c) {
  c->actor = 0;
  delete c;
}

int mark_corr(cell *c) {
  if(c->norm_map == D_ROCK) {
    c->norm_map = D_CORR;
    c->hardness = 0;
  }
  return 0;
}

void generate_dungeon(int test_room)
{
int i, j, k;
  /* Initialize cells  */
  for(i = 0; i < D_HEIGHT; i++) {
    for(j = 0; j < D_WIDTH; j++) {
      dungeon[i][j] = create_cell(j, i);
    }
  }
  if(test_room) {
    /* Draw one big room */
    rooms[0] = create_room(0, 0, 80, 21);
    /* Place 5 big columns */
    int k;
    for(k = 0; k < 4; k++) {
      for(i = 0; i < 5; i++) {
	for(j = 0; j < 10; j++) {
	  dungeon[i + 8][5 + j + (k * 20)]->norm_map = D_ROCK;
	  dungeon[i + 8][5 + j + (k * 20)]->hardness = 170;
	}
      }
    }
    /* Place one staircase */
    dungeon[1][1]->norm_map = D_DOWN_STAIRS;
  } else {
    /* Place the rooms */
    for(i = 0; i < num_rooms; i++) {
      int rx, ry, rh, rw, reset;
      while(1) {
	reset = 0;
	rx = 1 + rand() % 71; ry = 1 + rand() % 18;
	rh = 5 + rand() % 10; rw = 6 + rand() % 15;
	for(j = ry; j < ry + rh && !reset; j++) {
	  for(k = rx; k < rx + rw && !reset; k++) {
	    if(j == D_HEIGHT || k == D_WIDTH ||
	       dungeon[j][k]->norm_map == D_FLOOR) {
	      reset = 1;
	    }
	  }
	}
	if(!reset) {
	  rooms[i] = create_room(rx, ry, rw, rh);
	  break;
	}
      }
    }   
    /* Place stairs */
    int count = 2 + rand() % 3;
    while(count > 0) {
      int rx = rand() % D_WIDTH;
      int ry = rand() % D_HEIGHT;
      if(dungeon[ry][rx]->norm_map == D_FLOOR) {
	if(rand() % 2) {
	  dungeon[ry][rx]->norm_map = D_UP_STAIRS;
	} else {
	  dungeon[ry][rx]->norm_map = D_DOWN_STAIRS;
	}
	count--;
      }
    }
    /* Draw the corridors */
    room *ri, *rj, *min;
    double min_dist, dist;
    int x1, x2, y1, y2, stop = 0;
    ri = rooms[0];
    while(!stop) {
      min_dist = INF;
      stop = 1;
      for(j = 0; j < num_rooms; j++) {
	rj = rooms[j];
	if(rj != ri && !rj->connected) {
	  dist = sqrt(pow(rj->x-ri->x, 2) + pow(rj->y-ri->y, 2));  
	  if(dist < min_dist) {
	    min_dist = dist;
	    min = rj;
	    stop = 0;
	  }
	}
      }
      if(stop) break;
      x1 = ri->x + ri->w/2;
      x2 = min->x + min->w/2;
      y1 = ri->y + ri->h/2;
      y2 = min->y + min->h/2; 
      bres_cells(x1, y1, x2, y2, mark_corr);
      min->connected = ri->connected = 1;
      ri = min;
    }
  }
}

void display_dungeon(int color, int fog, int sneak) {
  int i, j;
  for(i = 0; i < D_HEIGHT; i++) {
    for(j = 0; j < D_WIDTH; j++) {
      cell *c = dungeon[i][j];
      actor *a = c->actor;
      object *o = c->obj;
      int f = c->fog,
	s = sneak ? c->seen : 0,
	d = c->detect_map,
	cr = c->cursor;
      if(cr == 1) {
	if(a) {
	  attron(COLOR_PAIR(get_color(a)));
	  mvaddch(i + 1, j, get_icon(a));
	  attroff(COLOR_PAIR(get_color(a)));
	} else if(o && s) {
	  attron(COLOR_PAIR(o->color));
	  mvaddch(i + 1, j, o->icon);
	  attroff(COLOR_PAIR(o->color));
	} else if(o) {
	  attron(COLOR_PAIR(o->color + 1));
	  mvaddch(i + 1, j, o->icon);
	  attroff(COLOR_PAIR(o->color + 1));
	} else if(d && s) {
	  attron(COLOR_PAIR(get_color(dungeon[i+1][j]->actor)));
	  mvaddch(i + 1, j, d);
	  attroff(COLOR_PAIR(get_color(dungeon[i+1][j]->actor)));
	} else if(d) {
	  attron(COLOR_PAIR(get_color(dungeon[i+1][j]->actor) + 1));
	  mvaddch(i + 1, j, d);
	  attroff(COLOR_PAIR(get_color(dungeon[i+1][j]->actor) + 1));
	} else if(s) {
	  attron(COLOR_PAIR(WB));
	  mvaddch(i + 1, j, dungeon[i][j]->pc_view);
	  attroff(COLOR_PAIR(WB));
	} else {
	  attron(COLOR_PAIR(BAW));
	  mvaddch(i + 1, j, dungeon[i][j]->pc_view);
	  attroff(COLOR_PAIR(BAW));
	}
      } else if(cr == 2) {
	if(a) {
	  attron(COLOR_PAIR(get_color(a) + 20));
	  mvaddch(i + 1, j, get_icon(a));
	  attroff(COLOR_PAIR(get_color(a) + 20));
	} else if(o) {
	  attron(COLOR_PAIR(o->color + 20));
	  mvaddch(i + 1, j, o->icon);
	  attroff(COLOR_PAIR(o->color + 20));
	} else if(d) {
	  attron(COLOR_PAIR(get_color(dungeon[i+1][j]->actor) + 20));
	  mvaddch(i + 1, j, d);
	  attroff(COLOR_PAIR(get_color(dungeon[i+1][j]->actor) + 20));
	} else {
	  attron(COLOR_PAIR(BAR));
	  mvaddch(i + 1, j, dungeon[i][j]->pc_view);
	  attroff(COLOR_PAIR(BAR));
	}
      } else if(fog) {
	if(color == WB) {
	  if(a && f) {
	    attron(COLOR_PAIR(get_color(a) + 1));
	    mvaddch(i + 1, j, get_icon(a));
	    attroff(COLOR_PAIR(get_color(a) + 1));
	  } else if(o && f) {
	    attron(COLOR_PAIR(o->color + 1));
	    mvaddch(i + 1, j, o->icon);
	    attroff(COLOR_PAIR(o->color + 1));
	  } else if(d && f) {
	    attron(COLOR_PAIR(get_color(dungeon[i+1][j]->actor) + 1));
	    mvaddch(i + 1, j, dungeon[i][j]->detect_map);
	    attroff(COLOR_PAIR(get_color(dungeon[i+1][j]->actor) + 1));
	  } else if(f) {
	    attron(COLOR_PAIR(BAW));
	    mvaddch(i + 1, j, dungeon[i][j]->norm_map);
	    attroff(COLOR_PAIR(BAW));
	  } else {
	    attron(COLOR_PAIR(color));
	    mvaddch(i + 1, j, dungeon[i][j]->pc_view);
	    attroff(COLOR_PAIR(color));
	  }
	} else if(color == GB) {
	  if(f) {
	    attron(COLOR_PAIR(GW));
	  } else {
	    attron(COLOR_PAIR(GB));
	  }
	  if(a) {
	    mvaddch(i + 1, j, get_icon(a));
	  } else if(o) {
	    mvaddch(i + 1, j, o->icon);
	  } else if(d) {
	    mvaddch(i + 1, j, dungeon[i][j]->detect_map);
	  } else {
	    mvaddch(i + 1, j, dungeon[i][j]->norm_map);
	  }
	  if(f) {
	    attroff(COLOR_PAIR(GW));
	  } else {
	    attron(COLOR_PAIR(GB));
	  }
	} else {
	  attron(COLOR_PAIR(color));
	  if(a) {
	    mvaddch(i + 1, j, get_icon(a));
	  } else if(o) {
	    mvaddch(i + 1, j, o->icon);
	  } else if(d) {
	    mvaddch(i + 1, j, dungeon[i][j]->detect_map);
	  } else {
	    mvaddch(i + 1, j, dungeon[i][j]->norm_map);
	  }
	  attroff(COLOR_PAIR(color));
	}
      } else {
	if(color == WB) {
	  if(a && s) {
	    attron(COLOR_PAIR(get_color(a) + 1));
	    mvaddch(i + 1, j, get_icon(a));
	    attroff(COLOR_PAIR(get_color(a) + 1));
	  } else if(a) {
	    attron(COLOR_PAIR(get_color(a)));
	    mvaddch(i + 1, j, get_icon(a));
	    attroff(COLOR_PAIR(get_color(a)));
	  } else if(o && s) {
	    attron(COLOR_PAIR(o->color + 1));
	    mvaddch(i + 1, j, o->icon);
	    attroff(COLOR_PAIR(o->color + 1));
	  } else if(o) {
	    attron(COLOR_PAIR(o->color));
	    mvaddch(i + 1, j, o->icon);
	    attroff(COLOR_PAIR(o->color));
	  } else if(d && s) {
	    attron(COLOR_PAIR(get_color(dungeon[i+1][j]->actor) + 1));
	    mvaddch(i + 1, j, d);
	    attroff(COLOR_PAIR(get_color(dungeon[i+1][j]->actor) + 1));
	  } else if(d) {
	    attron(COLOR_PAIR(get_color(dungeon[i+1][j]->actor)));
	    mvaddch(i + 1, j, d);
	    attroff(COLOR_PAIR(get_color(dungeon[i+1][j]->actor)));
	  } else if(s) {
	    attron(COLOR_PAIR(BAW));
	    mvaddch(i+1, j, dungeon[i][j]->norm_map);
	    attroff(COLOR_PAIR(BAW));
	  } else {
	    attron(COLOR_PAIR(color));
	    mvaddch(i + 1, j, dungeon[i][j]->norm_map);
	    attroff(COLOR_PAIR(color));
	  }
	} else {
	  if(s) {
	    attron(COLOR_PAIR(color + 1));
	  } else {
	    attron(COLOR_PAIR(color));
	  }
	  if(a) {
	    mvaddch(i + 1, j, get_icon(a));
	  } else if(o) {
	    mvaddch(i + 1, j, o->icon);
	  } else if(d) {
	    mvaddch(i + 1, j, dungeon[i][j]->detect_map);
	  } else {
	    mvaddch(i + 1, j, dungeon[i][j]->norm_map);
	  }
	  if(s) {
	    attron(COLOR_PAIR(color + 1));
	  } else {
	    attroff(COLOR_PAIR(color));
	  }
	}
      }
    }
  }
  refresh();
}

void destroy_dungeon(void) {
  int i, j;
  /* Free the cells */
  for(i = 0; i < D_HEIGHT; i++) {
    for(j = 0; j < D_WIDTH; j++) {
      dungeon[i][j]->actor = 0;
      dungeon[i][j]->obj = 0;
      destroy_cell(dungeon[i][j]);
    }
  }
  /* Free the rooms */
  for(i = 0; i < num_rooms; i++) {
    destroy_room(rooms[i]);
  }
}
