#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>

#include "nav.h"
#include "dungeon.h"
#include "dijkstra.h"

void generate_nav_maps(int x, int y) {
  /* Dijkstra no tunnels */
  dijkstra(dungeon[y][x], 0);
  /* Record data to no tunnels nav map */
  int i, j;
  for(i = 0; i < D_HEIGHT; i++) {
    for(j = 0; j < D_WIDTH; j++) {
      int dist = dungeon[i][j]->dist;
      char ch;
      if(dist >= 0 && dist < 10) {
	ch = dist + 48;
      } else if(dist >= 10 && dist < 36) {
	ch = dist + 87;
      } else if(dist >= 36 && dist < 62) {
	ch = dist + 29;
      } else {
	ch = dungeon[i][j]->norm_map;
      }
      dungeon[i][j]->nav_map = ch;
    }
  }
  /* Dijkstra with tunnels */
  dijkstra(dungeon[y][x], 1);
  /* Record data to tunneler's nav map */
  for(i = 0; i < D_HEIGHT; i++) {
    for(j = 0; j < D_WIDTH; j++) {
      int t_dist = dungeon[i][j]->t_dist;
      char ch;
      if(t_dist >= 0 && t_dist < 10) {
	ch = t_dist + 48;
      } else if(t_dist >= 10 && t_dist < 36) {
	ch = t_dist + 87;
      } else if(t_dist >= 36 && t_dist < 62) {
	ch = t_dist + 29;
      } else {
	ch = dungeon[i][j]->norm_map;
      }
      dungeon[i][j]->t_nav_map = ch;
    }
  }
}

void display_nav_maps(int tunnels) {
  attron(COLOR_PAIR(WB));
  int i, j;
  for(i = 0; i < D_HEIGHT; i++) {
    for(j = 0; j < D_WIDTH; j++) {
      cell *c = dungeon[i][j];
      if(c->actor) {
	attron(COLOR_PAIR(BAW));
	mvaddch(i + D_HEIGHT + 2, j, get_icon(dungeon[i][j]->actor));
	attroff(COLOR_PAIR(BAW));
      } else if(tunnels) {
	mvaddch(i + D_HEIGHT + 2, j, dungeon[i][j]->t_nav_map);
      } else {
	mvaddch(i + D_HEIGHT + 2, j, dungeon[i][j]->nav_map);
      }
    }
  }
  attroff(COLOR_PAIR(WB));
  refresh();
}
