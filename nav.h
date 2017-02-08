#ifndef NAV_H
# define NAV_H

#include "dungeon.h"
#include "heap.h"

class actor;
char get_icon(actor *a);

void generate_nav_maps(int x, int y);
void display_nav_maps(int tunnels);

#endif
