#ifndef EVENT_H
#define EVENT_H

#include "actor.h"

class event {
 public:
  int next_turn, priority, repeat;
  event() {}
  virtual ~event() {};
  virtual int execute(void) = 0;
  virtual event *copy(void) = 0;
};

class actor_move : public event {
 public:
  actor *target;
  int (*action)(actor *a);
  actor_move() {};
  ~actor_move() {};
  int execute(void);
  event *copy(void);
};

class bomb_explode : public event {
 public:
  object *target;
  int (*action)(object *o);
  bomb_explode() {};
  ~bomb_explode() {};
  int execute(void);
  event *copy(void);
};

int cmp_event(void *e1, void *e2);
void disp_event(void *e);

#endif
