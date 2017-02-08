#include <ncurses.h>

#include "event.h"
#include "dungeon.h"

int actor_move::execute(void) {
  if(!this->target->alive) {
    this->repeat = 0;
    return 0;
  }
  this->next_turn += 100 / this->target->speed;
  return this->action(this->target);
}

event *actor_move::copy(void) {
  actor_move *e = new actor_move();
  e->next_turn = this->next_turn;
  e->priority = this->priority;
  e->action = this->action;
  e->target = this->target;
  e->repeat = this->repeat;
  return e;
}

int bomb_explode::execute(void) {
  int x = this->target->x, y = this->target->y;
  dungeon[y][x]->obj = 0;
  this->repeat = 0;
  return this->action(this->target);
}

event *bomb_explode::copy(void) {
  bomb_explode *e = new bomb_explode();
  e->next_turn = this->next_turn;
  e->priority = this->priority;
  e->target = this->target;
  e->repeat = this->repeat;
  e->action = this->action;
  return e;
}

int cmp_event(void *e1, void *e2) {
  event *e1e = (event *)e1, *e2e = (event *)e2;
  int n1 = e1e->next_turn, n2 = e2e->next_turn;
  if(n1 == n2) {
    return e1e->priority - e2e->priority;
  } else {
    return n1 - n2;
  }
}

void disp_event(void *a) {
  event *e = (event *)a;
  if(e) {
    printw("%d %d", e->next_turn, e->repeat);
  } else {
    printw("HEAP ERROR");
  }
}
