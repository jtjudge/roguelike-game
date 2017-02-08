#ifndef GAME_H
# define GAME_H

#include "actor.h"
#include "object.h"

#define MAX_MONS 50
#define MAX_OBJS 50
#define MAX_ROOMS 20

#define STD_MESSAGE "Press 'q' to quit, 'm' for monster list, 'i' for inventory, 'e' for equipped."

extern int num_mons, num_objs;
extern pc *p;
extern monster **mons;
extern object **objs;

void set_up_game(int x, int y, int spawn_mons, int spawn_objs,
		 int test_room, int fog);
void update_fog_of_war(void);
void do_moves(int smooth, int sight);
void tear_down_game(void);
void write_to_header(const char *s);
void clear_header(void);
void write_end_message(void);
void display_pc_stats(void);

#endif
