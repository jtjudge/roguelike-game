#include <cstdlib>
#include <cstring>

#include "object.h"
#include "dungeon.h"
#include "parse.h"

object *create_object(obj_desc *d) {
  object *o = new object();
  int i, rx, ry;
  while(1) {
    rx = rand() % D_WIDTH;
    ry = rand() % D_HEIGHT;
    /* Place objects in any non-rock cell */
    if(dungeon[ry][rx]->norm_map != D_ROCK &&
       !(dungeon[ry][rx]->obj)) {
      o->x = rx;
      o->y = ry;
      dungeon[ry][rx]->obj = o;
      break;
    }
  }
  o->icon = d->type;
  o->color = d->colors[0] * 2;
  o->weight = d->w_base;
  for(i = 0; i < d->w_numdice; i++) {
    o->weight += 1 + (rand() % d->w_sides);
  }
  o->hit = d->h_base;
  for(i = 0; i < d->h_numdice; i++) {
    o->hit += 1 + (rand() % d->h_sides);
  }
  o->attr = d->a_base;
  for(i = 0; i < d->a_numdice; i++) {
    o->attr += 1 + (rand() % d->a_sides);
  }
  o->val = d->v_base;
  for(i = 0; i < d->v_numdice; i++) {
    o->val += 1 + (rand() % d->v_sides);
  }
  o->dodge = d->do_base;
  for(i = 0; i < d->do_numdice; i++) {
    o->dodge += 1 + (rand() % d->do_sides);
  }
  o->def = d->de_base;
  for(i = 0; i < d->de_numdice; i++) {
    o->def += 1 + (rand() % d->de_sides);
  }
  o->speed = d->s_base;
  for(i = 0; i < d->s_numdice; i++) {
    o->speed += 1 + (rand() % d->s_sides);
  }
  o->dam_base = d->da_base;
  o->dam_dice = d->da_numdice;
  o->dam_sides = d->da_sides;
  o->inv = o->live_bomb = 0;
  o->area_of_effect = DEF_RAD;
  o->desc = d;
  return o;
}

void destroy_object(object *o) {
  delete o;
}
