#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ncurses.h>

#include "actor.h"
#include "heap.h"
#include "dungeon.h"
#include "nav.h"
#include "game.h"
#include "parse.h"

int main(int argc, char *argv[]) {
  
  /* Get command line arguments */
  int i, seed = 0, smooth = 0, detect = 0,
    speed = 0, montype = -1, objtype = -1,
    testroom = 0, fog = 1;
  num_objs = -1;
  num_mons = -1;
  num_rooms = -1;
  pc_light_rad = -1;
  for(i = 1; i < argc; i++) {
    char *s = argv[i];
    if(strcmp("--nummon", s) == 0) {
      char *num = argv[++i];
      if(num) {
	num_mons = atoi(num);
	if(num_mons > MAX_MONS) {
	  printf("Too many monsters.\n");
	  return -1;
	} else if(num_mons < 0) {
	  printf("Too few monsters.\n");
	  return -1;
	}
      } else {
	printf("Invalid args.\n");
	return -1;
      }
    } else if(strcmp("--numroom", s) == 0) {
      char *num = argv[++i];
      if(num) {
	num_rooms = atoi(num);
	if(num_rooms > MAX_ROOMS) {
	  printf("Too many rooms.\n");
	  return -1;
	} else if(num_rooms < 1) {
	  printf("Too few rooms.\n");
	  return -1;
	}
      } else {
	printf("Invalid args.\n");
	return -1;
      }
    } else if(strcmp("--numobj", s) == 0) {
      char *num = argv[++i];
      if(num) {
	num_objs = atoi(num);
	if(num_objs > MAX_OBJS) {
	  printf("Too many objects.\n");
	  return -1;
	} else if(num_objs < 0) {
	  printf("Too few objects.\n");
	  return -1;
	}
      } else {
	printf("Invalid args.\n");
	return -1;
      }
    } else if(strcmp("--seed", s) == 0) {
      char *num = argv[++i];
      if(num) {
	seed = atoi(num);
      } else {
	printf("Invalid args.\n");
	return -1;
      }
    } else if(strcmp("--smooth", s) == 0) {
      smooth = 1;
    } else if(strcmp("--detect", s) == 0) {
      detect = 1;
    } else if(strcmp("--speed", s) == 0) {
      char *num = argv[++i];
      if(num) {
	speed = atoi(num);
      } else {
	printf("Invalid args.\n");
	return -1;
      }
    } else if(strcmp("--montype", s) == 0) {
      char *type = argv[++i];
      if(type) {
	montype = atoi(type);
      } else {
	printf("Invalid args.\n");
	return -1;
      }
    } else if(strcmp("--objtype", s) == 0) {
      char *type = argv[++i];
      if(type) {
	objtype = atoi(type);
      } else {
	printf("Invalid args.\n");
	return -1;
      }
    } else if(strcmp("--testroom", s) == 0) {
      testroom = 1;
    } else if(strcmp("--nofog", s) == 0) {
      fog = 0;
    } else if(strcmp("--lightrad", s) == 0) {
      char *num = argv[++i];
      if(num) {
	pc_light_rad = atoi(num);
      } else {
	printf("Invalid args.\n");
	return -1;
      }
    } else {
      printf("%s is invalid.\n", s);
      return -1;
    }
  }
  if(seed) {
    srand(seed);
  } else {
    srand(time(NULL));
  }
  if(num_mons < 0) num_mons = 3 + rand() % 4;
  if(num_rooms < 1) num_rooms = 5 + rand() % 16;
  if(num_objs < 0) num_objs = 10 + rand() % 6;
  if(pc_light_rad < 1)   
    pc_light_rad = PC_SIGHT_RAD;
  /* Get monster and object descriptions */
  if(parse_desc_files()) return -1;
  destroy_invalid_desc();

  if(montype > num_mon_desc - 1) {
    printf("Montype index too large.\n");
    destroy_all_desc();
    return -1;
  }

  if(objtype > num_obj_desc - 1) {
    printf("Objtype index too large.\n");
    destroy_all_desc();
    return -1;
  }
  
  /* Create persistant PC */
  if(speed) {
    pc_speed = speed;
  } else {
    pc_speed = PC_SPEED;
  }
  p = create_pc();
    
  /* Curses stuff */
  initscr();
  raw();
  noecho();
  curs_set(0);
  keypad(stdscr, TRUE);
  if (getenv ("ESCDELAY") == NULL) set_escdelay(25);
  /* Colors defined in dungeon.h */
  start_color();
  /* Black on black will be displayed as white on black */
  init_pair(BAB, COLOR_WHITE, COLOR_BLACK);
  init_pair(BAW, COLOR_BLACK, COLOR_WHITE);
  init_pair(RB, COLOR_RED, COLOR_BLACK);
  init_pair(RW, COLOR_RED, COLOR_WHITE);
  init_pair(GB, COLOR_GREEN, COLOR_BLACK);
  init_pair(GW, COLOR_GREEN, COLOR_WHITE);
  init_pair(YB, COLOR_YELLOW, COLOR_BLACK);
  init_pair(YW, COLOR_YELLOW, COLOR_WHITE);
  init_pair(BUB, COLOR_BLUE, COLOR_BLACK);
  init_pair(BUW, COLOR_BLUE, COLOR_WHITE);
  init_pair(MB, COLOR_MAGENTA, COLOR_BLACK);
  init_pair(MW, COLOR_MAGENTA, COLOR_WHITE);
  init_pair(CB, COLOR_CYAN, COLOR_BLACK);
  init_pair(CW, COLOR_CYAN, COLOR_WHITE);
  init_pair(WB, COLOR_WHITE, COLOR_BLACK);
  /* White on white will be displayed as black on white */
  init_pair(WW, COLOR_BLACK, COLOR_WHITE);

  /* For stats bars */
  init_pair(GG, COLOR_GREEN, COLOR_GREEN);
  init_pair(RR, COLOR_RED, COLOR_RED);
  init_pair(YY, COLOR_YELLOW, COLOR_YELLOW);
  init_pair(CC, COLOR_CYAN, COLOR_CYAN);

  /* For ranged combat cursor */
  init_pair(BAR, COLOR_BLACK, COLOR_RED);
  init_pair(GR, COLOR_GREEN, COLOR_RED);
  init_pair(YR, COLOR_YELLOW, COLOR_RED);
  init_pair(BUR, COLOR_BLUE, COLOR_RED);
  init_pair(MR, COLOR_MAGENTA, COLOR_RED);
  init_pair(CR, COLOR_CYAN, COLOR_RED);
  init_pair(WR, COLOR_WHITE, COLOR_RED);
  
  /* Set up the game */
  int spawnmons = montype == -1 ? 1 : 0;
  int spawnobjs = objtype == -1 ? 1 : 0;
  rooms = (room **)malloc(num_rooms * sizeof(room *));

  set_up_game(-1, -1, spawnmons, spawnobjs, testroom, fog);
  
  /* Create monsters if necessary */
  if(!spawnmons) {
    mons = (monster **)malloc(num_mons * sizeof(monster *));
    int i;
    for(i = 0; i < num_mons; i++) {
      mons[i] = create_monster(m_descs[montype], p->x, p->y);
    }
  }
  
  /* Create objects if necessary */
  if(!spawnobjs) {
    objs = (object **)malloc((num_objs + CARRY_SIZE + EQUIP_SIZE)
			     * sizeof(object *));
    int i;
    for(i = 0; i < num_objs; i++) {
      objs[i] = create_object(o_descs[objtype]);
    }
  }

  if(fog) update_fog_of_war();
  
  write_to_header(STD_MESSAGE);
  display_pc_stats();
  display_dungeon(WB, fog, 0);
  
  /* GAME LOOP */
  do_moves(smooth, detect);

  /* Death screen */
  display_dungeon(RB, 0, 0);
  write_end_message();
  int ch = getch();
  while(ch != 0161) {
    ch = getch();
  }
  /* Clean up */
  tear_down_game();
  destroy_pc(p);
  destroy_all_desc();
  endwin();
}
