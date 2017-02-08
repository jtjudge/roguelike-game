#ifndef PARSE_H
# define PARSE_H

#define MAX_DESC_LINES 100
#define MAX_DESC 20
#define MAX_COLORS 8

#define WEAPON '|'
#define OFFHAND ')'
#define RANGED '}'
#define ARMOR '['
#define HELMET ']'
#define CLOAK '('
#define GLOVES '{'
#define BOOTS '\\'
#define RING '='
#define AMULET '"'
#define LIGHT '_'
#define SCROLL '~'
#define BOOK '?'
#define FLASK '!'
#define GOLD '$'
#define AMMUNITION '/'
#define FOOD ','
#define WAND '-'
#define CONTAINER '%'
#define BOMB '*'
#define STACK '&'

class mon_desc {
 public:
  char *name;
  char symb;
  int num_colors;
  int colors[MAX_COLORS];
  char *desc[MAX_DESC_LINES];
  int desc_lines;
  int s_base, s_numdice, s_sides;
  int d_base, d_numdice, d_sides;
  int hp_base, hp_numdice, hp_sides;
  int smart, psychic, tunneler,
    erratic, pass, guard;
  int light_rad;
  int inv, inc, dup, ndef;
  mon_desc() {};
  ~mon_desc() {};
};

class obj_desc {
 public:
  char *name;
  char type;
  int num_colors;
  int colors[MAX_COLORS];
  int w_base, w_numdice, w_sides;
  int h_base, h_numdice, h_sides;
  int da_base, da_numdice, da_sides;
  int a_base, a_numdice, a_sides;
  int v_base, v_numdice, v_sides;
  int do_base, do_numdice, do_sides;
  int de_base, de_numdice, de_sides;
  int s_base, s_numdice, s_sides;
  char *desc[MAX_DESC_LINES];
  int desc_lines;
  int inv, inc, dup, ndef;
  obj_desc() {};
  ~obj_desc() {};
};

extern int num_mon_desc, num_obj_desc;
extern mon_desc *m_descs[MAX_DESC];
extern obj_desc *o_descs[MAX_DESC];

int parse_desc_files(void);
void destroy_invalid_desc(void);
void destroy_all_desc(void);
void display_all_mon_desc(void);
void display_all_obj_desc(void);

#endif
