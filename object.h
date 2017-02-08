#ifndef OBJECT_H
# define OBJECT_H

#define DEF_RAD 5

class obj_desc;

class object {
 public:
  int x, y;
  char icon;
  int color, weight, hit, attr,
    val, dodge, def, speed;
  int dam_base, dam_dice, dam_sides;
  int inv, live_bomb, area_of_effect;
  obj_desc *desc;
  object() {}
  ~object() {}
};

object *create_object(obj_desc *d);
void destroy_object(object *o);

#endif
