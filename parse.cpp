#include <cstdlib>
#include <fstream>
#include <iostream>
#include <cstring>
#include <ncurses.h>

#include "parse.h"

using namespace std;

int num_mon_desc = 0, num_obj_desc = 0;
mon_desc *m_descs[MAX_DESC];
obj_desc *o_descs[MAX_DESC];
fstream f;

void getline(char *line) {
  int i;
  for(i = 0; i < 100; i++) {
    line[i] = '\0';
  }
  i = 0;
  while(!f.eof()) {
    char c = f.get();
    if(c == '\n') break;
    line[i++] = c;
  }
}

int getword(char *line) {
  int i, endl = 0;
  for(i = 0; i < 100; i++) {
    line[i] = '\0';
  }
  i = 0;
  while(!f.eof()) {
    char c = f.get();
    if(i > 0 && c == ' ') break;
    if(c == '\n') {
      endl = 1;
      break;
    }
    if(c != ' ') {
      line[i++] = c;
    }
  }
  if(endl) return 1;
  return 0;
}

void getdie(char *line) {
  int i;
  for(i = 0; i < 100; i++) {
    line[i] = '\0';
  }
  i = 0;
  while(!f.eof()) {
    char c = f.get();
    if(c == '+' || c == 'd' || c == '\n') break;
    if(c != ' ') line[i++] = c;
  }
}

int get_color_code(char *line) {
  if(strcmp(line, "RED") == 0) return COLOR_RED;
  if(strcmp(line, "GREEN") == 0) return COLOR_GREEN;
  if(strcmp(line, "BLUE") == 0) return COLOR_BLUE;
  if(strcmp(line, "CYAN") == 0) return COLOR_CYAN;
  if(strcmp(line, "YELLOW") == 0) return COLOR_YELLOW;
  if(strcmp(line, "MAGENTA") == 0) return COLOR_MAGENTA;
  if(strcmp(line, "WHITE") == 0) return COLOR_WHITE;
  if(strcmp(line, "BLACK") == 0) return COLOR_BLACK;
  return -1;
}

char const *get_color_from_code(int c) {
  char const *s;
  if(c == COLOR_RED) s = "RED";
  if(c == COLOR_GREEN) s = "GREEN";
  if(c == COLOR_BLUE) s = "BLUE";
  if(c == COLOR_CYAN) s = "CYAN";
  if(c == COLOR_YELLOW) s = "YELLOW";
  if(c == COLOR_MAGENTA) s = "MAGENTA";
  if(c == COLOR_WHITE) s = "WHITE";
  if(c == COLOR_BLACK) s = "BLACK";
  return s;
}

char get_type_code(char *s) {
  if(strcmp(s, "WEAPON") == 0) return WEAPON;
  if(strcmp(s, "OFFHAND") == 0) return OFFHAND;
  if(strcmp(s, "RANGED") == 0) return RANGED;
  if(strcmp(s, "ARMOR") == 0) return ARMOR;
  if(strcmp(s, "HELMET") == 0) return HELMET;
  if(strcmp(s, "CLOAK") == 0) return CLOAK;
  if(strcmp(s, "GLOVES") == 0) return GLOVES;
  if(strcmp(s, "BOOTS") == 0) return BOOTS;
  if(strcmp(s, "RING") == 0) return RING;
  if(strcmp(s, "AMULET") == 0) return AMULET;
  if(strcmp(s, "LIGHT") == 0) return LIGHT;
  if(strcmp(s, "SCROLL") == 0) return SCROLL;
  if(strcmp(s, "BOOK") == 0) return BOOK;
  if(strcmp(s, "FLASK") == 0) return FLASK;
  if(strcmp(s, "GOLD") == 0) return GOLD;
  if(strcmp(s, "AMMUNITION") == 0) return AMMUNITION;
  if(strcmp(s, "FOOD") == 0) return FOOD;
  if(strcmp(s, "WAND") == 0) return WAND;
  if(strcmp(s, "CONTAINER") == 0) return CONTAINER;
  if(strcmp(s, "BOMB") == 0) return BOMB;
  return '\0';
}

char const *get_type_from_code(char c) {
  char const *s;
  if(c == WEAPON) s = "WEAPON";
  if(c == OFFHAND) s = "OFFHAND";
  if(c == RANGED) s = "RANGED";
  if(c == ARMOR) s = "ARMOR";
  if(c == HELMET) s = "HELMET";
  if(c == CLOAK) s = "CLOAK";
  if(c == GLOVES) s = "GLOVES";
  if(c == BOOTS) s = "BOOTS";
  if(c == RING) s = "RING";
  if(c == AMULET) s = "AMULET";
  if(c == LIGHT) s = "LIGHT";
  if(c == SCROLL) s = "SCROLL";
  if(c == BOOK) s = "BOOK";
  if(c == FLASK) s = "FLASK";
  if(c == GOLD) s = "GOLD";
  if(c == AMMUNITION) s = "AMMUNITION";
  if(c == FOOD) s = "FOOD";
  if(c == WAND) s = "WAND";
  if(c == CONTAINER) s = "CONTAINER";
  if(c == BOMB) s = "BOMB";
  return s;
}

int name_taken(char *name) {
  int i;
  for(i = 0; i < num_mon_desc; i++) {
    if(strcmp(name, m_descs[i]->name) == 0) return 1;
  }
  return 0;
}

int o_name_taken(char *name) {
  int i;
  for(i = 0; i < num_obj_desc; i++) {
    if(strcmp(name, o_descs[i]->name) == 0) return 1;
  }
  return 0;
}

mon_desc *create_m_desc(void) {
  mon_desc *m = (mon_desc *)malloc(sizeof(mon_desc));
  /* Null out the pointers, so we know what to free
     if the desc ends up being invalid */
  m->name = NULL;
  int i;
  for(i = 0; i < MAX_DESC_LINES; i++) {
    m->desc[i] = NULL;
  }
  int name = 0, symb = 0, color = 0, desc = 0,
    speed = 0, dam = 0, hp = 0, abil = 0,
    rad = 0;
  char line[100];
  while(1) {
    if(f.eof()) break;
    getword(line);
    if(strcmp(line, "NAME") == 0) {
      if(++name > 1) { m->dup = 1; break; }
      getline(line);
      m->name = strdup(line);
      if(name_taken(m->name)) { m->ndef = 1; break; }
    }
    if(strcmp(line, "SYMB") == 0) {
      if(++symb > 1) { m->dup = 1; break; }
      getword(line);
      m->symb = line[0];
    }
    if(strcmp(line, "COLOR") == 0) {
      if(++color > 1) { m->dup = 1; break; }
      int endl = 0; m->num_colors = 0;
      while(!endl) {
	if(m->num_colors == MAX_COLORS) { m->inv = 1; break; }
	endl = getword(line);
	m->colors[m->num_colors] = get_color_code(line);
	if(m->colors[m->num_colors] == -1) { m->inv = 1; break; }
	m->num_colors++;
      }
      if(m->inv) break;
    }
    if(strcmp(line, "DESC") == 0) {
      if(++desc > 1) { m->dup = 1; break; }
      m->desc_lines = 0;
      int end = 0;
      while(!end) {
	getline(line);
	if(strcmp(line, ".") == 0) end = 1;
	if(strlen(line) > 77) {
	  m->inv = 1; break;
	} else {
	  m->desc[m->desc_lines] = strdup(line);
	  m->desc_lines++;
	}
      }
    }
    if(strcmp(line, "RAD") == 0) {
      if(++rad > 1) { m->dup = 1; break; }
      getword(line);
      m->light_rad = atoi(line);
    }
    if(strcmp(line, "SPEED") == 0) {
      if(++speed > 1) { m->dup = 1; break; }
      getdie(line);
      m->s_base = atoi(line);
      getdie(line);
      m->s_numdice = atoi(line);
      getdie(line);
      m->s_sides = atoi(line);
    }
    if(strcmp(line, "DAM") == 0) {
      if(++dam > 1) { m->dup = 1; break; }
      getdie(line);
      m->d_base = atoi(line);
      getdie(line);
      m->d_numdice = atoi(line);
      getdie(line);
      m->d_sides = atoi(line);
    }
    if(strcmp(line, "HP") == 0) {
      if(++hp > 1) { m->dup = 1; break; }
      getdie(line);
      m->hp_base = atoi(line);
      getdie(line);
      m->hp_numdice = atoi(line);
      getdie(line);
      m->hp_sides = atoi(line);
    }
    if(strcmp(line, "ABIL") == 0) {
      if(++abil > 1) { m->dup = 1; break; }
      m->smart = m->psychic = m->tunneler =
	m->erratic = m->pass = m->guard = 0;
      int endl = 0;
      while(!endl) {
	endl = getword(line);
	if(strcmp(line, "SMART") == 0) {
	  m->smart = 1;
	} else if(strcmp(line, "TELE") == 0) {
	  m->psychic = 1;
	} else if(strcmp(line, "TUNNEL") == 0) {
	  m->tunneler = 1;
	} else if(strcmp(line, "ERRATIC") == 0) {
	  m->erratic = 1;
	} else if(strcmp(line, "PASS") == 0) {
	  m->pass = 1;
        } else if(strcmp(line, "GUARD") == 0) {
	  m->guard = 1;
	}
      }
    }
    if(strcmp(line, "END") == 0) break;
  }
  if(!name || !symb || !color || !desc ||
     !speed || !dam || !hp || !abil || !rad) m->inc = 1;
  return m;
}

obj_desc *create_o_desc(void) {
  obj_desc *o = (obj_desc *)malloc(sizeof(obj_desc));
  o->name = NULL;
  int i;
  for(i = 0; i < MAX_DESC_LINES; i++) {
    o->desc[i] = NULL;
  }
  int name = 0, type = 0, color = 0, desc = 0,
    weight = 0, hit = 0, dam = 0, attr = 0,
    val = 0, dodge = 0, def = 0, speed = 0;
  char line[100];
  while(1) {
    if(f.eof()) break;
    getword(line);
    if(strcmp(line, "NAME") == 0) {
      if(++name > 1) { o->dup = 1; break; }
      getline(line);
      o->name = strdup(line);
      if(o_name_taken(o->name)) { o->ndef = 1; break; }
    }
    if(strcmp(line, "TYPE") == 0) {
      if(++type > 1) { o->dup = 1; break; }
      getword(line);
      o->type = get_type_code(line);
      if(o->type == '\0') { o->inv = 1; break; }
    }
    if(strcmp(line, "COLOR") == 0) {
      if(++color > 1) { o->dup = 1; break; }
      int endl = 0; o->num_colors = 0;
      while(!endl) {
	if(o->num_colors == MAX_COLORS) { o->inv = 1; break; }
	endl = getword(line);
	o->colors[o->num_colors] = get_color_code(line);
	if(o->colors[o->num_colors] == -1) { o->inv = 1; break; }
	o->num_colors++;
      }
      if(o->inv) break;
    }
    if(strcmp(line, "DESC") == 0) {
      if(++desc > 1) { o->dup = 1; break; }
      o->desc_lines = 0;
      int end = 0;
      while(!end) {
	getline(line);
	if(strcmp(line, ".") == 0) end = 1;
	if(strlen(line) > 77) {
	  o->inv = 1; break;
	} else {
	  o->desc[o->desc_lines] = strdup(line);
	  o->desc_lines++;
	}
      }
    }
    if(strcmp(line, "WEIGHT") == 0) {
      if(++weight > 1) { o->dup = 1; break; }
      getdie(line);
      o->w_base = atoi(line);
      getdie(line);
      o->w_numdice = atoi(line);
      getdie(line);
      o->w_sides = atoi(line);
    }
    if(strcmp(line, "HIT") == 0) {
      if(++hit > 1) { o->dup = 1; break; }
      getdie(line);
      o->h_base = atoi(line);
      getdie(line);
      o->h_numdice = atoi(line);
      getdie(line);
      o->h_sides = atoi(line);
    }
    if(strcmp(line, "DAM") == 0) {
      if(++dam > 1) { o->dup = 1; break; }
      getdie(line);
      o->da_base = atoi(line);
      getdie(line);
      o->da_numdice = atoi(line);
      getdie(line);
      o->da_sides = atoi(line);
    }
    if(strcmp(line, "ATTR") == 0) {
      if(++attr > 1) { o->dup = 1; break; }
      getdie(line);
      o->a_base = atoi(line);
      getdie(line);
      o->a_numdice = atoi(line);
      getdie(line);
      o->a_sides = atoi(line);
    }
    if(strcmp(line, "VAL") == 0) {
      if(++val > 1) { o->dup = 1; break; }
      getdie(line);
      o->v_base = atoi(line);
      getdie(line);
      o->v_numdice = atoi(line);
      getdie(line);
      o->v_sides = atoi(line);
    }
    if(strcmp(line, "DODGE") == 0) {
      if(++dodge > 1) { o->dup = 1; break; }
      getdie(line);
      o->do_base = atoi(line);
      getdie(line);
      o->do_numdice = atoi(line);
      getdie(line);
      o->do_sides = atoi(line);
    }
    if(strcmp(line, "DEF") == 0) {
      if(++def > 1) { o->dup = 1; break; }
      getdie(line);
      o->de_base = atoi(line);
      getdie(line);
      o->de_numdice = atoi(line);
      getdie(line);
      o->de_sides = atoi(line);}
    if(strcmp(line, "SPEED") == 0) {
      if(++speed > 1) { o->dup = 1; break; }
      getdie(line);
      o->s_base = atoi(line);
      getdie(line);
      o->s_numdice = atoi(line);
      getdie(line);
      o->s_sides = atoi(line);
    }
    if(strcmp(line, "END") == 0) break;
  }
  if(!name || !type || !color || !desc ||
     !weight || !hit || !dam || !attr ||
     !val || !dodge || !def || !speed) o->inc = 1;
  return o;
}

void destroy_invalid_desc(void) {
  int i, j, count = 0;
  for(i = 0; i < num_mon_desc; i++) {
    mon_desc *m = m_descs[i];
    if(m->inv || m->inc || m->dup || m->ndef) {
      if(m->name) free(m->name);
      for(j = 0; j < m->desc_lines; j++) {
	if(m->desc[j]) free(m->desc[j]);
      }
      free(m);
      m_descs[i] = NULL;
      count++;
    }
  }
  j = 0;
  for(i = 0; i < num_mon_desc; i++) {
    if(m_descs[i]) {
      m_descs[j] = m_descs[i];
      j++;
    }
  }
  num_mon_desc -= count;
  count = 0;
  for(i = 0; i < num_obj_desc; i++) {
    obj_desc *o = o_descs[i];
    if(o->inv || o->inc || o->dup || o->ndef) {
      if(o->name) free(o->name);
      for(j = 0; j < o->desc_lines; j++) {
	if(o->desc[j]) free(o->desc[j]);
      }
      free(o);
      o_descs[i] = NULL;
      count++;
    }
  }
  j = 0;
  for(i = 0; i < num_obj_desc; i++) {
    if(o_descs[i]) {
      o_descs[j] = o_descs[i];
      j++;
    }
  }
  num_obj_desc -= count;
}

void destroy_all_desc(void) {
  int i, j;
  for(i = 0; i < num_mon_desc; i++) {
    mon_desc *m = m_descs[i];
    if(!m->inv) {
      free(m->name);
      for(j = 0; j < m->desc_lines; j++) {
	free(m->desc[j]);
      }
    }
    free(m);
  }
  for(i = 0; i < num_obj_desc; i++) {
    obj_desc *o = o_descs[i];
    if(!o->inv) {
      free(o->name);
      for(j = 0; j < o->desc_lines; j++) {
	free(o->desc[j]);
      }
    }
    free(o);
  }
}

void display_all_mon_desc(void) {
  printf("\n");
  int i, j;
  for(i = 0; i < num_mon_desc; i++) {
    mon_desc *m = m_descs[i];
    if(m->inv) {
      printf("Invalid fields in \"%s\"\n\n", m->name);
    } else if(m->ndef) {
      printf("Multiple definitions for \"%s\"\n\n", m->name);
    } else if(m->inc) {
      printf("Missing fields in \"%s\"\n\n", m->name);
    } else if(m->dup) {
      printf("Duplicate fields in \"%s\"\n\n", m->name);
    } else {
      printf("%s\n", m->name);
      for(j = 0; j < m->desc_lines; j++) {
	printf("%s\n", m->desc[j]);
      }
      printf("%c\n", m->symb);
      for(j = 0; j < m->num_colors; j++) {
	printf("%s ", get_color_from_code(m->colors[j]));
      }
      printf("\n");
      printf("%d+%dd%d\n", m->s_base,
	     m->s_numdice, m->s_sides);
      if(m->smart) printf("SMART ");
      if(m->psychic) printf("TELE ");
      if(m->tunneler) printf("TUNNEL ");
      if(m->erratic) printf("ERRATIC ");
      if(m->pass) printf("PASS");
      printf("\n");
      printf("%d+%dd%d\n", m->hp_base,
	     m->hp_numdice, m->hp_sides);
      printf("%d+%dd%d\n\n", m->d_base,
	     m->d_numdice, m->d_sides);
    }
  }
}

void display_all_obj_desc(void) {
  printf("\n");
  int i, j;
  for(i = 0; i < num_obj_desc; i++) {
    obj_desc *o = o_descs[i];
    if(o->inv) {
      printf("Invalid fields in \"%s\"\n\n", o->name);
    } else if(o->ndef) {
      printf("Multiple definitions for \"%s\"\n\n", o->name);
    } else if(o->inc) {
      printf("Missing fields in \"%s\"\n\n", o->name);
    } else if(o->dup) {
      printf("Duplicate fields in \"%s\"\n\n", o->name);
    } else {
      printf("%s\n", o->name);
      for(j = 0; j < o->desc_lines; j++) {
	printf("%s\n", o->desc[j]);
      }
      printf("%s\n", get_type_from_code(o->type));
      for(j = 0; j < o->num_colors; j++) {
	printf("%s ", get_color_from_code(o->colors[j]));
      }
      printf("\n");
      printf("%d+%dd%d\n", o->h_base, o->h_numdice, o->h_sides);
      printf("%d+%dd%d\n", o->da_base, o->da_numdice, o->da_sides);
      printf("%d+%dd%d\n", o->do_base, o->do_numdice, o->do_sides);
      printf("%d+%dd%d\n", o->de_base, o->de_numdice, o->de_sides);
      printf("%d+%dd%d\n", o->w_base, o->w_numdice, o->w_sides);
      printf("%d+%dd%d\n", o->s_base, o->s_numdice, o->s_sides);
      printf("%d+%dd%d\n", o->a_base, o->a_numdice, o->a_sides);
      printf("%d+%dd%d\n\n", o->v_base, o->v_numdice, o->v_sides);
    }
  }
}

int parse_desc_files(void) {
  char path[50];
  snprintf(path, sizeof(path), "monster_desc.txt");
  f.open(path, ios::in);
  char line[100];
  int header_check = 0;
  if(!f.is_open()) {
    printf("Could not read monster description file\n");
    return -1;
  }
  while(1) {
    getline(line);
    if(f.eof()) break;
    if(!header_check) {
      if(strcmp(line, "RLG327 MONSTER DESCRIPTION 1") != 0) {
	printf("Invalid file header.\n");
	return -1;
      }
      header_check = 1;
    }
    if(strcmp(line, "BEGIN MONSTER") == 0) {
      m_descs[num_mon_desc] = create_m_desc();
      num_mon_desc++;
    }
  }
  f.close();
  snprintf(path, sizeof(path), "object_desc.txt");
  f.open(path, ios::in);
  if(!f.is_open()) {
    printf("Could not read object description file\n");
    return -1;
  }
  header_check = 0;
  while(1) {
    getline(line);
    if(f.eof()) break;
    if(!header_check) {
      if(strcmp(line, "RLG327 OBJECT DESCRIPTION 1") != 0) {
	printf("Invalid file header.\n");
	return -1;
      }
      header_check = 1;
    }
    if(strcmp(line, "BEGIN OBJECT") == 0) {
      o_descs[num_obj_desc] = create_o_desc();
      num_obj_desc++;
    }
  }
  f.close();
  return 0;
}
