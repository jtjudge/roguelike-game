#include <stdio.h>

#include "heap.h"
#include "dungeon.h"

int cmp_cell(void *c1, void *c2) {
  return ((cell *)c1)->dist - ((cell *)c2)->dist;
}

int t_cmp_cell(void *c1, void *c2) {
  return ((cell *)c1)->t_dist - ((cell *)c2)->t_dist;
}

void disp_cell(void *c) {
  printf("%c", ((cell *)c)->norm_map);
}

void dijkstra(cell *target, int tunnels) {
  heap *h = new heap();
  if(tunnels) {
    h->create(t_cmp_cell, disp_cell);
  } else {
    h->create(cmp_cell, disp_cell);
  }
  /* Update dungeon data for generating nav map with tunnels */
  int i, j;
  cell *c;
  for(i = 0; i < D_HEIGHT; i++) {
    for(j = 0; j < D_WIDTH; j++) {
      c = dungeon[i][j];
      /* All distances start out as infinity */
      if(tunnels) {
	c->t_dist = INF;
      } else {
	c->dist = INF;
      }
      /* All nodes start as unvisited */
      c->visited = 0;
      /* Assign weights according to hardness */
      unsigned char ch = c->hardness;
      if(ch == 0 || ch < 85) {
	c->weight = 1;
      } else if(ch > 84 && ch < 171) {
	c->weight = 2;
      } else if(ch > 170 && ch < 255) {
	c->weight = 3;
      } else {
	/* Borders */
	c->weight = 4;
      }
      if(c == target) {
	if(tunnels) {
	  c->t_dist = 0;
	} else {
	  c->dist = 0;
	}
	c->weight = 0;
	/* Origin is first on the queue */
        h->insert((void *)c);
      }
    }
  }
  cell *u, *v;
  while(h->size > 0) {
    u = (cell *)h->remove_min();
    u->visited = 1;
    int i, j, x = u->x, y = u->y;
    for(i = -1; i < 2; i++) {
      if(y + i > 0 && y + i < 21) {
	for(j = -1; j < 2; j++) {
	  if(x + j > 0 && x + j < 80) {
	    v = dungeon[y + i][x + j];
	    if(v->visited != 1 && v->weight < 4 &&
	       (tunnels || v->hardness == 0)) {
	      int alt = tunnels ? u->t_dist + v->weight :
		u->dist + v->weight;
	      if((tunnels && alt < v->t_dist) ||
		 (!tunnels && alt < v->dist)) {
		if(tunnels) v->t_dist = alt; else v->dist = alt;
		h->insert((void *)v);
	      }
	    }
	  }
	}
      }
    }
  }
  h->destroy();
  delete h;
}
