#ifndef ACTOR_H
# define ACTOR_H

#define INV_SIZE 22
#define EQUIP_SIZE 12
#define CARRY_SIZE 10

#define PC_HP 100
#define PC_SIGHT_RAD 6
#define PC_SPEED 10
#define PC_DAM_BASE 0
#define PC_DAM_NUM 1
#define PC_DAM_SIDES 5

#include "parse.h"
#include "object.h"

extern int pc_speed;
extern int pc_light_rad;

class cell;

class actor {
 public:
  int x, y, speed, priority, alive, hp,
    max_hp, color, dam_base, dam_num,
    dam_sides, detecting, searching,
    target_x, target_y, light_rad;
  char icon;
  mon_desc *desc;
  actor() {}
  ~actor() {}
  int damage(int val);
};

class pc : public actor {
 public:
  int kills, clears, visits, sneaking;
  object *inventory[INV_SIZE];
  pc() {}
  ~pc() {}
  int pickup(object *o);
  int equip(int slot);
  int unequip(int slot);
  int drop(int slot);
  int expunge(int slot);
  object *get_obj(int slot);
  object **get_inv();
  void update_stats(void);
};

class monster : public actor {
 public:
  int smart, psychic, tunneler, erratic, pass, guard;
  monster() {}
  ~monster() {}
};

void write_to_footer(const char *, const char *);

pc *create_pc(void);
void destroy_pc(pc *p);
monster *create_monster(int montype, int px, int py);
monster *create_monster(mon_desc *d, int px, int py);
void destroy_monster(monster *m);

int attack_actor(actor *victim, cell *vic_c,
		 actor *attacker, cell *atk_c, int p);
int ranged_attack(actor *victim, actor *attacker,
		  object *weapon, int p);
void move_actor(actor *a, cell *c1, cell *c2);
void displace_actor(actor *a);
int get_slot_for_type(char c);

#endif
