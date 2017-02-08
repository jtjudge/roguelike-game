==== README for Assignment 1.10 ====
========== Jeffrey Judge ===========

I chose to extend the roguelike game
with a few new features to make it
into a rudimentary stealth game.

- STEALTH GAMEPLAY
  The player can now crouch down and
sneak by pressing 's'. This limits
the range of monsters lines of sight
significantly--when sneaking, the
player is noticed only when they pass
into the white highlighted sight radius
around a monster.
  NOTE: if you want to use NPCs other
than the one provided below, you must
add a "RAD" field to them, e.g. "RAD 3".
  NOTE: you can only see the
sight areas when fog-of-war is off--
so I recommend the "--nofog" switch
if you want to play stealthy.
  These sight areas change direction
depending on whether the monster is
searching for the pc or not--see this status
with the "--detect" switch, which places
appropriate stealth indicators above monster
positions.
  If a monster is searching for the player,
the area to their back is exposed. This
allows the pc to run up and backstab for 2x
damage. Beware that backstabbed monsters will
immediately strike back if they are left alive.
  If a monster has detected the pc (!), it will
continue pursuit until line of sight is broken,
even if the pc leaves its sight radius. Meaning
if a monster sees you, you must duck around
a corner in sneak mode to get rid of this
detected status.
  This mechanic allows for the pc to lure an npc
into investigating a position by entering its
line of sight without sneaking, then circling
around stealthily and backstabbing.

- GUARDS
  A new monster ability has been added: guards.
A guard will constantly be in "searching" mode,
meaning its back will always be exposed. If not
pursuing the pc or investigating his location,
a guard will patrol between randomly selected
points on opposite ends of the dungeon.
  Make a guard monster by placing "GUARD" in
the ABIL section. Note that this overrides any
other abilities--guards cannot be telepathic,
tunneling, or erratic monsters.

- RANGED GAMEPLAY
  The player can now use his ranged weapons by
first equipping one into the appropriate slot,
then pressing 'r'. The ranged cursor will appear
and the player can move it to all corners of the
dungeon using the usual movement controls. Spaces
outside the player's vision are invalid for
ranged combat, which is indicated with a red
cursor.
  The damage done by a ranged attack is only equal
to the damage of the ranged weapon--not every equipped
weapon as it is with melee attacks. However, sneak
attacks (2x damage) can be performed with ranged
weapons, and the player is not immediately seen by
the recipient of the attack, lending a huge advantage
to stealthy ranged playstyles. The victim of a ranged
sneak attack will proceed to search in the direction
from which it was hit, making a change in positions
necessary for continued stealth.
  Ammo is infinite for all ranged weapons.

- BOMBS
  A new object type has been added: "BOMB".
These are time-delayed explosives that, when
dropped from a carry slot and given a number
of turns to wait, explode with a radius of 5
squares and deal damage equal to the base
damage in the object description.
  Simply use "BOMB" in the TYPE field of an
object description to create one, and specify
the damage, e.g. "DAM 50 + 0d1".
  I designed the bombs to be used in conjunction
with the new sneak action. The pc can lure enemies
to his last position, where they will find a bomb
about to go off in their face.
  Bombs are represented with '*' characters.

For the best experience, follow these
steps:

1) Copy and paste the example monster
and example bomb below to the TOP of
your desc files

2) Run the game with
"./rlg --testroom --nofog --detect --nummon 5
--montype 0 --numobj 5 --objtype 0"
to test out the sneaking, guards, and bombs

3) Swap the example ranged weapon into the
top slot of the object file and run again
to test sneaky ranged gameplay


BEGIN MONSTER
NAME Guard
SYMB G
COLOR CYAN
DESC
Just a regular Joe doing his job.
.
SPEED 4 + 1d4
HP 40 + 1d6
DAM 10 + 1d5
ABIL GUARD
RAD 5
END

BEGIN OBJECT
NAME Grenade
TYPE BOMB
COLOR YELLOW
DESC
Frag out!
.
DAM 50 + 0d1
HIT 0 + 0d1
SPEED 0 + 0d1
DODGE 0 + 0d1
ATTR 0 + 0d1
DEF 0 + 0d1
WEIGHT 0 + 0d1
VAL 0 + 0d1
END

BEGIN OBJECT
NAME Rifle
TYPE RANGED
COLOR MAGENTA
DESC
There are many like it,
but this one is mine.
.
DAM 25 + 1d5
HIT 0 + 0d1
SPEED 0 + 0d1
DODGE 0 + 0d1
ATTR 0 + 0d1
DEF 0 + 0d1
WEIGHT 0 + 0d1
VAL 0 + 0d1
END

Command line switches:

"--nofog"
  disables fog of war

"--seed <num>"
  seeds the RNG with num

"--nummon <num>"
  sets num monsters per level
  (0 < num < 50)

"--numroom <num>"
  sets num rooms per level
  (6 < num < 20)
  
"--numobj <num>"
  sets num objects per level
  (10 < num < 50)

"--montype <index>"
  spawns only monsters from the template
  at the specified index of the description
  file--starts from 0

"--objtype <index>"
  spawns only objects from the template at
  the specified index of the description
  file--starts at 0

"--lightrad <num>"
  sets the radius of the pc's fog of war

"--testroom"
  makes the first level a large open space
  with several columns--useful for testing
  enemy pathfinding

"--smooth"
  adds a short delay between all moves, as
  opposed to the default jump cut

"--speed <num>"
  sets the player speed to num--values
  between 1 and 100

"--detect"
  adds detection indicators above monsters'
  heads--'!' if they see PC, '?' if they are
  looking for him

All code is C++ written by me.
