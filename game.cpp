#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ncurses.h>
#include <cstring>

#include "game.h"
#include "actor.h"
#include "event.h"
#include "heap.h"
#include "nav.h"
#include "dungeon.h"
#include "bres.h"
#include "dijkstra.h"
#include "parse.h"
#include "object.h"

int num_mons;
int num_objs;
pc *p;
monster **mons;
object **objs;
int cleared = 0;
int fog_of_war;

void place_pc(int x, int y) {
  if(x == -1) {
    /* Randomly assign spawn to a room */
    while(1) {
      int rx = rand() % D_WIDTH;
      int ry = rand() % D_HEIGHT;
      if(dungeon[ry][rx]->norm_map == D_FLOOR) {
	p->x = rx;
        p->y = ry;
	p->target_x = rx;
	p->target_y = ry;
	dungeon[ry][rx]->actor = p;
	break;
      }
    }
  } else {
  /* If x and y are specified */
    p->x = x;
    p->y = y;
    dungeon[y][x]->actor = p;
  }
}

void spawn_monsters(void) {
  int px = p->x, py = p->y, i;
  for(i = 0; i < num_mons; i++) {
    if(num_mon_desc) {
      mons[i] = create_monster(m_descs[rand() % num_mon_desc], px, py);
    } else {
      mons[i] = create_monster(rand() % 16, px, py);
    }
  }
}

void spawn_objects(void) {
  if(num_obj_desc) {
    int i, count = 0;
    for(i = 0; i < num_objs + count; i++) {
      if(!objs[i]) {
	objs[i] = create_object(o_descs[rand() % num_obj_desc]);
      } else {
	count++;
      }
    }
  } else {
    num_objs = 0;
  }
}

int is_blocked(cell *c) {
  return c->blocked;
}

int cell_in_sight_rad(actor *a, cell *c) {
  int r = a->light_rad, dx = c->x - a->x, dy = c->y - a->y;
  if(!(abs(dx) < r && abs(dy) < r)) return 0;
  cell *n = get_cell_between(a->x, a->y,
			     a->target_x, a->target_y);
  if(n->x < a->x && n->y < a->y) {
    /* TOP LEFT */
    return dx <= 0 && dx >= r*-1 && dy <= 0 && dy >= r*-1;
  } else if(n->x == a->x && n->y < a->y) {
    /* TOP */
    return abs(dx) <= r/2 && dy <= 0 && dy >= r*-1;
  } else if(n->x > a->x && n->y < a->y) {
    /* TOP RIGHT */
    return dx >= 0 && dx <= r && dy <= 0 && dy >= r*-1;
  } else if(n->x < a->x && n->y == a->y) {
    /* LEFT */
    return dx <= 0 && dx >= r*-1 && abs(dy) <= r/2;
  } else if(n->x == a->x && n->y == a->y) {
    /* CENTER */
    return abs(dx) <= (r+1)/2 && abs(dy) <= (r+1)/2;
  } else if(n->x > a->x && n->y == a->y) {
    /* RIGHT */
    return dx >= 0 && dx <= r && abs(dy) <= r/2;
  } else if(n->x < a->x && n->y > a->y) {
    /* BOTTOM LEFT */
    return dx <= 0 && dx >= r*-1 && dy >= 0 && dy <= r;
  } else if(n->x == a->x && n->y > a->y) {
    /* BOTTOM */
    return abs(dx) <= r/2 && dy >= 0 && dy <= r;
  } else if(n->x > a->x && n->y > a->y) {
    /* BOTTOM RIGHT */
    return dx >= 0 && dx <= r && dy >= 0 && dy <= r;
  }
  return 0;
}

int cell_in_sight_line(actor *a, cell *c) {
  return !bres_cells(a->x, a->y, c->x, c->y, is_blocked);
}

int detect_pc(monster *m) {
  if(m->psychic || ((!p->sneaking || m->detecting ||
		     cell_in_sight_rad(m, dungeon[p->y][p->x]))
		    && cell_in_sight_line(m, dungeon[p->y][p->x]))) {
    m->target_x = p->x;
    m->target_y = p->y;
    m->detecting = 1;
    m->searching = 1;
    return 1;
  } else {
    m->detecting = 0;
    if(!(m->smart || m->guard) || (m->x == m->target_x &&
				   m->y == m->target_y)) {
      m->searching = 0;
    }
    return 0;
  }
}

void update_detect_map(void) {
  int i, j;
  for(i = 0; i < D_HEIGHT; i++) {
    for(j = 0; j < D_WIDTH; j++) {
      dungeon[i][j]->detect_map = 0;
    }
  }
  if(p->alive) {
    for(i = 0; i < num_mons; i++) {
      if(mons[i]->alive && mons[i]->y > 0) {
	if(mons[i]->searching) {
	  dungeon[mons[i]->y - 1][mons[i]->x]->detect_map = '?';
	}
	if(mons[i]->detecting) {
	  dungeon[mons[i]->y - 1][mons[i]->x]->detect_map = '!';
	}
      }
    }
  }
}

void update_fog_of_war(void) {
  int i, j, x = p->x, y = p->y,
    rad = p->light_rad, neg = rad * -1;
  for(i = 0; i < D_HEIGHT; i++) {
    for(j = 0; j < D_WIDTH; j++) {
      dungeon[i][j]->fog = 0;
    }
  }
  for(i = neg; i < rad + 1; i++) {
    for(j = neg; j < rad + 1; j++) {
      if(x + j > -1 && x + j < D_WIDTH &&
	 y + i > -1 && y + i < D_HEIGHT) {
	cell *c = dungeon[y + i][x + j];
	if(cell_in_sight_line(p, c) && cell_in_sight_rad(p, c)
	   && c->hardness == 0) {
	  c->fog = 1;
	  c->pc_view = c->norm_map;
	}
      }
    }
  }
}

void update_sight_areas(void) {
  int x, y, i, j, k, rad, neg;
  for(i = 0; i < D_HEIGHT; i++) {
    for(j = 0; j < D_WIDTH; j++) {
      cell *c = dungeon[i][j];
      c->seen = 0;
      c->blocked = c->hardness > 0 ? 1 : 0;
    }
  }
  for(i = 0; i < num_mons; i++) {
    monster *m = mons[i];
    if(m->alive) {
      x = m->x; y = m->y;
      rad = m->light_rad; neg = rad * -1;
      for(j = neg; j < rad + 1; j++) {
	for(k = neg; k < rad + 1; k++) {
	  if(x + k > -1 && x + k < D_WIDTH &&
	     y + j > -1 && y + j < D_HEIGHT) {
	    cell *c = dungeon[y + j][x + k];
	    if(cell_in_sight_line(mons[i], c) &&
	       cell_in_sight_rad(mons[i], c)) {
	      c->seen = 1;
	    }	   
	  }
	}
      }
    }
  }
}
  
void set_up_game(int x, int y, int spawn_mons, int spawn_objs,
		 int test_room, int fog) {
  int i;
  fog_of_war = fog;
  generate_dungeon(test_room);
  place_pc(x, y);
  generate_nav_maps(p->x, p->y);
  if(spawn_mons) {
    mons = (monster **)malloc(num_mons * sizeof(monster *));
    for(i = 0; i < num_mons; i++) {
      mons[i] = 0;
    }
    spawn_monsters();
  }
  if(spawn_objs) {
    objs = (object **)malloc((num_objs + CARRY_SIZE + EQUIP_SIZE)
			     * sizeof(object *));
    for(i = 0; i < num_objs; i++) {
      objs[i] = 0;
    }
    spawn_objects();
  }
}

void tear_down_game(void) {
  destroy_dungeon();
  int i;
  for(i = 0; i < num_mons; i++) {
    destroy_monster(mons[i]);
    mons[i] = 0;
  }
  for(i = 0; i < num_objs; i++) {
    destroy_object(objs[i]);
    objs[i] = 0;
  }
  free(mons);
  free(objs);
}

void change_level(void) {
  destroy_dungeon();
  int i;
  for(i = 0; i < num_mons; i++) {
    destroy_monster(mons[i]);
  }
  for(i = 0; i < num_objs; i++) {
    if(!objs[i]->inv) {
      destroy_object(objs[i]);
      objs[i] = 0;
    }
  }
  generate_dungeon(0);
  place_pc(-1, -1);
  spawn_monsters();
  spawn_objects();
  cleared = 0;
  p->visits++;
}

void clear_header(void) {
  int i;
  for(i = 0; i < D_WIDTH; i++) {
    mvaddch(0, i, ' ');
  }
}

void write_to_header(const char *s) {
  clear_header();
  mvprintw(0, 0, s);
}

void clear_footer(void) {
  int i;
  for(i = 0; i < D_WIDTH; i++) {
    mvaddch(22, i, ' ');
    mvaddch(23, i, ' ');
  }
}

void write_to_footer(const char *s1, const char *s2) {
  int i, j, l;
  for(i = D_WIDTH - 1; i > 32; i--) {
    mvaddch(22, i, ' ');
    mvaddch(23, i, ' ');
  }
  l = strlen(s1);
  for(i = D_WIDTH, j = 0; j <= l; i--, j++) {
    mvaddch(22, i, s1[l - j]);
  }
  l = strlen(s2);
  for(i = D_WIDTH, j = 0; j <= l; i--, j++) {
    mvaddch(23, i, s2[l - j]);
  }
}

void display_pc_stats(void) {
  int hp = p->hp, mhp = p->max_hp,
    num = (hp+((mhp/20)-1))/(mhp/20),
    max = (mhp+((mhp/20)-1))/(mhp/20),
    speed = p->speed, i;
  for(i = 0; i < max + 3; i++) {
    mvaddch(22, i, ' ');
    mvaddch(23, i, ' ');
  }
  mvprintw(22, 0, "HP");
  if(hp < mhp / 4) {
    attron(COLOR_PAIR(RR));
  } else if(hp < mhp / 2) {
    attron(COLOR_PAIR(YY));
  } else {
    attron(COLOR_PAIR(GG));
  }
  for(i = 0; i < num; i++) {
    mvaddch(22, i + 3, ' ');
  }
  if(hp < mhp / 4) {
    attroff(COLOR_PAIR(RR));
  } else if(hp < mhp / 2) {
    attroff(COLOR_PAIR(YY));
  } else {
    attroff(COLOR_PAIR(GG));
  }
  if(p->sneaking) {
    mvprintw(22, 3 + max, "--SNEAK--");
  } else {
    mvprintw(22, 3 + max, "         ");
  }
  mvprintw(23, 0, "SP");
  num = speed == 1 ? 0 : (speed + 4) / 5;
  attron(COLOR_PAIR(CC));
  for(i = 0; i < num; i++) {
    mvaddch(23, i + 3, ' ');
  }
  attroff(COLOR_PAIR(CC));
}

void write_end_message(void) {
  clear_header();
  mvprintw(0, 0,
    "RIP. %d / %d monsters slain. %d / %d floors cleared.",
	   p->kills, p->visits * num_mons, p->clears, p->visits);
  if(p->kills == 0) {
    mvprintw(0, 61, "Too bad! Q to quit.");
  } else if(p->kills < 1) {
    mvprintw(0, 60, "Good try! Q to quit.");
  } else if(p->kills < 5) {
    mvprintw(0, 50, "You're pretty good! Q to quit.");
  } else {
    mvprintw(0, 55, "You are a pro! Q to quit.");
  }
}

void display_monster_menu(void) {
  int i, j, offset = 0;
  attron(COLOR_PAIR(WB));
  write_to_header("ESC to exit.");
  attroff(COLOR_PAIR(WB));
  clear_footer();
  while(1) {
    for(i = 0; i < num_mons && i < 21; i++) {
      for(j = 0; j < 25; j++) {
	mvaddch(i + 1, j, ' ');
      }
    }
    for(i = 0; i < num_mons && i < 21; i++) {
      char ch = mons[i + offset]->icon;
      int dx = p->x - mons[i + offset]->x;
      int dy = p->y - mons[i + offset]->y;
      if(!mons[i + offset]->alive) {
	attron(COLOR_PAIR(RB));
      } else {
	attron(COLOR_PAIR(WB));
      }
      if(dx > 0 && dy > 0) {
	mvprintw(i + 1, 0, "[%c] %d north and %d west", ch, dy, dx);
      } else if(dx < 0 && dy > 0) {
	mvprintw(i + 1, 0, "[%c] %d north and %d east", ch, dy, dx * -1);
      } else if(dx > 0 && dy < 0) {
	mvprintw(i + 1, 0, "[%c] %d south and %d west", ch, dy * -1, dx);
      } else if(dx < 0 && dy < 0) {
	mvprintw(i + 1, 0, "[%c] %d south and %d east", ch, dy * -1, dx * -1);
      } else if(dx == 0 && dy > 0) {
	mvprintw(i + 1, 0, "[%c] %d north", ch, dy);
      } else if(dx == 0 && dy < 0) {
	mvprintw(i + 1, 0, "[%c] %d south", ch, dy * -1);
      } else if(dx > 0 && dy == 0) {
	mvprintw(i + 1, 0, "[%c] %d west", ch, dx);
      } else if(dx < 0 && dy == 0) {
	mvprintw(i + 1, 0, "[%c] %d east", ch, dx * -1);
      } else {
	mvprintw(i + 1, 0, "[%c] at your location", ch);
      }
      if(!mons[i + offset]->alive) {
	attroff(COLOR_PAIR(RB));
      } else {
	attroff(COLOR_PAIR(WB));
      }
    }
    attron(COLOR_PAIR(WB));
    mvprintw(0, 66, "Kills: %d / %d", p->kills,
	     p->visits * num_mons);
    mvprintw(1, 66, "Clears: %d / %d",
	     p->clears, p->visits);
    attroff(COLOR_PAIR(WB));
    int c = getch();
    while(c != 033 && c != 0403 && c != 0402) {
      c = getch();
    }
    if(c == 0403 && offset > 0) offset--;
    if(c == 0402 && offset < num_mons - 21) offset++;
    if(c == 033) break;
  }
  attron(COLOR_PAIR(WB));
  write_to_header(STD_MESSAGE);
  attroff(COLOR_PAIR(WB));
  if(cleared) {
    display_dungeon(GB, fog_of_war, p->sneaking);
  } else {
    display_dungeon(WB, fog_of_war, p->sneaking);
  }
  display_pc_stats();
}

void display_pc_carry(void) {
  int i, j, offset = 0;
  attron(COLOR_PAIR(WB));
  write_to_header("ESC to exit.");
  attroff(COLOR_PAIR(WB));
  clear_footer();
  while(1) {
    for(i = 0; i < CARRY_SIZE && i < 21; i++) {
      for(j = 0; j < 25; j++) {
	mvaddch(i + 1, j, ' ');
      }
    }
    for(i = 0; i < CARRY_SIZE && i < 21; i++) {
      object *o = p->inventory[i + offset];
      if(o) {
	char icon = o->icon;
	char *name = o->desc->name;
	int color = o->color;
	mvprintw(i + 1, 0, "[%d] ", i);
	attron(COLOR_PAIR(color));
	mvprintw(i + 1, 4, "%c ", icon);
	attroff(COLOR_PAIR(color));
	mvprintw(i + 1, 6, "%s", name);
      } else {
	mvprintw(i + 1, 0, "[%d] EMPTY", i);
      }
    }
    attron(COLOR_PAIR(WB));
    mvprintw(0, 66, "Kills: %d / %d", p->kills,
	     p->visits * num_mons);
    mvprintw(1, 66, "Clears: %d / %d",
	     p->clears, p->visits);
    attroff(COLOR_PAIR(WB));
    int c = getch();
    while(c != 033 && c != 0403 && c != 0402) {
      c = getch();
    }
    if(c == 0403 && offset > 0) offset--;
    if(c == 0402 && offset < CARRY_SIZE - 21) offset++;
    if(c == 033) break;
  }
  attron(COLOR_PAIR(WB));
  write_to_header(STD_MESSAGE);
  attroff(COLOR_PAIR(WB));
  if(cleared) {
    display_dungeon(GB, fog_of_war, p->sneaking);
  } else {
    display_dungeon(WB, fog_of_war, p->sneaking);
  }
  display_pc_stats();
}

void display_pc_equipped(void) {
  int i, j, offset = 0;
  attron(COLOR_PAIR(WB));
  write_to_header("ESC to exit.");
  attroff(COLOR_PAIR(WB));
  clear_footer();
  while(1) {
    for(i = 0; i < EQUIP_SIZE && i < 21; i++) {
      for(j = 0; j < 25; j++) {
	mvaddch(i + 1, j, ' ');
      }
    }
    for(i = 0; i < EQUIP_SIZE && i < 21; i++) {
      object *o = p->inventory[i + CARRY_SIZE + offset];
      if(o) {
	char icon = o->icon;
	char *name = o->desc->name;
	int color = o->color;
	mvprintw(i + 1, 0, "[%c] ", i + 97);
	attron(COLOR_PAIR(color));
	mvprintw(i + 1, 4, "%c ", icon);
	attroff(COLOR_PAIR(color));
	mvprintw(i + 1, 6, " %s", name);
      } else {
	mvprintw(i + 1, 0, "[%c] EMPTY", i + 97);
      }
    }
    attron(COLOR_PAIR(WB));
    mvprintw(0, 66, "Kills: %d / %d", p->kills,
	     p->visits * num_mons);
    mvprintw(1, 66, "Clears: %d / %d",
	     p->clears, p->visits);
    attroff(COLOR_PAIR(WB));
    int c = getch();
    while(c != 033 && c != 0403 && c != 0402) {
      c = getch();
    }
    if(c == 0403 && offset > 0) offset--;
    if(c == 0402 && offset < EQUIP_SIZE - 21) offset++;
    if(c == 033) break;
  }
  attron(COLOR_PAIR(WB));
  write_to_header(STD_MESSAGE);
  attroff(COLOR_PAIR(WB));
  if(cleared) {
    display_dungeon(GB, fog_of_war, p->sneaking);
  } else {
    display_dungeon(WB, fog_of_war, p->sneaking);
  }
  display_pc_stats();
}

void display_pc_item(object *o) {
  int i, j, offset = 0;
  attron(COLOR_PAIR(WB));
  write_to_header("");
  mvprintw(0, 0, "Inspecting %s", o->desc->name);
  mvprintw(0, 68, "ESC to exit.");
  mvprintw(1, 0, "DAM: %d+%dd%d   SPEED: %d", o->dam_base,
	   o->dam_dice, o->dam_sides, o->speed);
  attroff(COLOR_PAIR(WB));
  clear_footer();
  while(1) {
    for(i = 0; i < o->desc->desc_lines && i < 18; i++) {
      for(j = 0; j < 80; j++) {
	mvaddch(i + 3, j, ' ');
      }
    }
    for(i = 0; i < o->desc->desc_lines && i < 18; i++) {
      char *s = o->desc->desc[i + offset];
      mvprintw(i + 3, 0, "%s", s);
    }
    int c = getch();
    while(c != 033 && c != 0403 && c != 0402) {
      c = getch();
    }
    if(c == 0403 && offset > 0) offset--;
    if(c == 0402 && offset < o->desc->desc_lines - 18) offset++;
    if(c == 033) break;
  }
  attron(COLOR_PAIR(WB));
  write_to_header(STD_MESSAGE);
  attroff(COLOR_PAIR(WB));
  if(cleared) {
    display_dungeon(GB, fog_of_war, p->sneaking);
  } else {
    display_dungeon(WB, fog_of_war, p->sneaking);
  }
  display_pc_stats();
}

int pc_move(actor *a) {
  if(!a->alive) return 0;
  pc *p = (pc *)a;
  int valid = 0, reset = 0,
    quit = 0, bomb = 0, x, y;
  while(!valid) {
    x = p->x; y = p->y;
    int key = getch(), moved = 0, i;
    for(i = 32; i < D_WIDTH; i++) {
      mvaddch(22, i, ' ');
      mvaddch(23, i, ' ');
    }
    switch(key)
      {
      case 0406: /* TOP LEFT */
	y--;
	x--;
	moved = 1;
	break;
      case 0171:
	y--;
	x--;
	moved = 1;
	break;
      case 0403: /* UP */
	y--;
	moved = 1;
	break;
      case 0153:
	y--;
	moved = 1;
	break;
      case 0523: /* TOP RIGHT */
	y--;
	x++;
	moved = 1;
	break;
      case 0165:
	y--;
	x++;
	moved = 1;
	break;
      case 0404: /* LEFT */
	x--;
	moved = 1;
	break;
      case 0150:
	x--;
	moved = 1;
	break;
      case 040: /* CENTER */
	moved = 1;
	break;
      case 0324:
	moved = 1;
	break;
      case 0405: /* RIGHT */
	x++;
	moved = 1;
	break;
      case 0154:
	x++;
	moved = 1;
	break;
      case 0550: /* BOTTOM LEFT */
	x--;
	y++;
	moved = 1;
	break;
      case 0142:
	x--;
	y++;
	moved = 1;
	break;
      case 0402: /* DOWN */
	y++;
	moved = 1;
	break;
      case 0152:
	y++;
	moved = 1;
	break;
      case 0522: /* BOTTOM RIGHT */
	x++;
	y++;
	moved = 1;
	break;
      case 0156:
	x++;
	y++;
	moved = 1;
	break;
      case 074:  /* UP STAIRS */
	if(dungeon[y][x]->norm_map == D_UP_STAIRS) {
	  reset = 1;
	  valid = 1;
	}
	break;
      case 076:  /* DOWN STAIRS */
	if(dungeon[y][x]->norm_map == D_DOWN_STAIRS) {
	  reset = 1;
	  valid = 1;
	}
	break;
      case 0163:  /* s -- SNEAK */
	{
	  p->sneaking = p->sneaking ? 0 : 1;
	  if(cleared) {
	    display_dungeon(GB, fog_of_war, p->sneaking);
	  } else {
	    display_dungeon(WB, fog_of_war, p->sneaking);
	  }
	  display_pc_stats();
	  break;
	}
      case 0162: /* r -- RANGED ATTACK */
	{
	  object *o = p->get_obj(get_slot_for_type(RANGED));
	  if(!o) {
	    write_to_footer("[RANGED] No weapon equipped in slot 'c'.", "");
	    break;
	  }
	  int curs_x = p->x, curs_y = p->y;
	  dungeon[curs_y][curs_x]->cursor = 1;
	  write_to_footer("[RANGED] Select target: 'r' to confirm",
			  "ESC to cancel");
	  int k = getch();
	  while(k != 033 && k != 0162) {
	    if(cleared) {
	      display_dungeon(GB, fog_of_war, 1);
	    } else {
	      display_dungeon(WB, fog_of_war, 1);
	    }
	    dungeon[curs_y][curs_x]->cursor = 0;
	    k = getch();
	    switch(k)
	      {
	      case 0406: /* TOP LEFT */
		if(curs_y == 0 || curs_x == 0) break;
		curs_y--;
		curs_x--;
		break;
	      case 0171:
		if(curs_y == 0 || curs_x == 0) break;
		curs_y--;
		curs_x--;
		break;
	      case 0403: /* UP */
		if(curs_y == 0) break;
		curs_y--;
		break;
	      case 0153:
		if(curs_y == 0) break;
		curs_y--;
		break;
	      case 0523: /* TOP RIGHT */
		if(curs_y == 0 || curs_x == 79) break;
		curs_y--;
		curs_x++;
		break;
	      case 0165:
		if(curs_y == 0 || curs_x == 79) break;
		curs_y--;
		curs_x++;
		break;
	      case 0404: /* LEFT */
		if(curs_x == 0) break;
		curs_x--;
		break;
	      case 0150:
		if(curs_x == 0) break;
		curs_x--;
		break;
	      case 040: /* CENTER */
		break;
	      case 0324:
		break;
	      case 0405: /* RIGHT */
		if(curs_x == 79) break;
		curs_x++;
		break;
	      case 0154:
		if(curs_x == 79) break;
		curs_x++;
		break;
	      case 0550: /* BOTTOM LEFT */
		if(curs_x == 0 || curs_y == 20) break;
		curs_x--;
		curs_y++;
		break;
	      case 0142:
		if(curs_x == 0 || curs_y == 20) break;
		curs_x--;
		curs_y++;
		break;
	      case 0402: /* DOWN */
		if(curs_y == 20) break;
		curs_y++;
		break;
	      case 0152:
		if(curs_y == 20) break;
		curs_y++;
		break;
	      case 0522: /* BOTTOM RIGHT */
		if(curs_x == 79 || curs_y == 20) break;
		curs_x++;
		curs_y++;
		break;
	      case 0156:
		if(curs_x == 79 || curs_y == 20) break;
		curs_x++;
		curs_y++;
		break;
	      default:
		break;
	      }
	    if(!bres_cells(p->x, p->y, curs_x, curs_y, is_blocked)) {
	       dungeon[curs_y][curs_x]->cursor = 1;
	    } else {
	      dungeon[curs_y][curs_x]->cursor = 2;
	    }
	  }
	  if(k == 033) {
	    write_to_footer("[RANGED] Cancelled.", "");
	    dungeon[curs_y][curs_x]->cursor = 0;
	    if(cleared) {
	      display_dungeon(GB, fog_of_war, 1);
	    } else {
	      display_dungeon(WB, fog_of_war, 1);
	    }
	    break;
	  } else if(dungeon[curs_y][curs_x]->cursor == 2) {
	    write_to_footer("[RANGED] Invalid target. Cancelled.", "");
	    dungeon[curs_y][curs_x]->cursor = 0;
	    if(cleared) {
	      display_dungeon(GB, fog_of_war, 1);
	    } else {
	      display_dungeon(WB, fog_of_war, 1);
	    }
	    break;
	  } else {
	    actor *a = dungeon[curs_y][curs_x]->actor;
	    if(a) {
	      ranged_attack(a, p, p->get_obj(get_slot_for_type(RANGED)), 1);
	    } else {
	      write_to_footer("[RANGED] Shot at nothing.", "");
	    }
	    moved = 1;
	    valid = 1;
	    dungeon[curs_y][curs_x]->cursor = 0;
	    break;
	  }
	}
      case 0161: /* q -- QUIT */
	quit = 1;
	valid = 1;
	break;
      case 0155: /* m -- MONSTER MENU */
	display_monster_menu();
	break;
      case 0151: /* i -- CARRY DISPLAY */
	display_pc_carry();
	break;
      case 0145: /* e -- EQUIPPED DISPLAY */
	display_pc_equipped();
	break;
      case 0167: /* w -- WEAR */
	{
	  int equipped = 0, s = -1;
	  write_to_footer("[WEAR] Slot? (0 - 9):", "ESC to cancel");
	  while(s != 033 - 48 && !equipped) {
	    s = getch() - 48;
	    object *o = p->get_obj(s);
	    int err = p->equip(s);
	    if(err == -1) {
	      write_to_footer("[WEAR] Please type a number, 0 - 9:",
			      "ESC to cancel");
	    } else if(err == -2) {
	      write_to_footer("[WEAR] Nothing in that slot, try again:",
			      "ESC to cancel");
	    } else if(err == -3) {
	      write_to_footer("[WEAR] Cannot equip that object, try again:",
			      "ESC to cancel");
	    } else if(err == 0) {
	      char buff[80];
	      snprintf(buff, sizeof(buff),
		       "[WEAR] %s equipped in empty slot %c.",
			   o->desc->name, get_slot_for_type(o->icon) + 87);
	      write_to_footer(buff, "");
	      equipped = 1;
	    } else {
	      char buff[80];
	      snprintf(buff, sizeof(buff),
		       "[WEAR] %s swapped into slot %c.", o->desc->name,
			   get_slot_for_type(o->icon) + 87);
	      write_to_footer(buff, "");
	      equipped = 1;
	    }
	  }
	  if(!equipped) {
	    write_to_footer("[WEAR] Cancelled.", "");
	  }
	  break;
	}
      case 0164: /* t -- TAKE OFF */
	{
	  int unequipped = 0, s = -1;
	  write_to_footer("[TAKE OFF] Slot? (a - l):", "ESC to cancel");
	  while(s != 033 - 87 && !unequipped) {
	    s = getch() - 87;
	    int err = p->unequip(s);
	    if(err == -1) {
	      write_to_footer("[TAKE OFF] Please type a letter, a - l",
			      "ESC to cancel");
	    } else if(err == -2) {
	      write_to_footer("[TAKE OFF] Nothing in that slot, try again:",
			      "ESC to cancel");
	    } else if(err == -3) {
	      write_to_footer("[TAKE OFF] No room, please drop something.", "");
	      unequipped = 1;
	    } else {
	      write_to_footer("[TAKE OFF] Took off item.", "");
	      unequipped = 1;
	    }
	  }
	  if(!unequipped) {
	    write_to_footer("[TAKE OFF] Cancelled.", "");
	  }
	  break;
	}
      case 0144: /* d -- DROP */
	{
	  int dropped = 0, s = -1;
	  write_to_footer("[DROP] Slot? (0 - 9):", "ESC to cancel");
	  while(s != 033 - 48 && !dropped) {
	    s = getch() - 48;
	    int err = p->drop(s);
	    if(err == -1) {
	      write_to_footer("[DROP] Please type a number, 0 - 9",
			      "ESC to cancel");
	    } else if(err == -2) {
	      write_to_footer("[DROP] Nothing in that slot, try again:",
			      "ESC to cancel");
	    } else if(err == -3) {
	      write_to_footer("[DROP] Location obstructed.", "");
	      dropped = 1;
	    } else if(err == 1) {
	      /* Drop bomb */
	      object *b = dungeon[p->y][p->x]->obj;
	      char buff[80];
	      snprintf(buff, sizeof(buff),
		       "[DROP] Dropping %s. Set delay at __? (1 - 9)",
		       b->desc->name);
	      write_to_footer(buff, "");
	      int delay = getch() - 48;
	      while(delay < 1 || delay > 9) {
		write_to_footer("Invalid value. Set delay at __? (1 - 9)", "");
		delay = getch() - 48;
	      }
	      snprintf(buff, sizeof(buff),
		       "[DROP] Dropped bomb with delay of %d turns.",
		       delay);
	      write_to_footer(buff, "");
	      dropped = 1;
	      moved = 1;
	      bomb = delay;
	      b->live_bomb = 1;
	    } else {
	      write_to_footer("[DROP] Dropped item.", "");
	      dropped = 1;
	    }
	  }
	  if(!dropped) {
	    write_to_footer("[DROP] Cancelled.", "");
	  }
	  break;
	}
      case 0170: /* x -- EXPUNGE */
	{
	  int destroyed = 0, s = -1;
	  write_to_footer("[EXPUNGE] Slot? (0 - 9):", "ESC to cancel");
	  while(s != 033 - 48 && !destroyed) {
	    s = getch() - 48;
	    int err = p->expunge(s);
	    if(err == -1) {
	      write_to_footer("[EXPUNGE] Please type a number, 0 - 9:",
			      "ESC to cancel");
	    } else if(err == -2) {
	      write_to_footer("[EXPUNGE] Nothing in that slot, try again:",
			      "ESC to cancel");
	    } else {
	      write_to_footer("[EXPUNGE] Item expunged.", "");
	      destroyed = 1;
	    }
	  }
	  if(!destroyed) {
	    write_to_footer("[EXPUNGE] Cancelled.", "");
	  }
	  break;
	}
      case 0111: /* I -- INSPECT */
	{
	  int inspected = 0, s = -1;
	  write_to_footer("[INSPECT] Slot? (0 - 9):", "ESC to cancel");
	  while(s != 033 - 48 && !inspected) {
	    s = getch() - 48;
	    object *o = p->get_obj(s);
	    if(!o) {
	      write_to_footer("[INSPECT] Invalid slot, try again:",
			      "ESC to cancel");
	    } else {
	      write_to_footer("", "");
	      display_pc_item(o);
	      inspected = 1;
	    }
	  }
	  if(!inspected) {
	    write_to_footer("[INSPECT] Cancelled.", "");
	  }
	  break;
	}
      default:
	break;
      }
    if(moved && x < 80 && x > -1 && y < 21 && y > -1
	      && dungeon[y][x]->norm_map != D_ROCK) {
      valid = 1;
    }
  }
  if(!reset && !quit) {
    if(dungeon[y][x]->actor && dungeon[y][x]->actor != p) {
      if(attack_actor(dungeon[y][x]->actor, dungeon[y][x],
		      p, dungeon[p->y][p->x], 1)) {
	p->kills++;
      }
    } else {
      move_actor(p, dungeon[p->y][p->x], dungeon[y][x]);
      object *o = dungeon[y][x]->obj;
      if(o && !p->pickup(o)) {
	char buff[80];
	snprintf(buff, sizeof(buff), "Picked up %s",
		 o->desc->name);
        write_to_footer(buff, "");
      }
    }
  }
  p->target_x = p->x;
  p->target_y = p->y;
  if(quit) return -1;
  if(reset) return -2;
  if(bomb) return bomb;
  return 0;
}

int smart_move(monster *m) {
  int i, j, x = m->x, y = m->y;
  generate_nav_maps(m->target_x, m->target_y);
  cell *mins[8]; int m_count = 0;
  cell *min = dungeon[y][x];
  cell *c = 0;
  if(m->tunneler) {
    for(i = -1; i < 2; i++) {
      for(j = -1; j < 2; j++) {
	c = dungeon[y + i][x + j];
	if(c->t_dist < min->t_dist) {
	  min = c;
	  m_count = 0;
	  mins[m_count++] = c;
	} else if(c->dist == min->dist) {
	  mins[m_count++] = c;
	}
      }
    }
  } else {
    for(i = -1; i < 2; i++) {
      for(j = -1; j < 2; j++) {
	c = dungeon[y + i][x + j];
	if(c->dist < min->dist) {
	  min = c;
	  m_count = 0;
	  mins[m_count++] = c;
	} else if(c->dist == min->dist) {
	  mins[m_count++] = c;
	}
      }
    }
  }
  if(m_count > 1) {
    min = mins[rand() % m_count];
  }
  if(min->hardness > 0) {
    if(m->tunneler) {
      int h = min->hardness;
      h -= 85;
      if(h <= 0) {
	min->hardness = 0;
	min->norm_map = D_CORR;
	move_actor(m, dungeon[y][x], min);
	return 0;
      } else {
	min->hardness -= 85;
	return 0;
      }
    } else {
      return 0;
    }
  }
  if(min->actor == p) {
    attack_actor(min->actor, min, m, dungeon[y][x], 0);
  } else {
    move_actor(m, dungeon[y][x], min);
  }
  return 0;
}

int dumb_move(monster *m) {
  int x = m->x, y = m->y;
  cell *c = get_cell_between(x, y, m->target_x, m->target_y);
  if(c->hardness > 0) {
    if(m->tunneler) {
      int h = c->hardness;
      h -= 85;
      if(h <= 0) {
	c->hardness = 0;
	c->norm_map = D_CORR;
	move_actor(m, dungeon[y][x], c);
	return 0;
      } else {
	c->hardness -= 85;
	return 0;
      }
    } else {
      return 0;
    }
  }
  if(c->actor == p) {
    attack_actor(c->actor, c, m, dungeon[y][x], 0);
  } else {
    move_actor(m, dungeon[y][x], c);
  }
  return 0;
}

int rand_move(monster *m) {
  if(!m->erratic) m->searching = 0;
  int x = m->x, y = m->y,
    rx, ry;
  cell *c = 0;
  while(1) {
    rx = (rand() % 3) - 1;
    ry = (rand() % 3) - 1;
    if(x + rx < 80 && x + rx > -1 &&
       y + ry < 21 && y + ry > -1) {
      c = dungeon[y + ry][x + rx];
      if((m->tunneler && c->hardness != 255) ||
	 c->hardness == 0) break;
    }
  }
  if(c->hardness > 0) {
    if(m->tunneler) {
      int h = c->hardness;
      h -= 85;
      if(h <= 0) {
	c->hardness = 0;
	c->norm_map = D_CORR;
	move_actor(m, dungeon[y][x], c);
	return 0;
      } else {
	c->hardness -= 85;
	return 0;
      }
    } else {
      return 0;
    }
  }
  if(c->actor == p) {
    attack_actor(c->actor, c, m, dungeon[y][x], 0);
  } else {
    move_actor(m, dungeon[y][x], c);
  }
  return 0;
}

void get_new_dest(monster *m) {
  int rx, ry, x_offset = 0, y_offset = 0;
  if(m->x < 40) x_offset = 40;
  if(m->y < 10) y_offset = 10;
  while(1) {
    rx = x_offset + rand() % 40;
    ry = y_offset + rand() % 11;
    if(dungeon[ry][rx]->norm_map == D_FLOOR) {
      m->target_x = rx;
      m->target_y = ry;
      m->searching = 1;
      break;
    }
  }
}

int monster_move(actor *a) {
  if(!a->alive) return 0;
  monster *m = (monster *)a;
  int d = detect_pc(m);
  int s = m->smart, p = m->psychic,
    t = m->tunneler, e = m->erratic;
  if(m->guard) {
    if(!m->searching) get_new_dest(m);
    smart_move(m);
  } else if(e && rand() % 2) {
    rand_move(m);
  } else if(!t && !p && !s) {
    if(d) {
      dumb_move(m);
    } else {
      rand_move(m);
    }
  } else if(!p && !t && s) {
    if(d || m->searching) {
      smart_move(m);
    } else {
      rand_move(m);
    }
  } else if(!t && p && !s) {
    dumb_move(m);
  } else if(!t && p && s) {
    smart_move(m);
  } else if(t && !p && !s) {
    if(d) {
      dumb_move(m);
    } else {
      rand_move(m);
    }
  } else if(t && !p && s) {
    if(d || m->searching) {
      smart_move(m);
    } else {
      rand_move(m);
    }
  } else if(t && p && !s) {
    dumb_move(m);
  } else if(t && p && s) {
    smart_move(m);
  }
  if(!m->searching) {
    m->target_x = m->x;
    m->target_y = m->y;
  }
  return 0;
}

actor_move *move_event(actor *a) {
  actor_move *e = new actor_move();
  e->next_turn = a == p ? 0 : 100 / a->speed;
  e->priority = a->priority;
  e->repeat = 1;
  e->target = a;
  if(a == p) {
    e->action = pc_move;
  } else {
    e->action = monster_move;
  }
  return e;
}

void do_bomb_animation(int rad, int x, int y) {
  int count = 0, i, j;
  while(count < rad) {
    if(p->sneaking) {
      for(i = y - rad; i < y + rad; i++) {
	for(j = x - rad; j < x + rad; j++) {
	  if(i < 21 && i > -1 && j < 80 && j > -1)
	    dungeon[i][j]->seen = 0;
	}
      }
    } else {
      for(i = 0; i < D_HEIGHT; i++) {
	for(j = 0; j < D_WIDTH; j++) {
	  dungeon[i][j]->seen = 0;
	}
      }
    }
    i = y - count;
    for(j = x - count; j < x + count; j++) {
      if(i < 21 && i > -1 && j < 80 && j > -1 &&
	 !bres_cells(x, y, j, i, is_blocked))
	dungeon[i][j]->seen = 1;
    }
    i = y + count;
    for(j = x - count; j < x + count; j++) {
      if(i < 21 && i > -1 && j < 80 && j > -1 &&
	 !bres_cells(x, y, j, i, is_blocked))
	dungeon[i][j]->seen = 1;
    }
    j = x - count;
    for(i = y - count; i < y + count; i++) {
      if(i < 21 && i > -1 && j < 80 && j > -1 &&
	 !bres_cells(x, y, j, i, is_blocked))
	dungeon[i][j]->seen = 1;
    }
    j = x + count;
    for(i = y - count; i < y + count; i++) {
      if(i < 21 && i > -1 && j < 80 && j > -1 &&
	 !bres_cells(x, y, j, i, is_blocked))
	dungeon[i][j]->seen = 1;
    }
    if(cleared) {
      display_dungeon(GB, fog_of_war, 1);
    } else {
      display_dungeon(WB, fog_of_war, 1);
    }
    count++;
    usleep(75000);
  }
}

int blow_up(object *o) {
  int rad = o->area_of_effect, i, j;
  do_bomb_animation(rad, o->x, o->y);
  for(i = o->y - rad; i < o->y + rad; i++) {
    for(j = o->x - rad; j < o->x + rad; j++) {
      if(i < 21 && i > -1 && j < 80 && j > -1 &&
	 !bres_cells(o->x, o->y, j, i, is_blocked)) {
	cell *c = dungeon[i][j];
	if(c->actor) {
	  int val = c->actor->damage(o->dam_base);
	  char buff[80];
	  if(c->actor != p) {
	    monster *m = (monster *)c->actor;
	    snprintf(buff, sizeof(buff), "%s took %d damage from %s",
		     m->desc->name, val, o->desc->name);
	    if(!m->alive) {
	      snprintf(buff, sizeof(buff), "%s killed by %s",
		     m->desc->name, o->desc->name);
	      c->actor = 0;
	      p->kills++;
	    }
	    write_to_footer(buff, "");
	  } else {
	    snprintf(buff, sizeof(buff), "Took %d damage from %s",
		     val, o->desc->name);
	    write_to_footer(buff, "");
	  }
	}
      }
    }
  }
  return 0;
}

bomb_explode *bomb_event(actor_move *am, object *b, int delay) {
  bomb_explode *e = new bomb_explode();
  e->next_turn = am->next_turn + ((100/am->target->speed)*delay);
  e->action = blow_up;
  e->priority = num_mons + 1;
  e->repeat = 1;
  e->target = b;
  return e;
}

void do_moves(int smooth, int sight) {
  heap *eq = new heap();
  event *e;
  int i, stop = 0;
  /* Make a priority queue for events */
  eq->create(cmp_event, disp_event);
  /* Give turn to pc if alive */
  if(p->alive) eq->insert(move_event(p));
  /* Give turns to monsters who are alive */
  for(i = 0; i < num_mons; i++) {
    if(mons[i]->alive) eq->insert(move_event(mons[i]));
  }
  while(!stop) {
    if(smooth) {
      usleep(25000);
    }
    /* If the dungeon is recently empty apart from p */
    if(eq->size == 1 && !cleared) {
      /* Flag to display dungeon in green */
      cleared = 1;
      p->clears++;
    }
    /* Remove an event from queue */
    e = (event *)eq->remove_min();
    /* Make the move */
    int val = e->execute();

    /* Refresh pc stats */
    p->update_stats();
    display_pc_stats();

    /* Update detection and fog-of-war */
    if(sight) update_detect_map();
    if(fog_of_war) update_fog_of_war();

    update_sight_areas();
    
    /* If pc has just been killed, stop */
    if(!p->alive) stop = 1;
    /* If "quit" key was pressed, stop */
    if(val == -1) stop = 1;
    /* If new level, flush queue and change */
    if(val == -2) {
      while(eq->size) {
	event *ev = (event *)eq->remove_min();
	delete ev;
      }
      eq->create(cmp_event, disp_event);
      int i;
      change_level();
      e->next_turn = 0;
      for(i = 0; i < num_mons; i++) {
	eq->insert(move_event(mons[i]));
      }
    }
    if(val > 0) { /* Bomb has been placed */
      actor_move *am = (actor_move *)e;
      object *o = dungeon[am->target->y][am->target->x]->obj;
      eq->insert(bomb_event(am, o, val));
    }
    /* Reinsert non-bomb events into queue */
    if(e->repeat) {
      eq->insert(e->copy());
    }
    delete e;
    
    /* Display screen */
    if(cleared) {
      display_dungeon(GB, fog_of_war, p->sneaking);
    } else {
      display_dungeon(WB, fog_of_war, p->sneaking);
    }
  }
  /* Clean up */
  eq->destroy();
  delete eq;
}
