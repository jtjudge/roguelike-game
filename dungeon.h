#ifndef DUNGEON_H
# define DUNGEON_H

#define D_HEIGHT 21
#define D_WIDTH 80
#define INF 2147483647

#define D_FLOOR '.'
#define D_CORR '#'
#define D_ROCK ' '
#define D_PC '@'
#define D_UP_STAIRS '<'
#define D_DOWN_STAIRS '>'

/* Color codes: "RB" = "red on black" */
#define BAB 0
#define BAW 1
#define RB 2
#define RW 3
#define GB 4
#define GW 5
#define YB 6
#define YW 7
#define BUB 8
#define BUW 9
#define MB 10
#define MW 11
#define CB 12
#define CW 13
#define WB 14
#define WW 15
#define GG 16
#define YY 18
#define CC 19

/* For ranged combat cursor */
#define BAR 20
#define RR 22
#define GR 24
#define YR 26
#define BUR 28
#define MR 30
#define CR 32
#define WR 34

class actor;
class object;

int get_color(actor *a);
char get_icon(actor *a);

class room {
 public:
  int x, y, w, h, connected;
  room() {};
  ~room() {};
};

class cell {
 public:
  char norm_map, pc_view,
    nav_map, t_nav_map, detect_map;
  unsigned char hardness;
  int x, y, weight, dist, t_dist,
    visited, fog, seen, blocked,
    cursor;
  class actor *actor;
  object *obj;
  cell() {};
  ~cell() {};
};

extern int num_rooms;
extern room **rooms;
extern cell *dungeon[D_HEIGHT][D_WIDTH];

void generate_dungeon(int test_room);
void spawn_pc(int x, int y);
void display_dungeon(int color, int fog, int sneak);
void destroy_dungeon(void);

int bres_cells(int x1, int y1, int x2, int y2, int (*func)(cell *));

#endif
