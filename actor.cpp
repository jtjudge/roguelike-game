#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>

#include "actor.h"
#include "parse.h"
#include "dungeon.h"

int pc_speed, pc_light_rad;

pc *create_pc(void) {
  pc *p = new pc();
  p->speed = pc_speed;
  p->priority = 0;
  p->alive = 1;
  p->icon = D_PC;
  p->color = GB;
  p->kills = 0;
  p->clears = 0;
  p->visits = 1;
  p->sneaking = p->searching = p->detecting = 0;
  p->hp = p->max_hp = PC_HP;
  p->dam_base = PC_DAM_BASE;
  p->dam_num = PC_DAM_NUM;
  p->dam_sides = PC_DAM_SIDES;
  p->light_rad = pc_light_rad;
  int i;
  for(i = 0; i < INV_SIZE; i++) {
    p->inventory[i] = 0;
  }
  return p;
}

void destroy_pc(pc *p) {
  delete p;
}

monster *create_monster(int montype, int px, int py) {
  monster *m = new monster();
  /* Randomly place in dungeon */
  while(1) {
    int rx = rand() % D_WIDTH;
    int ry = rand() % D_HEIGHT;
    /* Defines a 15 X 10 "safe zone" around pc spawn */
    if(!((rx > px - 15 && rx < px + 15) &&
	 (ry > py - 10 || ry < py + 10))) {
      /* Place monsters in any room cell */
      if(dungeon[ry][rx]->norm_map == D_FLOOR &&
	 !(dungeon[ry][rx]->actor)) {
	m->x = rx;
	m->y = ry;
	dungeon[ry][rx]->actor = (actor *)m;
	break;
      }
    }
  }
  m->target_x = m->x;
  m->target_y = m->y;
  m->priority = 1;
  m->alive = 1;
  /* Assign attributes */
  if(montype == 0 || montype == 8) {
    m->smart = m->psychic = m->tunneler = 0;
  }
  if(montype == 1 || montype == 9) {
    m->smart = 1;
    m->psychic = m->tunneler = 0;
  }
  if(montype == 2 || montype == 10) {
    m->psychic = 1;
    m->smart = m->tunneler = 0;
  }
  if(montype == 3 || montype == 11) {
    m->smart = m->psychic = 1;
    m->tunneler = 0;
  }
  if(montype == 4 || montype == 12) {
    m->tunneler = 1;
    m->smart = m->psychic = 0;
  }
  if(montype == 5 || montype == 13) {
    m->smart = m->tunneler = 1;
    m->psychic = 0;
  }
  if(montype == 6 || montype == 14) {
    m->psychic = m->tunneler = 1;
    m->smart = 0;
  }
  if(montype == 7 || montype == 15) {
    m->smart = m->psychic = m->tunneler = 1;
  }
  m->erratic = montype > 7 ? 1 : 0;
  m->pass = m->guard = 0;
  if(montype < 10) {
    m->icon = montype + 48;
  } else {
    m->icon = montype + 87;
  }
  /* Default settings */
  m->color = RB;
  m->hp = m->max_hp = 20;
  m->dam_base = 10;
  m->dam_num = 1;
  m->dam_sides = 6;
  m->speed = 5 + rand() % 16;
  m->desc = 0;
  m->light_rad = 5;
  m->detecting = m->searching = 0;
  return m;
}

/* Create monster using a description */
monster *create_monster(mon_desc *d, int px, int py) {
  if(!d || d->inv || d->inc || d->dup || d->ndef) return 0;
  monster *m = new monster();
  m->desc = d;
  m->icon = d->symb;
  m->color = d->colors[0] * 2;
  m->smart = d->smart;
  m->psychic = d->psychic;
  m->tunneler = d->tunneler;
  m->erratic = d->erratic;
  m->pass = d->pass;
  m->guard = d->guard;
  m->max_hp = d->hp_base;
  m->speed = d->s_base;
  m->light_rad = d->light_rad;
  int i;
  for(i = 0; i < d->hp_numdice; i++) {
    m->max_hp += 1 + (rand() % d->hp_sides);
  }
  m->hp = m->max_hp;
  for(i = 0; i < d->s_numdice; i++) {
    m->speed += 1 + (rand() % d->s_sides);
  }
  m->dam_base = d->d_base;
  m->dam_num = d->d_numdice;
  m->dam_sides = d->d_sides;
  /* Randomly place in dungeon */
  while(1) {
    int rx = rand() % D_WIDTH;
    int ry = rand() % D_HEIGHT;
    /* Defines a 15 X 10 "safe zone" around pc spawn */
    if(!((rx > px - 15 && rx < px + 15) &&
	 (ry > py - 10 || ry < py + 10))) {
      /* Place monsters in any room cell */
      if(dungeon[ry][rx]->norm_map == D_FLOOR &&
	 !(dungeon[ry][rx]->actor)) {
	m->x = rx;
	m->y = ry;
	dungeon[ry][rx]->actor = (actor *)m;
	break;
      }
    }
  }
  m->target_x = m->x;
  m->target_y = m->y;
  m->priority = 1;
  m->alive = 1;
  m->detecting = m->searching = 0;
  return m;
}

void destroy_monster(monster *m) {
  delete m;
}

int actor::damage(int val) {
  this->hp -= val;
  if(this->hp <= 0) {
    this->hp = 0;
    this->alive = 0;
  }
  return val;
}

int attack_actor(actor *vic, cell *vic_c,
		 actor *atk, cell *atk_c, int p) {
  int dam = atk->dam_base, sneak = 0, i;
  for(i = 0; i < atk->dam_num; i++) {
    dam += 1 + rand() % atk->dam_sides;
  }
  if(p) {
    pc *p = (pc *)atk;
    for(i = CARRY_SIZE; i < CARRY_SIZE + EQUIP_SIZE; i++) {
      object *o = p->inventory[i];
      if(o) {
	dam += o->dam_base;
	int j;
	for(j = 0; j < o->dam_dice; j++) {
	  dam += 1 + rand() % o->dam_sides;
	}
      }
    }
  }
  if(!vic->detecting) {
    dam *= 2;
    sneak = 1;
  }
  vic->hp -= dam;
  vic->detecting = 1;
  vic->searching = 1;
  vic->target_x = atk->x;
  vic->target_y = atk->y;
  char buff[80];
  if(p && vic->desc) {
    if(sneak) {
      snprintf(buff, sizeof(buff), "Backstabbed %s for %d damage",
	       vic->desc->name, dam);
    } else {
      snprintf(buff, sizeof(buff), "Hit %s for %d damage",
	       vic->desc->name, dam);
    }
  } else if(p) {
    snprintf(buff, sizeof(buff), "Hit %c for %d damage",
	     vic->icon, dam);
  } else if(atk->desc) {
    snprintf(buff, sizeof(buff), "Took %d damage from %s",
	     dam, atk->desc->name);
  } else {
    snprintf(buff, sizeof(buff), "Took %d damage from %c",
	     dam, atk->icon);
  }
  if(vic->hp <= 0) {
    vic->hp = 0;
    vic->alive = 0;
    vic_c->actor = 0;
    atk_c->actor = 0;
    vic_c->actor = atk;
    atk->x = vic_c->x;
    atk->y = vic_c->y;
    if(p) {
      write_to_footer(buff, "Monster killed");
    } else {
      write_to_footer(buff, "Died");
    }
    return 1;
  }
  write_to_footer(buff, "");
  return 0;
}

void move_actor(actor *a, cell *c1, cell *c2) {
  actor *c2a = c2->actor;
  c1->actor = c2a;
  c2->actor = a;
  a->x = c2->x;
  a->y = c2->y;
  if(c2a) {
    c2a->x = c1->x;
    c2a->y = c1->y;
  }
}

int get_slot_for_type(char c) {
  int s = 0;
  if(c == WEAPON) s = 10;
  if(c == OFFHAND) s = 11;
  if(c == RANGED) s = 12;
  if(c == ARMOR) s = 13;
  if(c == HELMET) s = 14;
  if(c == CLOAK) s = 15;
  if(c == GLOVES) s = 16;
  if(c == BOOTS) s = 17;
  if(c == AMULET) s = 18;
  if(c == LIGHT) s = 19;
  if(c == RING) s = 20;
  return s;
}

int ranged_attack(actor *vic, actor *atk,
		  object *weapon, int p) {
  int dam = weapon->dam_base, i, sneak = 0;
  for(i = 0; i < weapon->dam_dice; i++) {
    dam += 1 + rand() % weapon->dam_sides;
  }
    if(!vic->detecting) {
    dam *= 2;
    sneak = 1;
  }
  vic->hp -= dam;
  vic->searching = 1;
  vic->target_x = atk->x;
  vic->target_y = atk->y;
  char buff[80];
  if(p && vic->desc) {
    if(sneak) {
      snprintf(buff, sizeof(buff), "Sneak attack on %s for %d damage",
	       vic->desc->name, dam);
    } else {
      snprintf(buff, sizeof(buff), "Hit %s for %d damage",
	       vic->desc->name, dam);
    }
  } else if(p) {
    snprintf(buff, sizeof(buff), "Hit %c for %d damage",
	     vic->icon, dam);
  } else if(atk->desc) {
    snprintf(buff, sizeof(buff), "Took %d damage from %s",
	     dam, atk->desc->name);
  } else {
    snprintf(buff, sizeof(buff), "Took %d damage from %c",
	     dam, atk->icon);
  }
  if(vic->hp <= 0) {
    vic->hp = 0;
    vic->alive = 0;
    dungeon[vic->y][vic->x]->actor = 0;
    if(p) {
      write_to_footer(buff, "Monster killed");
    } else {
      write_to_footer(buff, "Died");
    }
    return 1;
  }
  write_to_footer(buff, "");
  return 0;
}

int pc::pickup(object *o) {
  if(!o) return -1;
  if(o->live_bomb) return -2;
  int i;
  for(i = 0; i < CARRY_SIZE; i++) {
    if(!this->inventory[i]) {
      this->inventory[i] = o;
      o->inv = 1;
      o->x = o->y = -1;
      dungeon[this->y][this->x]->obj = 0;
      return 0;
    }
  }
  return -2;
}

int pc::equip(int slot) {
  if(slot < 0 || slot > CARRY_SIZE) return -1;
  object *o1 = this->inventory[slot];
  if(!o1) return -2;
  int e_slot = get_slot_for_type(o1->icon);
  if(!e_slot) return -3;
  if(e_slot == 20 && this->inventory[20]) e_slot++;
  object *o2 = this->inventory[e_slot];
  int swapped = 0;
  if(o2) {
    this->inventory[slot] = o2;
    swapped = 1;
  } else {
    this->inventory[slot] = 0;
  }
  this->inventory[e_slot] = o1;
  return swapped;
}

int pc::unequip(int slot) {
  if(slot < CARRY_SIZE ||
     slot > CARRY_SIZE + EQUIP_SIZE) return -1;
  object *o = this->inventory[slot];
  if(!o) return -2;
  int i;
  for(i = 0; i < CARRY_SIZE; i++) {
    if(!this->inventory[i]) {
      this->inventory[slot] = 0;
      this->inventory[i] = o;
      return 0;
    }
  }
  return -3;
}

int pc::drop(int slot) {
  if(slot < 0 || slot > CARRY_SIZE) return -1;
  object *o = this->inventory[slot];
  if(!o) return -2;
  if(dungeon[this->y][this->x]->obj) return -3;
  dungeon[this->y][this->x]->obj = o;
  o->inv = 0;
  o->x = this->x;
  o->y = this->y;
  this->inventory[slot] = 0;
  /* Bomb drop code */
  if(o->icon == BOMB) return 1;
  return 0;
}

int pc::expunge(int slot) {
  if(slot < 0 || slot > CARRY_SIZE) return -1;
  object *o = this->inventory[slot];
  if(!o) return -2;
  o->inv = 0;
  this->inventory[slot] = 0;
  return 0;
}

object *pc::get_obj(int slot) {
  if(slot < 0 || slot > CARRY_SIZE + EQUIP_SIZE) return 0;
  return this->inventory[slot];
}

object **pc::get_inv() {
  return this->inventory;
}

void pc::update_stats(void) {
  /* Reset stats */
  this->speed = pc_speed;
  int i;
  for(i = CARRY_SIZE; i < CARRY_SIZE + EQUIP_SIZE; i++) {
    object *o = this->inventory[i];
    if(o) {
      this->speed += o->speed;
    }
  }
  if(this->speed < 1) this->speed = 1;
  if(this->speed > 100) this->speed = 100;
}

int get_color(actor *a) { return a->color; }
char get_icon(actor *a) { return a->icon; }
