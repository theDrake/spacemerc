/******************************************************************************
   Filename: space_merc.c

     Author: David C. Drake (http://davidcdrake.com)

Description: Function definitions for SpaceMerc, a 3D first-person shooter
             developed for the Pebble smartwatch (SDK 2.0). Copyright 2014,
             David C. Drake. More info online: http://davidcdrake.com/spacemerc
******************************************************************************/

#include "space_merc.h"

/******************************************************************************
   Function: set_player_direction

Description: Sets the player's orientation to a given direction and updates the
             compass accordingly.

     Inputs: new_direction - Desired orientation.

    Outputs: None.
******************************************************************************/
void set_player_direction(const int16_t new_direction)
{
  g_player->direction = new_direction;
  switch(new_direction)
  {
    case NORTH:
      gpath_rotate_to(g_compass_path, TRIG_MAX_ANGLE / 2);
      break;
    case SOUTH:
      gpath_rotate_to(g_compass_path, 0);
      break;
    case EAST:
      gpath_rotate_to(g_compass_path, TRIG_MAX_ANGLE * 0.75);
      break;
    default: // case WEST:
      gpath_rotate_to(g_compass_path, TRIG_MAX_ANGLE / 4);
      break;
  }
  layer_mark_dirty(window_get_root_layer(g_graphics_window));
}

/******************************************************************************
   Function: move_player

Description: Attempts to move the player one cell forward in a given direction.

     Inputs: direction - Desired direction of movement.

    Outputs: None.
******************************************************************************/
void move_player(const int16_t direction)
{
  GPoint destination = get_cell_farther_away(g_player->position, direction, 1);

  // Check for movement into the exit, ending the current mission:
  if (gpoint_equal(&g_player->position, &g_mission->starting_point) &&
      g_player->direction == g_mission->entrance_direction)
  {
    end_mission();
  }
  else if (occupiable(destination))
  {
    // Shift the player's position:
    g_player->position = destination;

    // Check for completion of extricate/expropriate missions:
    if (get_cell_type(destination) == HUMAN ||
        get_cell_type(destination) == ITEM)
    {
      set_cell_type(destination, EMPTY);
      g_mission->completed = true;
    }
    layer_mark_dirty(window_get_root_layer(g_graphics_window));
  }
}

/******************************************************************************
   Function: move_npc

Description: Attempts to move a given NPC one cell forward in a given
             direction.

     Inputs: npc       - Pointer to the NPC to be moved.
             direction - Desired direction of movement.

    Outputs: None.
******************************************************************************/
void move_npc(npc_t *npc, const int16_t direction)
{
  GPoint destination = get_cell_farther_away(npc->position, direction, 1);

  if (occupiable(destination))
  {
    npc->position = destination;
  }
}

/******************************************************************************
   Function: determine_npc_behavior

Description: Determines what a given NPC should do.

     Inputs: npc - Pointer to the NPC of interest.

    Outputs: None.
******************************************************************************/
void determine_npc_behavior(npc_t *npc)
{
  if (touching(npc->position, g_player->position))
  {
    damage_player(npc->power);
  }
  else
  {
    move_npc(npc, get_pursuit_direction(npc->position,
                                        g_player->position));
  }
}

/******************************************************************************
   Function: get_pursuit_direction

Description: Determines in which direction a character at a given position
             ought to move in order to pursue a character at another given
             position. (Simplistic: no complex path-finding.)

     Inputs: pursuer - Position of the pursuing character.
             pursuee - Position of the character being pursued.

    Outputs: Integer representing the direction in which the NPC ought to move.
******************************************************************************/
int16_t get_pursuit_direction(const GPoint pursuer, const GPoint pursuee)
{
  int16_t diff_x                     = pursuer.x - pursuee.x,
          diff_y                     = pursuer.y - pursuee.y;
  const int16_t horizontal_direction = diff_x > 0 ? WEST  : EAST,
                vertical_direction   = diff_y > 0 ? NORTH : SOUTH;
  bool checked_horizontal_direction  = false,
       checked_vertical_direction    = false;

  // Check for alignment along the x-axis:
  if (diff_x == 0)
  {
    if (diff_y == 1 /* The two are already touching. */ ||
        occupiable(get_cell_farther_away(pursuer,
                                         vertical_direction,
                                         1)))
    {
      return vertical_direction;
    }
    checked_vertical_direction = true;
  }

  // Check for alignment along the y-axis:
  if (diff_y == 0)
  {
    if (diff_x == 1 /* The two are already touching. */ ||
        occupiable(get_cell_farther_away(pursuer,
                                         horizontal_direction,
                                         1)))
    {
      return horizontal_direction;
    }
    checked_horizontal_direction = true;
  }

  // If not aligned along either axis, a direction in either axis will do:
  while (!checked_horizontal_direction || !checked_vertical_direction)
  {
    if (checked_vertical_direction ||
        (!checked_horizontal_direction && rand() % 2))
    {
      if (occupiable(get_cell_farther_away(pursuer,
                                           horizontal_direction,
                                           1)))
      {
        return horizontal_direction;
      }
      checked_horizontal_direction = true;
    }
    if (!checked_vertical_direction)
    {
      if (occupiable(get_cell_farther_away(pursuer,
                                           vertical_direction,
                                           1)))
      {
        return vertical_direction;
      }
      checked_vertical_direction = true;
    }
  }

  // If we reach this point, the NPC is stuck in a corner. I'm okay with that:
  return horizontal_direction;
}

/******************************************************************************
   Function: touching

Description: Determines whether two cells are "touching," meaning they are next
             to each other (diagonal doesn't count).

     Inputs: cell   - First set of cell coordinates.
             cell_2 - Second set of cell coordinates.

    Outputs: "True" if the two cells are touching.
******************************************************************************/
bool touching(const GPoint cell, const GPoint cell_2)
{
  const int16_t diff_x = cell.x - cell_2.x,
                diff_y = cell.y - cell_2.y;

  return ((diff_x == 0 && abs(diff_y) == 1) ||
          (diff_y == 0 && abs(diff_x) == 1));
}

/******************************************************************************
   Function: damage_player

Description: Damages the player according to his/her defense vs. a given damage
             value.

     Inputs: damage - Potential amount of damage.

    Outputs: None.
******************************************************************************/
void damage_player(int16_t damage)
{
  damage -= g_player->stats[ARMOR] / 2;
  if (damage < MIN_DAMAGE)
  {
    damage = MIN_DAMAGE;
  }
  if (g_player->damage_vibes_on)
  {
    vibes_short_pulse();
  }
  flash_screen();
  adjust_player_current_hp(damage * -1);
}

/******************************************************************************
   Function: damage_npc

Description: Damages a given NPC according to a given damage value. If this
             reduces the NPC's HP to zero or below, the NPC's death is handled.

     Inputs: npc    - Pointer to the NPC to be damaged.
             damage - Amount of damage.

    Outputs: None.
******************************************************************************/
void damage_npc(npc_t *npc, const int16_t damage)
{
  npc->hp -= damage;
  if (npc->hp <= 0)
  {
    g_mission->kills++;
    if ((g_mission->type == ASSASSINATE && npc->type == ALIEN_OFFICER) ||
        ((g_mission->type == OBLITERATE || g_mission->type == RETALIATE) &&
         g_mission->kills >= g_mission->num_npcs))
    {
      g_mission->completed = true;
    }
    remove_npc(npc);
  }
}

/******************************************************************************
   Function: damage_cell

Description: Damages a given "solid" cell according to a given damage value. If
             this reduces the cell's HP to zero or below, the cell will become
             "empty".

     Inputs: cell   - Coordinates of the cell to be damaged.
             damage - Potential amount of damage.

    Outputs: None.
******************************************************************************/
void damage_cell(GPoint cell, const int16_t damage)
{
  if (!out_of_bounds(cell) && get_cell_type(cell) > EMPTY)
  {
    g_mission->cells[cell.x][cell.y] -= damage;
    if (get_cell_type(cell) < SOLID)
    {
      set_cell_type(cell, EMPTY);
    }
  }
}

/******************************************************************************
   Function: adjust_player_money

Description: Adjusts the player's money by a given amount, which may be
             positive or negative. If the adjustment would increase the
             player's money above MAX_LARGE_INT_VALUE, the player's money is
             simply set to MAX_LARGE_INT_VALUE and "false" is returned. If it
             would reduce the player's money below zero, no adjustment is made
             and "false" is returned.

     Inputs: amount - Adjustment amount (which may be positive or negative).

    Outputs: "True" if the adjustment succeeds.
******************************************************************************/
bool adjust_player_money(const int32_t amount)
{
  if (g_player->money + amount < 0)
  {
    return false;
  }
  else if (g_player->money + amount > MAX_LARGE_INT_VALUE)
  {
    g_player->money = MAX_LARGE_INT_VALUE;

    return false;
  }
  g_player->money += amount;

  return true;
}

/******************************************************************************
   Function: adjust_player_current_hp

Description: Adjusts the player's current hit points by a given amount, which
             may be positive or negative. Hit points may not be increased above
             the player's max. hit points nor reduced below zero. If reduced to
             zero, the player character's death is handled.

     Inputs: amount - Adjustment amount (which may be positive or negative).

    Outputs: None.
******************************************************************************/
void adjust_player_current_hp(const int16_t amount)
{
  g_player->stats[CURRENT_HP] += amount;
  if (g_player->stats[CURRENT_HP] > g_player->stats[MAX_HP])
  {
    g_player->stats[CURRENT_HP] = g_player->stats[MAX_HP];
  }
  else if (g_player->stats[CURRENT_HP] <= 0)
  {
    show_window(g_main_menu_window);
    g_current_narration = DEATH_NARRATION;
    show_narration();
  }
}

/******************************************************************************
   Function: remove_npc

Description: Handles the removal of a given NPC from memory and from the
             current mission's list of NPCs.

     Inputs: npc - Pointer to the NPC to be killed.

    Outputs: None.
******************************************************************************/
void remove_npc(npc_t *npc)
{
  npc_t *npc_pointer, *npc_pointer_2;

  npc_pointer = npc_pointer_2 = g_mission->npcs;
  while (npc_pointer != NULL)
  {
    if (npc_pointer == npc)
    {
      if (npc_pointer->next != NULL)
      {
        if (npc_pointer == g_mission->npcs)
        {
          g_mission->npcs = npc_pointer->next;
        }
        else
        {
          npc_pointer_2->next = npc_pointer->next;
        }
      }
      else if (npc_pointer == g_mission->npcs)
      {
        g_mission->npcs = NULL;
      }
      else
      {
        npc_pointer_2->next = NULL;
      }
      free(npc_pointer);

      return;
    }
    npc_pointer_2 = npc_pointer;
    npc_pointer = npc_pointer->next;
  }
}

/******************************************************************************
   Function: adjust_player_current_ammo

Description: Adjusts the player's current ammo by a given amount, which may be
             positive or negative. Ammo may not be increased above the player's
             max. ammo value nor reduced below zero.

     Inputs: amount - Adjustment amount (which may be positive or negative).

    Outputs: None.
******************************************************************************/
void adjust_player_current_ammo(const int16_t amount)
{
  g_player->stats[CURRENT_ENERGY] += amount;
  if (g_player->stats[CURRENT_ENERGY] > g_player->stats[MAX_ENERGY])
  {
    g_player->stats[CURRENT_ENERGY] = g_player->stats[MAX_ENERGY];
  }
}

/******************************************************************************
   Function: end_mission

Description: Called when the player walks into the exit, ending the current
             mission. Determines how much reward money to bestow, etc.

     Inputs: None.

    Outputs: None.
******************************************************************************/
void end_mission(void)
{
  g_game_paused = true;
  show_window(g_main_menu_window);

  if (g_mission->completed)
  {
    adjust_player_money(g_mission->reward);
    g_current_narration = MISSION_ACCOMPLISHED_NARRATION;
  }
  else
  {
    g_current_narration = MISSION_FAILED_NARRATION;
  }

  show_narration();
}

/******************************************************************************
   Function: add_new_npc

Description: Creates an NPC of a given type at a given location and adds it to
             the current mission's linked list of NPCs. (If this would exceed
             the max. number of NPCs at one time, or if the given position
             isn't occupiable, a new NPC is not created.)

     Inputs: npc_type - Desired type for the new NPC.
             position - Desired spawn point for the new NPC.

    Outputs: None.
******************************************************************************/
void add_new_npc(const int16_t npc_type, const GPoint position)
{
  int16_t count = 1;
  npc_t *npc_pointer = g_mission->npcs;

  if (occupiable(position))
  {
    if (npc_pointer == NULL)
    {
      g_mission->npcs = malloc(sizeof(npc_t));
      init_npc(g_mission->npcs, npc_type, position);

      return;
    }
    while (npc_pointer->next != NULL)
    {
      count++;
      npc_pointer = npc_pointer->next;
    }
    if (count < MAX_NPCS_AT_ONE_TIME)
    {
      npc_pointer->next = malloc(sizeof(npc_t));
      init_npc(npc_pointer->next, npc_type, position);
    }
  }
}

/******************************************************************************
   Function: get_random_npc_type

Description: Returns a random NPC type, excluding the ALIEN_OFFICER type which
             is not meant to appear randomly.

     Inputs: None.

    Outputs: Integer representing an NPC type.
******************************************************************************/
int16_t get_random_npc_type(void)
{
  return rand() % NUM_RANDOM_NPC_TYPES;
}

/******************************************************************************
   Function: get_npc_spawn_point

Description: Returns a suitable NPC spawn point, outside the player's sphere of
             visibility. If the algorithm fails to find one, (-1, -1) is
             returned instead.

     Inputs: None.

    Outputs: The coordinates of a suitable NPC spawn point, or (-1, -1) if the
             algorithm fails to find one.
******************************************************************************/
GPoint get_npc_spawn_point(void)
{
  int16_t i, j, direction;
  bool checked_left, checked_right;
  GPoint spawn_point, spawn_point2;

  for (i = 0, direction = rand() % NUM_DIRECTIONS;
       i < NUM_DIRECTIONS;
       ++i, direction = (direction + 1 == NUM_DIRECTIONS ? NORTH :
                                                           direction + 1))
  {
    spawn_point = get_cell_farther_away(g_player->position,
                                        direction,
                                        MAX_VISIBILITY_DEPTH);
    if (out_of_bounds(spawn_point))
    {
      continue;
    }
    if (occupiable(spawn_point))
    {
      return spawn_point;
    }
    for (j = 1; j < MAX_VISIBILITY_DEPTH - 1; ++j)
    {
      checked_left = checked_right = false;
      do
      {
        // Check to the left:
        if (checked_right || rand() % 2)
        {
          spawn_point2 = get_cell_farther_away(spawn_point,
                                          get_direction_to_the_left(direction),
                                          j);
          checked_left = true;
        }
        // Check to the right:
        else if (!checked_right)
        {
          spawn_point2 = get_cell_farther_away(spawn_point,
                                         get_direction_to_the_right(direction),
                                         j);
          checked_right = true;
        }
        if (occupiable(spawn_point2))
        {
          return spawn_point2;
        }
      } while (!checked_left || !checked_right);
    }
  }

  return GPoint(-1, -1);
}

/******************************************************************************
   Function: get_floor_center_point

Description: Returns the central point, with respect to the graphics layer, of
             the floor of the cell at a given visual depth and position.

     Inputs: depth    - Front-back visual depth in "g_back_wall_coords".
             position - Left-right visual position in "g_back_wall_coords".

    Outputs: GPoint coordinates of the floor's central point within the
             designated cell.
******************************************************************************/
GPoint get_floor_center_point(const int16_t depth, const int16_t position)
{
  int16_t x_midpoint1, x_midpoint2, x, y;

  x_midpoint1 = 0.5 * (g_back_wall_coords[depth][position][TOP_LEFT].x +
                       g_back_wall_coords[depth][position][BOTTOM_RIGHT].x);
  if (depth == 0)
  {
    if (position < STRAIGHT_AHEAD)      // Just to the left of the player.
    {
      x_midpoint2 = -0.5 * GRAPHICS_FRAME_WIDTH;
    }
    else if (position > STRAIGHT_AHEAD) // Just to the right of the player.
    {
      x_midpoint2 = 1.5 * GRAPHICS_FRAME_WIDTH;
    }
    else                                // Directly under the player.
    {
      x_midpoint2 = x_midpoint1;
    }
    y = GRAPHICS_FRAME_HEIGHT;
  }
  else
  {
    x_midpoint2 = 0.5 *
      (g_back_wall_coords[depth - 1][position][TOP_LEFT].x +
       g_back_wall_coords[depth - 1][position][BOTTOM_RIGHT].x);
    y = 0.5 * (g_back_wall_coords[depth][position][BOTTOM_RIGHT].y +
               g_back_wall_coords[depth - 1][position][BOTTOM_RIGHT].y);
  }
  x = 0.5 * (x_midpoint1 + x_midpoint2);

  return GPoint(x, y);
}

/******************************************************************************
   Function: get_cell_farther_away

Description: Given a set of cell coordinates, returns new cell coordinates a
             given distance farther away in a given direction. (These may lie
             out-of-bounds.)

     Inputs: reference_point - Reference cell coordinates.
             direction       - Direction of interest.
             distance        - How far back we want to go.

    Outputs: Cell coordinates a given distance farther away from those passed
             in. (These may lie out-of-bounds.)
******************************************************************************/
GPoint get_cell_farther_away(const GPoint reference_point,
                             const int16_t direction,
                             const int16_t distance)
{
  switch (direction)
  {
    case NORTH:
      return GPoint(reference_point.x, reference_point.y - distance);
    case SOUTH:
      return GPoint(reference_point.x, reference_point.y + distance);
    case EAST:
      return GPoint(reference_point.x + distance, reference_point.y);
    default: // case WEST:
      return GPoint(reference_point.x - distance, reference_point.y);
  }
}

/******************************************************************************
   Function: get_direction_to_the_left

Description: Given a north/south/east/west reference direction, returns the
             direction to its left.

     Inputs: reference_direction - Direction from which to turn left.

    Outputs: Integer representing the direction to the left of the reference
             direction.
******************************************************************************/
int16_t get_direction_to_the_left(const int16_t reference_direction)
{
  switch (reference_direction)
  {
    case NORTH:
      return WEST;
    case WEST:
      return SOUTH;
    case SOUTH:
      return EAST;
    default: // case EAST:
      return NORTH;
  }
}

/******************************************************************************
   Function: get_direction_to_the_right

Description: Given a north/south/east/west reference direction, returns the
             direction to its right.

     Inputs: reference_direction - Direction from which to turn right.

    Outputs: Integer representing the direction to the right of the reference
             direction.
******************************************************************************/
int16_t get_direction_to_the_right(const int16_t reference_direction)
{
  switch (reference_direction)
  {
    case NORTH:
      return EAST;
    case EAST:
      return SOUTH;
    case SOUTH:
      return WEST;
    default: // case WEST:
      return NORTH;
  }
}

/******************************************************************************
   Function: get_cell_type

Description: Returns the type of cell at a given set of coordinates.

     Inputs: cell - Coordinates of the cell of interest.

    Outputs: The indicated cell's type.
******************************************************************************/
int16_t get_cell_type(const GPoint cell)
{
  if (out_of_bounds(cell))
  {
    return SOLID;
  }

  return g_mission->cells[cell.x][cell.y];
}

/******************************************************************************
   Function: set_cell_type

Description: Sets the cell at a given set of coordinates to a given type.
             (Doesn't test coordinates to ensure they're in-bounds!)

     Inputs: cell - Coordinates of the cell of interest.
             type - The cell type to be assigned at those coordinates.

    Outputs: None.
******************************************************************************/
void set_cell_type(GPoint cell, const int16_t type)
{
  g_mission->cells[cell.x][cell.y] = type;
}

/******************************************************************************
   Function: get_npc_at

Description: Returns a pointer to the NPC occupying a given cell.

     Inputs: cell - Coordinates of the cell of interest.

    Outputs: Pointer to the NPC occupying the indicated cell, or NULL if there
             is none.
******************************************************************************/
npc_t *get_npc_at(const GPoint cell)
{
  npc_t *npc = g_mission->npcs;

  while (npc != NULL)
  {
    if (gpoint_equal(&npc->position, &cell))
    {
      return npc;
    }
    npc = npc->next;
  }

  return NULL;
}

/******************************************************************************
   Function: out_of_bounds

Description: Determines whether a given set of cell coordinates lies outside
             the current location boundaries.

     Inputs: cell - Coordinates of the cell of interest.

    Outputs: "True" if the cell is out of bounds.
******************************************************************************/
bool out_of_bounds(const GPoint cell)
{
  return cell.x < 0               ||
         cell.x >= LOCATION_WIDTH ||
         cell.y < 0               ||
         cell.y >= LOCATION_HEIGHT;
}

/******************************************************************************
   Function: occupiable

Description: Determines whether the cell at a given set of coordinates may be
             occupied by a game character (i.e., it's within the location's
             boundaries, non-solid, not already occupied by another character,
             etc.).

     Inputs: cell - Coordinates of the cell of interest.

    Outputs: "True" if the cell is occupiable.
******************************************************************************/
bool occupiable(const GPoint cell)
{
  return get_cell_type(cell) <= EMPTY              &&
         !gpoint_equal(&g_player->position, &cell) &&
         get_npc_at(cell) == NULL;
}

/******************************************************************************
   Function: show_narration

Description: Displays narration text via the narration window according to
             "g_current_narration".

     Inputs: None.

    Outputs: None.
******************************************************************************/
void show_narration(void)
{
  static char narration_str[NARRATION_STR_LEN + 1];

  switch (g_current_narration)
  {
    case RETALIATE: // Max. 80 chars
      strcpy(narration_str, "One of our ");
      strcat_location_name(narration_str, g_mission->location_type, PLURAL);
      strcat(narration_str, " has been overrun by ");
      strcat_npc_name(narration_str, g_mission->primary_npc_type, PLURAL);
      strcat(narration_str, ": kill all ");
      strcat_int(narration_str, g_mission->num_npcs);
      strcat(narration_str, " for $");
      strcat_int(narration_str, g_mission->reward);
      break;
    case OBLITERATE: // Max. 56 chars
      strcpy(narration_str, "Eliminate all ");
      strcat_int(narration_str, g_mission->num_npcs);
      strcat(narration_str, " Fim from this ");
      strcat_location_name(narration_str, g_mission->location_type, SINGULAR);
      strcat(narration_str, " for $");
      strcat_int(narration_str, g_mission->reward);
      break;
    case EXPROPRIATE: // Max. 67 chars
      strcpy(narration_str, "Steal a data storage device from this Fim ");
      strcat_location_name(narration_str, g_mission->location_type, SINGULAR);
      strcat(narration_str, " for $");
      strcat_int(narration_str, g_mission->reward);
      break;
    case EXTRICATE: // Max. 66 chars
      strcpy(narration_str, "Rescue one of our officers from this Fim prison "
                            "to receive $");
      strcat_int(narration_str, g_mission->reward);
      break;
    case ASSASSINATE: // Max. 59 chars
      strcpy(narration_str, "Neutralize the leader of this Fim "
      strcat_location_name(narration_str, g_mission->location_type, SINGULAR);
      strcat(narration_str, " for $");
      strcat_int(narration_str, g_mission->reward);
      break;
    case DEATH_NARRATION:
      strcpy(narration_str, "You fell in battle, but your body was found and "
                            "resuscitated. Soldier on!");
      deinit_mission();
      break;
    case MISSION_FAILED_NARRATION:
    case MISSION_ACCOMPLISHED_NARRATION:
      strcpy(narration_str, "Mission ");
      g_current_narration == MISSION_ACCOMPLISHED_NARRATION ?
        strcpy(narration_str, "C") :
        strcpy(narration_str, "Inc");
      strcpy(narration_str, "omplete\n\nKills: ");
      strcat_int(narration_str, g_mission->kills);
      strcat(narration_str, "\nEnemies Remaining: ");
      strcat_int(narration_str,  g_mission->num_npcs - g_mission->kills);
      strcat(narration_str, "\nReward: $");
      g_current_narration == MISSION_ACCOMPLISHED_NARRATION ?
        strcat_int(narration_str, g_mission->reward) :
        strcpy(narration_str, "0");
      deinit_mission();
      break;
    case CONTROLS_NARRATION:
      strcpy(narration_str, "Forward: \"Up\"\nBack: \"Down\"\nLeft: \"Up\" x"
                            " 2\nRight: \"Down\" x 2\nShoot: \"Select\"");
      break;
    case GAME_INFO_NARRATION:
      strcpy(narration_str, "SpaceMerc was designed and programmed by "
                            "David C. Drake:\n\ndavidcdrake.com");
      break;
    case INTRO_NARRATION_1:
      strcpy(narration_str, "Humankind is at war with a hostile alien race "
                            "known as the Fim.");
      break;
    case INTRO_NARRATION_2:
      strcpy(narration_str, "As an elite interstellar mercenary, your skills "
                            "are in high demand.");
      break;
    default: // case INTRO_NARRATION_3:
      strcpy(narration_str, "Fame and fortune await as you risk life and limb "
                            "for humanity's future!");
      break;
  }
  text_layer_set_text(g_narration_text_layer, narration_str);
  show_window(g_narration_window);
}

/******************************************************************************
   Function: show_window

Description: Displays a given window. (Assumes that window has already been
             initialized.)

     Inputs: window - Pointer to the desired window.

    Outputs: None.
******************************************************************************/
void show_window(Window *window)
{
  if (!window_stack_contains_window(window))
  {
    window_stack_push(window, NOT_ANIMATED);
  }
  else
  {
    while (window_stack_get_top_window() != window)
    {
      window_stack_pop(NOT_ANIMATED);
    }
  }
}

/******************************************************************************
   Function: main_menu_draw_row_callback

Description: Instructions for drawing each row of the main menu.

     Inputs: ctx        - Pointer to the associated context.
             cell_layer - Pointer to the layer of the cell to be drawn.
             cell_index - Pointer to the index struct of the cell to be drawn.
             data       - Pointer to additional data (not used).

    Outputs: None.
******************************************************************************/
static void main_menu_draw_row_callback(GContext *ctx,
                                        const Layer *cell_layer,
                                        MenuIndex *cell_index,
                                        void *data)
{
  switch (cell_index->row)
  {
    case 0:
      menu_cell_basic_draw(ctx,
                           cell_layer,
                           g_mission == NULL ? "New Mission" :
                                               "Continue",
                           "Grab your gun and go!",
                           NULL);
      break;
    case 1:
      menu_cell_basic_draw(ctx,
                           cell_layer,
                           "Buy an Upgrade",
                           g_mission == NULL ? "Improved armor, etc." :
                                               "Not during missions!",
                           NULL);
      break;
    case 2:
      menu_cell_basic_draw(ctx,
                           cell_layer,
                           "Controls",
                           "How to play.",
                           NULL);
      break;
    case 3:
      menu_cell_basic_draw(ctx,
                           cell_layer,
                           "About",
                           "Credits.",
                           NULL);
      break;
    default:
      menu_cell_basic_draw(ctx,
                           cell_layer,
                           g_player->damage_vibes_on ? "Damage Vibes On" :
                                                       "Damage Vibes Off",
                           "Vibrate when hit?",
                           NULL);
      break;
  }
}

/******************************************************************************
   Function: main_menu_select_callback

Description: Called when a given cell of the main menu is selected.

     Inputs: menu_layer - Pointer to the menu layer.
             cell_index - Pointer to the index struct of the selected cell.
             data       - Pointer to additional data (not used).

    Outputs: None.
******************************************************************************/
void main_menu_select_callback(MenuLayer *menu_layer,
                               MenuIndex *cell_index,
                               void *data)
{
  switch (cell_index->row)
  {
    case 0: // New Mission / Continue
      if (g_mission == NULL)
      {
        g_mission = malloc(sizeof(mission_t));
        init_mission(rand() % NUM_MISSION_TYPES);
        break;
      }
      show_window(g_graphics_window);
      break;
    case 1: // Buy an Upgrade
      if (g_mission == NULL)
      {
        menu_layer_set_selected_index(g_upgrade_menu,
                                      (MenuIndex) {0, 0},
                                      MenuRowAlignCenter,
                                      NOT_ANIMATED);
        show_window(g_upgrade_menu_window);
      }
      break;
    case 2: // Controls
      g_current_narration = CONTROLS_NARRATION;
      show_narration();
      break;
    case 3: // About
      g_current_narration = GAME_INFO_NARRATION;
      show_narration();
      break;
    default: // Vibrations On/Off
      g_player->damage_vibes_on = !g_player->damage_vibes_on;
      menu_layer_reload_data(menu_layer);
      break;
  }
}

/******************************************************************************
   Function: upgrade_menu_draw_header_callback

Description: Instructions for drawing the upgrade menu's header.

     Inputs: ctx           - Pointer to the associated context.
             cell_layer    - Pointer to the cell layer.
             section_index - Section number of the header to be drawn.
             data          - Pointer to additional data (not used).

    Outputs: None.
******************************************************************************/
static void upgrade_menu_draw_header_callback(GContext *ctx,
                                              const Layer *cell_layer,
                                              uint16_t section_index,
                                              void *data)
{
  char header_str[UPGRADE_MENU_HEADER_STR_LEN + 1];

  strcpy(header_str, "Funds: $");
  strcat_int(header_str, g_player->money);
  menu_cell_basic_header_draw(ctx, cell_layer, header_str);
}

/******************************************************************************
   Function: upgrade_menu_draw_row_callback

Description: Instructions for drawing a given row of the upgrade menu.

     Inputs: ctx        - Pointer to the associated context.
             cell_layer - Pointer to the layer of the cell to be drawn.
             cell_index - Pointer to the index struct of the cell to be drawn.
             data       - Pointer to additional data (not used).

    Outputs: None.
******************************************************************************/
static void upgrade_menu_draw_row_callback(GContext *ctx,
                                           const Layer *cell_layer,
                                           MenuIndex *cell_index,
                                           void *data)
{
  int16_t new_stat_value;
  char title_str[UPGRADE_TITLE_STR_LEN + 1],
       subtitle_str[UPGRADE_SUBTITLE_STR_LEN + 1];

  // Determine the upgrade's title:
  switch (cell_index->row)
  {
    case ARMOR:
      strcpy(title_str, "Armor");
      break;
    case MAX_HP:
      strcpy(title_str, "Max. Health");
      break;
    case POWER:
      strcpy(title_str, "Laser Power");
      break;
    default: // case MAX_ENERGY:
      strcpy(title_str, "Max. Energy");
      break;
  }

  // Determine the upgrade's subtitle:
  if (g_player->stats[cell_index->row] >= MAX_SMALL_INT_VALUE)
  {
    strcpy(subtitle_str, "9999 (Maxed Out)");
  }
  else
  {
    strcpy(subtitle_str, "");
    strcat_int(subtitle_str, g_player->stats[cell_index->row]);
    strcat(subtitle_str, "->");
    new_stat_value = get_upgraded_stat_value(cell_index->row);
    strcat_int(subtitle_str, new_stat_value);
    strcat(subtitle_str, " $");
    strcat_int(subtitle_str, get_upgrade_cost(new_stat_value));
  }

  // Finally, draw the upgrade's row in the upgrade menu:
  menu_cell_basic_draw(ctx, cell_layer, title_str, subtitle_str, NULL);
}

/******************************************************************************
   Function: upgrade_menu_select_callback

Description: Called when a given cell of the upgrade menu is selected.

     Inputs: menu_layer - Pointer to the menu layer.
             cell_index - Pointer to the index struct of the selected cell.
             data       - Pointer to additional data (not used).

    Outputs: None.
******************************************************************************/
void upgrade_menu_select_callback(MenuLayer *menu_layer,
                                  MenuIndex *cell_index,
                                  void *data)
{
  int16_t new_stat_value;

  if (g_player->stats[cell_index->row] >= MAX_SMALL_INT_VALUE)
  {
    return;
  }

  new_stat_value = get_upgraded_stat_value(cell_index->row);
  if (adjust_player_money(get_upgrade_cost(new_stat_value) * -1))
  {
    g_player->stats[cell_index->row] = new_stat_value;
    menu_layer_reload_data(g_upgrade_menu);
  }
}

/******************************************************************************
   Function: get_upgraded_stat_value

Description: Determines what value a given stat will be raised to if the player
             purchases an upgrade for that stat.

     Inputs: stat_index - Index value of the stat of interest.

    Outputs: The new value the stat will have if it is upgraded.
******************************************************************************/
int16_t get_upgraded_stat_value(const int16_t stat_index)
{
  int16_t upgraded_stat_value = g_player->stats[stat_index] +
                                STAT_BOOST_PER_UPGRADE;

  if (upgraded_stat_value >= MAX_SMALL_INT_VALUE)
  {
    return MAX_SMALL_INT_VALUE;
  }

  return upgraded_stat_value;
}

/******************************************************************************
   Function: get_upgrade_cost

Description: Determines the cost for upgrading one of the player's stats to a
             given value.

     Inputs: upgraded_stat_value - The new value the stat will have if an
                                   upgrade is purchased.

    Outputs: The cost for upgrading a stat to the given value.
******************************************************************************/
int32_t get_upgrade_cost(const int16_t upgraded_stat_value)
{
  int32_t cost = upgraded_stat_value * UPGRADE_COST_MULTIPLIER;

  if (cost >= MAX_LARGE_INT_VALUE || cost < upgraded_stat_value)
  {
    return MAX_LARGE_INT_VALUE;
  }

  return cost;
}

/******************************************************************************
   Function: menu_get_num_sections_callback

Description: Returns the number of sections in a given menu.

     Inputs: menu_layer - Pointer to the menu of interest.
             data       - Pointer to additional data (not used).

    Outputs: The number of sections in the indicated menu.
******************************************************************************/
static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer,
                                               void *data)
{
  return 1;
}

/******************************************************************************
   Function: menu_get_header_height_callback

Description: Returns the section height for a given section of a given menu.

     Inputs: menu_layer    - Pointer to the menu of interest.
             section_index - Section number.
             data          - Pointer to additional data (not used).

    Outputs: The number of sections in the indicated menu.
******************************************************************************/
static int16_t menu_get_header_height_callback(MenuLayer *menu_layer,
                                               uint16_t section_index,
                                               void *data)
{
  return MENU_CELL_BASIC_HEADER_HEIGHT;
}

/******************************************************************************
   Function: menu_get_num_rows_callback

Description: Returns the number of rows in a given menu (or in a given section
             of a given menu).

     Inputs: menu_layer    - Pointer to the menu of interest.
             section_index - Section number.
             data          - Pointer to additional data (not used).

    Outputs: The number of rows in the indicated menu.
******************************************************************************/
static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer,
                                           uint16_t section_index,
                                           void *data)
{
  if (menu_layer == g_main_menu)
  {
    return MAIN_MENU_NUM_ROWS;
  }
  else // menu_layer == g_upgrade_menu
  {
    return UPGRADE_MENU_NUM_ROWS;
  }
}

/******************************************************************************
   Function: draw_scene

Description: Draws a (simplistic) 3D scene based on the player's current
             position, direction, and visibility depth.

     Inputs: layer - Pointer to the relevant layer.
             ctx   - Pointer to the relevant graphics context.

    Outputs: None.
******************************************************************************/
void draw_scene(Layer *layer, GContext *ctx)
{
  int16_t i, depth;
  GPoint cell, cell_2;

  // First, draw the background, floor, and ceiling:
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx,
                     layer_get_bounds(layer),
                     NO_CORNER_RADIUS,
                     GCornerNone);
  draw_floor_and_ceiling(ctx);

  // Now draw walls and cell contents:
  for (depth = MAX_VISIBILITY_DEPTH - 2; depth >= 0; --depth)
  {
    // Straight ahead at the current depth:
    cell = get_cell_farther_away(g_player->position,
                                 g_player->direction,
                                 depth);
    if (out_of_bounds(cell))
    {
      continue;
    }
    if (get_cell_type(cell) < SOLID)
    {
      draw_cell_walls(ctx, cell, depth, STRAIGHT_AHEAD);
      draw_cell_contents(ctx, cell, depth, STRAIGHT_AHEAD);
    }

    // To the left and right at the same depth:
    for (i = depth + 1; i > 0; --i)
    {
      cell_2 = get_cell_farther_away(cell,
                                get_direction_to_the_left(g_player->direction),
                                i);
      if (get_cell_type(cell_2) < SOLID)
      {
        draw_cell_walls(ctx, cell_2, depth, STRAIGHT_AHEAD - i);
        draw_cell_contents(ctx, cell_2, depth, STRAIGHT_AHEAD - i);
      }
      cell_2 = get_cell_farther_away(cell,
                               get_direction_to_the_right(g_player->direction),
                               i);
      if (get_cell_type(cell_2) < SOLID)
      {
        draw_cell_walls(ctx, cell_2, depth, STRAIGHT_AHEAD + i);
        draw_cell_contents(ctx, cell_2, depth, STRAIGHT_AHEAD + i);
      }
    }
  }

  // Draw applicable weapon fire:
  if (g_player_animation_mode > 0)
  {
    draw_player_laser_beam(ctx);
  }

  // Finally, draw the lower status bar:
  draw_status_bar(ctx);
}

/******************************************************************************
   Function: draw_player_laser_beam

Description: Draws the player's laser beam onto the screen.

     Inputs: ctx - Pointer to the relevant graphics context.

    Outputs: None.
******************************************************************************/
void draw_player_laser_beam(GContext *ctx)
{
  int16_t i;

  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_draw_line(ctx,
                     GPoint(SCREEN_CENTER_POINT_X, GRAPHICS_FRAME_HEIGHT),
                     SCREEN_CENTER_POINT);
  for (i = 0; i <= g_laser_base_width / 2; ++i)
  {
    if (i == g_laser_base_width / 2)
    {
      graphics_context_set_stroke_color(ctx, GColorBlack);
    }
    graphics_draw_line(ctx,
                      GPoint(SCREEN_CENTER_POINT_X - i, GRAPHICS_FRAME_HEIGHT),
                      GPoint(SCREEN_CENTER_POINT_X - i / 3,
                             SCREEN_CENTER_POINT_Y));
    graphics_draw_line(ctx,
                      GPoint(SCREEN_CENTER_POINT_X + i, GRAPHICS_FRAME_HEIGHT),
                      GPoint(SCREEN_CENTER_POINT_X + i / 3,
                             SCREEN_CENTER_POINT_Y));
  }
}

/******************************************************************************
   Function: draw_floor_and_ceiling

Description: Draws the floor and ceiling.

     Inputs: ctx - Pointer to the relevant graphics context.

    Outputs: None.
******************************************************************************/
void draw_floor_and_ceiling(GContext *ctx)
{
  int16_t x, y, max_y, shading_offset;

  x = 2;
  max_y = g_back_wall_coords[MAX_VISIBILITY_DEPTH - x]
                            [STRAIGHT_AHEAD]
                            [TOP_LEFT].y;
  while (max_y > GRAPHICS_FRAME_HEIGHT / 2 - MIN_WALL_HEIGHT / 2 &&
         x <= MAX_VISIBILITY_DEPTH)
  {
    max_y = g_back_wall_coords[MAX_VISIBILITY_DEPTH - ++x]
                              [STRAIGHT_AHEAD]
                              [TOP_LEFT].y;
  }
  graphics_context_set_stroke_color(ctx, GColorWhite);
  for (y = 0; y < max_y; ++y)
  {
    // Determine horizontal distance between points:
    shading_offset = 1 + y / MAX_VISIBILITY_DEPTH;
    if (y % MAX_VISIBILITY_DEPTH >= MAX_VISIBILITY_DEPTH / 2 +
                                    MAX_VISIBILITY_DEPTH % 2)
    {
      shading_offset++;
    }
    for (x = y % 2 ? 0 : (shading_offset / 2) + (shading_offset % 2);
         x < GRAPHICS_FRAME_WIDTH;
         x += shading_offset)
    {
      // Draw a point on the ceiling:
      graphics_draw_pixel(ctx, GPoint(x, y));

      // Draw a point on the floor:
      graphics_draw_pixel(ctx, GPoint(x, GRAPHICS_FRAME_HEIGHT - y));
    }
  }
}

/******************************************************************************
   Function: draw_cell_walls

Description: Draws any walls that exist along the back and sides of a given
             cell.

     Inputs: ctx      - Pointer to the relevant graphics context.
             cell     - Coordinates of the cell of interest.
             depth    - Front-back visual depth of the cell of interest in
                        "g_back_wall_coords".
             position - Left-right visual position of the cell of interest in
                        "g_back_wall_coords".

    Outputs: None.
******************************************************************************/
void draw_cell_walls(GContext *ctx,
                     const GPoint cell,
                     const int16_t depth,
                     const int16_t position)
{
  int16_t left, right, top, bottom, y_offset, exit_offset_x, exit_offset_y;
  bool back_wall_drawn, left_wall_drawn, right_wall_drawn, exit_present;
  GPoint cell_2;

  // Back wall:
  left          = g_back_wall_coords[depth][position][TOP_LEFT].x;
  right         = g_back_wall_coords[depth][position][BOTTOM_RIGHT].x;
  top           = g_back_wall_coords[depth][position][TOP_LEFT].y;
  bottom        = g_back_wall_coords[depth][position][BOTTOM_RIGHT].y;
  exit_present  = gpoint_equal(&cell, &g_mission->starting_point);
  exit_offset_y = (right - left) / 4;
  if (bottom - top < MIN_WALL_HEIGHT)
  {
    return;
  }
  back_wall_drawn = left_wall_drawn = right_wall_drawn = false;
  cell_2 = get_cell_farther_away(cell, g_player->direction, 1);
  if (get_cell_type(cell_2) >= SOLID)
  {
    draw_shaded_quad(ctx,
                     GPoint(left, top),
                     GPoint(left, bottom),
                     GPoint(right, top),
                     GPoint(right, bottom),
                     GPoint(left, top));
    graphics_context_set_stroke_color(ctx, GColorBlack);
    graphics_draw_line(ctx, GPoint(left, top), GPoint(right, top));
    graphics_draw_line(ctx, GPoint(left, bottom), GPoint(right, bottom));

    // Ad hoc solution to a minor visual issue (remove if no longer relevant):
    if (top == g_back_wall_coords[1][0][TOP_LEFT].y)
    {
      graphics_draw_line(ctx,
                         GPoint(left, bottom + 1),
                         GPoint(right, bottom + 1));
    }

    // Entrance/exit:
    if (exit_present && g_player->direction == g_mission->entrance_direction)
    {
      graphics_context_set_fill_color(ctx, GColorBlack);
      exit_offset_x = (right - left) / 3;
      graphics_fill_rect(ctx,
                         GRect(left + exit_offset_x,
                               top + exit_offset_y,
                               exit_offset_x,
                               bottom - top - exit_offset_y),
                         NO_CORNER_RADIUS,
                         GCornerNone);
    }

    back_wall_drawn = true;
  }

  // Left wall:
  right = left;
  if (depth == 0)
  {
    left = 0;
    y_offset = top;
  }
  else
  {
    left = g_back_wall_coords[depth - 1][position][TOP_LEFT].x;
    y_offset = top - g_back_wall_coords[depth - 1][position][TOP_LEFT].y;
  }
  if (position <= STRAIGHT_AHEAD)
  {
    cell_2 = get_cell_farther_away(cell,
                                get_direction_to_the_left(g_player->direction),
                                1);
    if (get_cell_type(cell_2) >= SOLID)
    {
      draw_shaded_quad(ctx,
                       GPoint(left, top - y_offset),
                       GPoint(left, bottom + y_offset),
                       GPoint(right, top),
                       GPoint(right, bottom),
                       GPoint(left, top - y_offset));
      graphics_context_set_stroke_color(ctx, GColorBlack);
      graphics_draw_line(ctx,
                         GPoint(left, top - y_offset),
                         GPoint(right, top));
      graphics_draw_line(ctx,
                         GPoint(left, bottom + y_offset),
                         GPoint(right, bottom));

      // Entrance/exit:
      if (exit_present && get_direction_to_the_left(g_player->direction) ==
                          g_mission->entrance_direction)
      {
        exit_offset_x = (right - left) / 3;
        fill_quad(ctx,
                  GPoint(depth == 0 ? 0 : left + exit_offset_x,
                         top - (depth == 0 ? y_offset - 4: y_offset / 3) +
                           exit_offset_y),
                  GPoint(depth == 0 ? 0 : left + exit_offset_x,
                         bottom + (depth == 0 ? y_offset : y_offset / 3)),
                  GPoint(right - exit_offset_x, top + exit_offset_y),
                  GPoint(right - exit_offset_x, bottom + 3),
                  GColorBlack);
      }

      left_wall_drawn = true;
    }
  }

  // Right wall:
  left = g_back_wall_coords[depth][position][BOTTOM_RIGHT].x;
  if (depth == 0)
  {
    right = GRAPHICS_FRAME_WIDTH - 1;
  }
  else
  {
    right = g_back_wall_coords[depth - 1][position][BOTTOM_RIGHT].x;
  }
  if (position >= STRAIGHT_AHEAD)
  {
    cell_2 = get_cell_farther_away(cell,
                               get_direction_to_the_right(g_player->direction),
                               1);
    if (get_cell_type(cell_2) >= SOLID)
    {
      draw_shaded_quad(ctx,
                       GPoint(left, top),
                       GPoint(left, bottom),
                       GPoint(right, top - y_offset),
                       GPoint(right, bottom + y_offset),
                       GPoint(left, top));
      graphics_context_set_stroke_color(ctx, GColorBlack);
      graphics_draw_line(ctx,
                         GPoint(left, top),
                         GPoint(right, top - y_offset));
      graphics_draw_line(ctx,
                         GPoint(left, bottom),
                         GPoint(right, bottom + y_offset));

      // Entrance/exit:
      if (exit_present && get_direction_to_the_right(g_player->direction) ==
                          g_mission->entrance_direction)
      {
        exit_offset_x = (right - left) / 3;
        fill_quad(ctx,
                  GPoint(left + exit_offset_x, top + exit_offset_y),
                  GPoint(left + exit_offset_x, bottom + 4),
                  GPoint(depth == 0 ? GRAPHICS_FRAME_WIDTH :
                                      right - exit_offset_x,
                         top - (depth == 0 ? y_offset - 5 : y_offset / 3) +
                           exit_offset_y),
                  GPoint(depth == 0 ? GRAPHICS_FRAME_WIDTH :
                                      right - exit_offset_x,
                         bottom + (depth == 0 ? y_offset : y_offset / 3)),
                  GColorBlack);
      }

      right_wall_drawn = true;
    }
  }

  // Draw vertical lines at corners:
  graphics_context_set_stroke_color(ctx, GColorBlack);
  cell_2 = get_cell_farther_away(cell, g_player->direction, 1);
  if ((back_wall_drawn && (left_wall_drawn ||
       get_cell_type(get_cell_farther_away(cell_2,
                                get_direction_to_the_left(g_player->direction),
                                1)) < SOLID)) ||
      (left_wall_drawn &&
       get_cell_type(get_cell_farther_away(cell_2,
                                get_direction_to_the_left(g_player->direction),
                                1)) < SOLID))
  {
    graphics_draw_line(ctx,
                       g_back_wall_coords[depth][position][TOP_LEFT],
                       GPoint(g_back_wall_coords[depth][position][TOP_LEFT].x,
                         g_back_wall_coords[depth][position][BOTTOM_RIGHT].y));
  }
  if ((back_wall_drawn && (right_wall_drawn ||
       get_cell_type(get_cell_farther_away(cell_2,
                               get_direction_to_the_right(g_player->direction),
                               1)) < SOLID)) ||
      (right_wall_drawn &&
       get_cell_type(get_cell_farther_away(cell_2,
                               get_direction_to_the_right(g_player->direction),
                               1)) < SOLID))
  {
    graphics_draw_line(ctx,
                    g_back_wall_coords[depth][position][BOTTOM_RIGHT],
                    GPoint(g_back_wall_coords[depth][position][BOTTOM_RIGHT].x,
                           g_back_wall_coords[depth][position][TOP_LEFT].y));
  }
}

/******************************************************************************
   Function: draw_cell_contents

Description: Draws an NPC or any other contents present in a given cell.

     Inputs: ctx      - Pointer to the relevant graphics context.
             cell     - Coordinates of the cell of interest.
             depth    - Front-back visual depth of the cell of interest in
                        "g_back_wall_coords".
             position - Left-right visual position of the cell of interest in
                        "g_back_wall_coords".

    Outputs: None.
******************************************************************************/
void draw_cell_contents(GContext *ctx,
                        const GPoint cell,
                        const int16_t depth,
                        const int16_t position)
{
  int16_t drawing_unit, // Reference variable for drawing contents at depth.
          content_type = get_cell_type(cell);
  GPoint floor_center_point;

  if (content_type == EMPTY)
  {
    if (get_npc_at(cell) == NULL)
    {
      return;
    }
    content_type = get_npc_at(cell)->type;
  }

  // Determine the drawing unit:
  drawing_unit = (g_back_wall_coords[depth][position][BOTTOM_RIGHT].x -
                  g_back_wall_coords[depth][position][TOP_LEFT].x) / 10;
  if ((g_back_wall_coords[depth][position][BOTTOM_RIGHT].x -
       g_back_wall_coords[depth][position][TOP_LEFT].x) % 10 >= 5)
  {
    drawing_unit++;
  }
  floor_center_point = get_floor_center_point(depth, position);

  // Draw a shadow on the ground:
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx,
                     GRect(floor_center_point.x - drawing_unit * 4,
                           floor_center_point.y - drawing_unit / 2,
                           drawing_unit * 8,
                           drawing_unit),
                     drawing_unit / 2,
                     GCornersAll);

  // Now draw the body, etc.:
  if (content_type == ALIEN_SOLDIER ||
      content_type == ALIEN_ELITE   ||
      content_type == ALIEN_OFFICER)
  {
    // Legs:
    draw_shaded_quad(ctx,
                     GPoint(floor_center_point.x - drawing_unit * 2,
                            floor_center_point.y - drawing_unit * 3),
                     GPoint(floor_center_point.x - drawing_unit * 2,
                            floor_center_point.y),
                     GPoint(floor_center_point.x - drawing_unit,
                            floor_center_point.y - drawing_unit * 3),
                     GPoint(floor_center_point.x - drawing_unit,
                            floor_center_point.y),
                     GPoint(g_back_wall_coords[depth][position][TOP_LEFT].x +
                              4,
                            g_back_wall_coords[depth][position][TOP_LEFT].y +
                              4));
    draw_shaded_quad(ctx,
                     GPoint(floor_center_point.x + drawing_unit,
                            floor_center_point.y - drawing_unit * 3),
                     GPoint(floor_center_point.x + drawing_unit,
                            floor_center_point.y),
                     GPoint(floor_center_point.x + drawing_unit * 2,
                            floor_center_point.y - drawing_unit * 3),
                     GPoint(floor_center_point.x + drawing_unit * 2,
                            floor_center_point.y),
                     GPoint(g_back_wall_coords[depth][position][TOP_LEFT].x +
                              4,
                            g_back_wall_coords[depth][position][TOP_LEFT].y +
                              4));

    // Waist:
    draw_shaded_quad(ctx,
                     GPoint(floor_center_point.x - drawing_unit * 2,
                            floor_center_point.y - drawing_unit * 4),
                     GPoint(floor_center_point.x - drawing_unit * 2,
                            floor_center_point.y - drawing_unit * 3),
                     GPoint(floor_center_point.x + drawing_unit * 2,
                            floor_center_point.y - drawing_unit * 4),
                     GPoint(floor_center_point.x + drawing_unit * 2,
                            floor_center_point.y - drawing_unit * 3),
                     GPoint(g_back_wall_coords[depth][position][TOP_LEFT].x +
                              4,
                            g_back_wall_coords[depth][position][TOP_LEFT].y +
                              4));

    // Torso:
    if (content_type == ALIEN_OFFICER)
    {
      draw_shaded_quad(ctx,
                       GPoint(floor_center_point.x - drawing_unit * 2,
                              floor_center_point.y - drawing_unit * 8),
                       GPoint(floor_center_point.x - drawing_unit * 2,
                              floor_center_point.y - drawing_unit * 4),
                       GPoint(floor_center_point.x + drawing_unit * 2,
                              floor_center_point.y - drawing_unit * 8),
                       GPoint(floor_center_point.x + drawing_unit * 2,
                              floor_center_point.y - drawing_unit * 4),
                       GPoint(g_back_wall_coords[depth][position][TOP_LEFT].x -
                                10,
                              g_back_wall_coords[depth][position][TOP_LEFT].y -
                                10));
    }
    else // content_type == ALIEN_SOLDIER || content_type == ALIEN_ELITE
    {
      graphics_fill_rect(ctx,
                         GRect(floor_center_point.x - drawing_unit * 2,
                               floor_center_point.y - drawing_unit * 8,
                               drawing_unit * 4,
                               drawing_unit * 4),
                         NO_CORNER_RADIUS,
                         GCornerNone);
    }

    // Arms:
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_rect(ctx,
                       GRect(floor_center_point.x - drawing_unit * 3,
                             floor_center_point.y - drawing_unit * 8,
                             drawing_unit,
                             drawing_unit * 3),
                       drawing_unit / 2,
                       GCornersLeft);
    if (content_type == ALIEN_ELITE)
    {
      graphics_fill_rect(ctx,
                         GRect(floor_center_point.x + drawing_unit * 2,
                               floor_center_point.y - drawing_unit * 8,
                               drawing_unit,
                               drawing_unit * 3),
                         drawing_unit / 2,
                         GCornersRight);
      graphics_context_set_fill_color(ctx, GColorBlack);
      graphics_fill_circle(ctx,
                           GPoint(floor_center_point.x +
                                    (drawing_unit * 2 + drawing_unit / 2),
                                  floor_center_point.y -
                                    (drawing_unit * 5 + drawing_unit / 2)),
                           drawing_unit / 2 + drawing_unit / 4);
      graphics_context_set_fill_color(ctx, GColorWhite); // For the head.
    }
    else
    {
      graphics_fill_rect(ctx,
                         GRect(floor_center_point.x + drawing_unit * 2,
                               floor_center_point.y - drawing_unit * 8,
                               drawing_unit,
                               drawing_unit * 4),
                         drawing_unit / 2,
                         GCornersRight);
    }

    // Head:
    graphics_fill_rect(ctx,
                       GRect(floor_center_point.x - drawing_unit,
                             floor_center_point.y - drawing_unit * 10,
                             drawing_unit * 2 + 1,
                             drawing_unit * 2),
                       drawing_unit,
                       GCornersTop);

    // Eyes:
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_circle(ctx,
                         GPoint(floor_center_point.x - drawing_unit / 2,
                                floor_center_point.y - (drawing_unit * 9)),
                         drawing_unit / 4);
    graphics_fill_circle(ctx,
                         GPoint(floor_center_point.x + drawing_unit / 2,
                                floor_center_point.y - (drawing_unit * 9)),
                         drawing_unit / 4);

    // Gun (placed here for efficiency reasons):
    graphics_fill_circle(ctx,
                         GPoint(floor_center_point.x -
                                  (drawing_unit * 2 + drawing_unit / 2),
                                floor_center_point.y -
                                  (drawing_unit * 5 + drawing_unit / 2)),
                         drawing_unit / 2 + drawing_unit / 4);
  }
  else if (content_type == HUMAN)
  {
    // Legs:
    graphics_fill_rect(ctx,
                       GRect(floor_center_point.x -
                               (drawing_unit + drawing_unit / 2),
                             floor_center_point.y - drawing_unit * 3,
                             drawing_unit,
                             drawing_unit * 3),
                       NO_CORNER_RADIUS,
                       GCornerNone);
    graphics_fill_rect(ctx,
                       GRect(floor_center_point.x + drawing_unit / 2,
                             floor_center_point.y - drawing_unit * 3,
                             drawing_unit,
                             drawing_unit * 3),
                       NO_CORNER_RADIUS,
                       GCornerNone);

    // Waist:
    graphics_fill_rect(ctx,
                       GRect(floor_center_point.x -
                               (drawing_unit + drawing_unit / 2),
                             floor_center_point.y - drawing_unit * 4,
                             drawing_unit * 3,
                             drawing_unit),
                       NO_CORNER_RADIUS,
                       GCornerNone);

    // Torso:
    draw_shaded_quad(ctx,
                     GPoint(floor_center_point.x -
                              (drawing_unit + drawing_unit / 2),
                            floor_center_point.y - drawing_unit * 8),
                     GPoint(floor_center_point.x -
                              (drawing_unit + drawing_unit / 2),
                            floor_center_point.y - drawing_unit * 4),
                     GPoint(floor_center_point.x +
                              (drawing_unit + drawing_unit / 2),
                            floor_center_point.y - drawing_unit * 8),
                     GPoint(floor_center_point.x +
                              (drawing_unit + drawing_unit / 2),
                            floor_center_point.y - drawing_unit * 4),
                     GPoint(g_back_wall_coords[depth][position][TOP_LEFT].x -
                              20,
                            g_back_wall_coords[depth][position][TOP_LEFT].y -
                              20));

    // Arms:
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_rect(ctx,
                       GRect(floor_center_point.x - drawing_unit * 2,
                             floor_center_point.y - drawing_unit * 8,
                             drawing_unit / 2,
                             drawing_unit * 4),
                       drawing_unit / 4,
                       GCornersLeft);
    graphics_fill_rect(ctx,
                       GRect(floor_center_point.x + (drawing_unit +
                               drawing_unit / 2),
                             floor_center_point.y - drawing_unit * 8,
                             drawing_unit / 2,
                             drawing_unit * 4),
                       drawing_unit / 4,
                       GCornersRight);

    // Head:
    graphics_fill_rect(ctx,
                       GRect(floor_center_point.x - drawing_unit / 2,
                             floor_center_point.y - drawing_unit * 10,
                             drawing_unit + 1,
                             drawing_unit * 2),
                       drawing_unit / 2,
                       GCornersAll);

    // Hair:
    draw_shaded_quad(ctx,
                     GPoint(floor_center_point.x - drawing_unit / 2,
                            floor_center_point.y - drawing_unit * 10),
                     GPoint(floor_center_point.x - drawing_unit / 2,
                            floor_center_point.y -
                              (drawing_unit * 9 + drawing_unit / 3)),
                     GPoint(floor_center_point.x + drawing_unit / 2,
                            floor_center_point.y - drawing_unit * 10),
                     GPoint(floor_center_point.x + drawing_unit / 2,
                            floor_center_point.y -
                              (drawing_unit * 9 + drawing_unit / 3)),
                     GPoint(g_back_wall_coords[depth][position][TOP_LEFT].x -
                              10,
                            g_back_wall_coords[depth][position][TOP_LEFT].y -
                              10));

    // Eyes:
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_circle(ctx,
                         GPoint(floor_center_point.x - drawing_unit / 4,
                                floor_center_point.y - (drawing_unit * 9)),
                         drawing_unit / 6);
    graphics_fill_circle(ctx,
                         GPoint(floor_center_point.x + drawing_unit / 4,
                                floor_center_point.y - (drawing_unit * 9)),
                         drawing_unit / 6);
  }
  else if (content_type == ROBOT)
  {
    // Tracks/wheels:
    draw_shaded_quad(ctx,
                     GPoint(floor_center_point.x - drawing_unit * 4,
                            floor_center_point.y - drawing_unit * 2),
                     GPoint(floor_center_point.x - drawing_unit * 4,
                            floor_center_point.y),
                     GPoint(floor_center_point.x - drawing_unit,
                            floor_center_point.y - drawing_unit * 2),
                     GPoint(floor_center_point.x - drawing_unit,
                            floor_center_point.y),
                     GPoint(g_back_wall_coords[depth][position][TOP_LEFT].x +
                              6,
                            g_back_wall_coords[depth][position][TOP_LEFT].y +
                              6));
    draw_shaded_quad(ctx,
                     GPoint(floor_center_point.x + drawing_unit,
                            floor_center_point.y - drawing_unit * 2),
                     GPoint(floor_center_point.x + drawing_unit,
                            floor_center_point.y),
                     GPoint(floor_center_point.x + drawing_unit * 4,
                            floor_center_point.y - drawing_unit * 2),
                     GPoint(floor_center_point.x + drawing_unit * 4,
                            floor_center_point.y),
                     GPoint(g_back_wall_coords[depth][position][TOP_LEFT].x +
                              6,
                            g_back_wall_coords[depth][position][TOP_LEFT].y +
                              6));

    // Arms:
    draw_shaded_quad(ctx,
                     GPoint(floor_center_point.x - drawing_unit * 2,
                            floor_center_point.y - drawing_unit * 5),
                     GPoint(floor_center_point.x - drawing_unit * 2,
                            floor_center_point.y - drawing_unit * 4),
                     GPoint(floor_center_point.x + drawing_unit * 2,
                            floor_center_point.y - drawing_unit * 5),
                     GPoint(floor_center_point.x + drawing_unit * 2,
                            floor_center_point.y - drawing_unit * 4),
                     GPoint(g_back_wall_coords[depth][position][TOP_LEFT].x -
                              10,
                            g_back_wall_coords[depth][position][TOP_LEFT].y -
                              10));
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_rect(ctx,
                       GRect(floor_center_point.x - drawing_unit * 4,
                             floor_center_point.y -
                               (drawing_unit * 5 + drawing_unit / 2),
                             drawing_unit * 2,
                             drawing_unit * 2),
                       drawing_unit / 3,
                       GCornersAll);
    graphics_fill_rect(ctx,
                       GRect(floor_center_point.x + drawing_unit * 2,
                             floor_center_point.y -
                               (drawing_unit * 5 + drawing_unit / 2),
                             drawing_unit * 2 + 1,
                             drawing_unit * 2),
                       drawing_unit / 3,
                       GCornersAll);

    // Body:
    graphics_fill_rect(ctx,
                       GRect(floor_center_point.x - drawing_unit,
                             floor_center_point.y - drawing_unit * 6,
                             drawing_unit * 2,
                             drawing_unit * 5),
                       drawing_unit / 2,
                       GCornersTop);

    // Neck:
    draw_shaded_quad(ctx,
                     GPoint(floor_center_point.x - drawing_unit / 2,
                            floor_center_point.y - drawing_unit * 7),
                     GPoint(floor_center_point.x - drawing_unit / 2,
                            floor_center_point.y - drawing_unit * 6),
                     GPoint(floor_center_point.x + drawing_unit / 2,
                            floor_center_point.y - drawing_unit * 7),
                     GPoint(floor_center_point.x + drawing_unit / 2,
                            floor_center_point.y - drawing_unit * 6),
                     GPoint(g_back_wall_coords[depth][position][TOP_LEFT].x -
                              10,
                            g_back_wall_coords[depth][position][TOP_LEFT].y -
                              10));

    // Head:
    graphics_fill_rect(ctx,
                       GRect(floor_center_point.x - drawing_unit * 2,
                             floor_center_point.y - drawing_unit * 9,
                             drawing_unit * 4 + 1,
                             drawing_unit * 2),
                       drawing_unit / 3,
                       GCornersAll);

    // Eyes:
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_circle(ctx,
                         GPoint(floor_center_point.x - drawing_unit,
                                floor_center_point.y - drawing_unit * 8),
                         drawing_unit / 2);
    graphics_fill_circle(ctx,
                         GPoint(floor_center_point.x + drawing_unit,
                                floor_center_point.y - drawing_unit * 8),
                         drawing_unit / 2);

    // Guns:
    graphics_fill_circle(ctx,
                         GPoint(floor_center_point.x - drawing_unit * 3,
                                floor_center_point.y -
                                  (drawing_unit * 4 + drawing_unit / 2)),
                         drawing_unit / 2);
    graphics_fill_circle(ctx,
                         GPoint(floor_center_point.x + drawing_unit * 3,
                                floor_center_point.y -
                                  (drawing_unit * 4 + drawing_unit / 2)),
                         drawing_unit / 2);
  }
  else if (content_type == BEAST)
  {
    // Legs:
    graphics_fill_rect(ctx,
                       GRect(floor_center_point.x - drawing_unit * 3,
                             floor_center_point.y - drawing_unit * 4,
                             drawing_unit * 2,
                             drawing_unit * 4),
                       NO_CORNER_RADIUS,
                       GCornerNone);
    graphics_fill_rect(ctx,
                       GRect(floor_center_point.x + drawing_unit,
                             floor_center_point.y - drawing_unit * 4,
                             drawing_unit * 2,
                             drawing_unit * 4),
                       NO_CORNER_RADIUS,
                       GCornerNone);

    // Body/Head:
    graphics_fill_circle(ctx,
                         GPoint(floor_center_point.x,
                                floor_center_point.y - drawing_unit * 5),
                         drawing_unit * 3);

    // Eyes:
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_rect(ctx,
                       GRect(floor_center_point.x -
                               (drawing_unit + drawing_unit / 2),
                             floor_center_point.y - drawing_unit * 7,
                             drawing_unit,
                             drawing_unit / 2),
                       drawing_unit / 4,
                       GCornersAll);
    graphics_fill_rect(ctx,
                       GRect(floor_center_point.x + drawing_unit / 2,
                             floor_center_point.y - drawing_unit * 7,
                             drawing_unit,
                             drawing_unit / 2),
                       drawing_unit / 4,
                       GCornersAll);

    // Mouth:
    graphics_fill_rect(ctx,
                       GRect(floor_center_point.x -
                               (drawing_unit + drawing_unit / 2),
                             floor_center_point.y - drawing_unit * 5,
                             drawing_unit,
                             drawing_unit + drawing_unit / 2 + (time(NULL) % 2
                               ? 0 : drawing_unit / 2)),
                       drawing_unit / 2,
                       GCornersAll);
    graphics_fill_rect(ctx,
                       GRect(floor_center_point.x - drawing_unit / 2,
                             floor_center_point.y - drawing_unit * 5,
                             drawing_unit,
                             drawing_unit + drawing_unit / 2 + (time(NULL) % 2
                               ? 0 : drawing_unit / 2)),
                       drawing_unit / 2,
                       GCornersAll);
    graphics_fill_rect(ctx,
                       GRect(floor_center_point.x + drawing_unit / 2,
                             floor_center_point.y - drawing_unit * 5,
                             drawing_unit,
                             drawing_unit + drawing_unit / 2 + (time(NULL) % 2
                               ? 0 : drawing_unit / 2)),
                       drawing_unit / 2,
                       GCornersAll);
  }
  else if (content_type == OOZE)
  {
    // Body:
    graphics_fill_circle(ctx,
                         GPoint(floor_center_point.x,
                                floor_center_point.y - drawing_unit * 2),
                         drawing_unit * 2);

    // Head:
    graphics_fill_circle(ctx,
                         GPoint(floor_center_point.x,
                                floor_center_point.y - drawing_unit * 6),
                         drawing_unit * 4);

    // Eyes:
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_rect(ctx,
                       GRect(floor_center_point.x - drawing_unit * 3,
                             floor_center_point.y - drawing_unit * 7,
                             drawing_unit * 2,
                             drawing_unit),
                       drawing_unit / 2,
                       GCornersAll);
    graphics_fill_rect(ctx,
                       GRect(floor_center_point.x + drawing_unit,
                             floor_center_point.y - drawing_unit * 7,
                             drawing_unit * 2,
                             drawing_unit),
                       drawing_unit / 2,
                       GCornersAll);
  }
  else if (content_type == FLOATING_MONSTROSITY)
  {
    draw_floating_monstrosity(ctx,
                              GPoint(floor_center_point.x,
                                     floor_center_point.y - drawing_unit * 6),
                              drawing_unit * 4,
                              depth == 0 ?
                  1 + (g_back_wall_coords[depth][position][TOP_LEFT].y / 2) /
                    MAX_VISIBILITY_DEPTH :
                  1 + ((g_back_wall_coords[depth][position][TOP_LEFT].y -
                    g_back_wall_coords[depth - 1][position][TOP_LEFT].y) / 2) /
                    MAX_VISIBILITY_DEPTH);
  }
  else // content_type == ITEM
  {
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_rect(ctx,
                       GRect(floor_center_point.x - drawing_unit * 2,
                             floor_center_point.y - drawing_unit * 6,
                             drawing_unit * 4,
                             drawing_unit * 6),
                       drawing_unit / 2,
                       GCornersTop);
  }
}

/******************************************************************************
   Function: draw_floating_monstrosity

Description: Draws a "floating monstrosity" with a spherical body according to
             given specifications.

     Inputs: ctx            - Pointer to the relevant graphics context.
             center         - The sphere's center point.
             radius         - The sphere's radius.
             shading_offset - Shading offset at the sphere's position relative
                              to the player.

    Outputs: None.
******************************************************************************/
void draw_floating_monstrosity(GContext *ctx,
                               const GPoint center,
                               const int16_t radius,
                               int16_t shading_offset)
{
  int32_t theta;
  int16_t i, x_offset, y_offset;

  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_context_set_fill_color(ctx, GColorWhite);

  graphics_fill_circle(ctx, center, radius);
  for (i = radius; i > radius / 3; --i)
  {
    if (i == 2 * (radius / 3))
    {
      shading_offset *= 2;
    }
    for (theta = i % 2 ? 0 : ((TRIG_MAX_RATIO / 360) * shading_offset) / 2;
         theta < NINETY_DEGREES;
         theta += (TRIG_MAX_RATIO / 360) * shading_offset)
    {
      x_offset = cos_lookup(theta) * i / TRIG_MAX_RATIO;
      y_offset = sin_lookup(theta) * i / TRIG_MAX_RATIO;
      graphics_draw_pixel(ctx,
                          GPoint(center.x - x_offset, center.y - y_offset));
      graphics_draw_pixel(ctx,
                          GPoint(center.x + x_offset, center.y - y_offset));
      graphics_draw_pixel(ctx,
                          GPoint(center.x - x_offset, center.y + y_offset));
      graphics_draw_pixel(ctx,
                          GPoint(center.x + x_offset, center.y + y_offset));
    }
  }
}

/******************************************************************************
   Function: draw_shaded_quad

Description: Draws a shaded quadrilateral according to specifications. Assumes
             the left and right sides are parallel.

     Inputs: ctx         - Pointer to the relevant graphics context.
             upper_left  - Coordinates of the upper-left point.
             lower_left  - Coordinates of the lower-left point.
             upper_right - Coordinates of the upper-right point.
             lower_right - Coordinates of the lower-right point.
             shading_ref - Reference coordinates to assist in determining
                           shading offset values for the quad's location in
                           the 3D environment. (For walls, this is the same as
                           "upper_left".)

    Outputs: None.
******************************************************************************/
void draw_shaded_quad(GContext *ctx,
                      const GPoint upper_left,
                      const GPoint lower_left,
                      const GPoint upper_right,
                      const GPoint lower_right,
                      const GPoint shading_ref)
{
  int16_t i, j, shading_offset, half_shading_offset;
  float shading_gradient = 0;

  shading_gradient = (float) (upper_right.y - upper_left.y) /
                             (upper_right.x - upper_left.x);

  for (i = upper_left.x;
       i <= upper_right.x && i < GRAPHICS_FRAME_WIDTH;
       ++i)
  {
    // Calculate a new shading offset for each "x" value:
    shading_offset = 1 + ((shading_ref.y + (i - upper_left.x) *
                           shading_gradient) / MAX_VISIBILITY_DEPTH);

    // Round up, if applicable:
    if ((int16_t) (shading_ref.y + (i - upper_left.x) *
        shading_gradient) % MAX_VISIBILITY_DEPTH >= MAX_VISIBILITY_DEPTH /
        2 + MAX_VISIBILITY_DEPTH % 2)
    {
      shading_offset++;
    }
    half_shading_offset = (shading_offset / 2) + (shading_offset % 2);

    // Now, draw points from top to bottom:
    for (j = upper_left.y + (i - upper_left.x) * shading_gradient;
         j < lower_left.y - (i - upper_left.x) * shading_gradient;
         ++j)
    {
      if ((j + (int16_t) ((i - upper_left.x) * shading_gradient) +
          (i % 2 == 0 ? 0 : half_shading_offset)) % shading_offset == 0)
      {
        graphics_context_set_stroke_color(ctx, GColorWhite);
      }
      else
      {
        graphics_context_set_stroke_color(ctx, GColorBlack);
      }
      graphics_draw_pixel(ctx, GPoint(i, j));
    }
  }
}

/******************************************************************************
   Function: fill_quad

Description: Draws a filled quadrilateral according to specifications. Assumes
             the left and right sides are parallel.

     Inputs: ctx         - Pointer to the relevant graphics context.
             upper_left  - Coordinates of the upper-left point.
             lower_left  - Coordinates of the lower-left point.
             upper_right - Coordinates of the upper-right point.
             lower_right - Coordinates of the lower-right point.
             color       - Desired color.

    Outputs: None.
******************************************************************************/
void fill_quad(GContext *ctx,
               const GPoint upper_left,
               const GPoint lower_left,
               const GPoint upper_right,
               const GPoint lower_right,
               const GColor color)
{
  int16_t i;
  float dy_over_width = (float) (upper_right.y - upper_left.y) /
                                (upper_right.x - upper_left.x);

  graphics_context_set_stroke_color(ctx, color);
  for (i = upper_left.x;
       i <= upper_right.x && i < GRAPHICS_FRAME_WIDTH;
       ++i)
  {
    graphics_draw_line(ctx,
                       GPoint(i, upper_left.y + (i - upper_left.x) *
                                 dy_over_width),
                       GPoint(i, lower_left.y - (i - upper_left.x) *
                                 dy_over_width));
  }
}

/******************************************************************************
   Function: draw_status_bar

Description: Draws the lower status bar.

     Inputs: ctx - Pointer to the relevant graphics context.

    Outputs: None.
******************************************************************************/
void draw_status_bar(GContext *ctx)
{
  // Health meter:
  draw_status_meter(ctx,
                    GPoint (STATUS_METER_PADDING,
                            GRAPHICS_FRAME_HEIGHT + STATUS_METER_PADDING),
                    (float) g_player->stats[CURRENT_HP] /
                      g_player->stats[MAX_HP]);

  // Ammo meter:
  draw_status_meter(ctx,
                    GPoint (SCREEN_CENTER_POINT_X + STATUS_METER_PADDING +
                              COMPASS_RADIUS + 1,
                            GRAPHICS_FRAME_HEIGHT + STATUS_METER_PADDING),
                    (float) g_player->stats[CURRENT_ENERGY] /
                      g_player->stats[MAX_ENERGY]);

  // Compass:
  graphics_fill_circle(ctx,
                       GPoint(SCREEN_CENTER_POINT_X,
                              GRAPHICS_FRAME_HEIGHT + STATUS_BAR_HEIGHT / 2),
                       COMPASS_RADIUS);
  graphics_context_set_fill_color(ctx, GColorBlack);
  gpath_draw_outline(ctx, g_compass_path);
  gpath_draw_filled(ctx, g_compass_path);
}

/******************************************************************************
   Function: draw_status_meter

Description: Draws a "status meter" (such as a "health meter") at a given point
             according to given max. and current values of the attribute to be
             represented.

     Inputs: ctx    - Pointer to the relevant graphics context.
             origin - Top-left corner of the status meter.
             ratio  - Ratio of "current value" / "max. value" for the attribute
                      to be represented.

    Outputs: None.
******************************************************************************/
void draw_status_meter(GContext *ctx,
                       const GPoint origin,
                       const float ratio)
{
  int16_t i, j;

  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_context_set_fill_color(ctx, GColorWhite);

  // First, draw a "full" meter:
  graphics_fill_rect(ctx,
                     GRect(origin.x,
                           origin.y,
                           STATUS_METER_WIDTH,
                           STATUS_METER_HEIGHT),
                     SMALL_CORNER_RADIUS,
                     GCornersAll);

  // Now shade the "empty" portion:
  for (i = origin.x + STATUS_METER_WIDTH;
       i >= origin.x + (ratio * STATUS_METER_WIDTH);
       --i)
  {
    for (j = origin.y + (i % 2);
         j <= origin.y + STATUS_METER_HEIGHT;
         j += 2)
    {
      graphics_draw_pixel(ctx, GPoint(i, j));
    }
  }
}

/******************************************************************************
   Function: flash_screen

Description: Briefly "flashes" the graphics frame by inverting all its pixels.

     Inputs: None.

    Outputs: None.
******************************************************************************/
void flash_screen(void)
{
  layer_set_hidden(inverter_layer_get_layer(g_inverter_layer), false);
  g_flash_timer = app_timer_register(FLASH_TIMER_DURATION,
                                     flash_timer_callback,
                                     NULL);
}

/******************************************************************************
   Function: flash_timer_callback

Description: Called when the flash timer reaches zero. Hides the inverter
             layer, ending the "flash."

     Inputs: data - Pointer to additional data (not used).

    Outputs: None.
******************************************************************************/
static void flash_timer_callback(void *data)
{
  layer_set_hidden(inverter_layer_get_layer(g_inverter_layer), true);
}

/******************************************************************************
   Function: player_timer_callback

Description: Called when the player timer reaches zero.

     Inputs: data - Pointer to additional data (not used).

    Outputs: None.
******************************************************************************/
static void player_timer_callback(void *data)
{
  if (g_graphics_window == NULL)
  {
    return;
  }
  else if (--g_player_animation_mode > 0)
  {
    g_player_timer = app_timer_register(PLAYER_TIMER_DURATION,
                                        player_timer_callback,
                                        NULL);
    g_laser_base_width = MIN_LASER_BASE_WIDTH;
  }
  layer_mark_dirty(window_get_root_layer(g_graphics_window));
}

/******************************************************************************
   Function: graphics_window_appear

Description: Called when the graphics window appears.

     Inputs: window - Pointer to the graphics window.

    Outputs: None.
******************************************************************************/
static void graphics_window_appear(Window *window)
{
  g_game_paused           = false;
  g_player_animation_mode = 0;
  layer_set_hidden(inverter_layer_get_layer(g_inverter_layer), true);
}

/******************************************************************************
   Function: graphics_window_disappear

Description: Called when the graphics window disappears.

     Inputs: window - Pointer to the graphics window.

    Outputs: None.
******************************************************************************/
static void graphics_window_disappear(Window *window)
{
  g_game_paused = true;
}

/******************************************************************************
   Function: graphics_up_single_repeating_click

Description: The graphics window's single-click handler for the Pebble's "up"
             button. Moves the player one cell forward.

     Inputs: recognizer - The click recognizer.
             context    - Pointer to the associated context.

    Outputs: None.
******************************************************************************/
void graphics_up_single_repeating_click(ClickRecognizerRef recognizer,
                                        void *context)
{
  if (!g_game_paused)
  {
    move_player(g_player->direction);
  }
}

/******************************************************************************
   Function: graphics_up_multi_click

Description: The graphics window's multi-click handler for the "up" button.
             Turns the player to the left.

     Inputs: recognizer - The click recognizer.
             context    - Pointer to the associated context.

    Outputs: None.
******************************************************************************/
void graphics_up_multi_click(ClickRecognizerRef recognizer, void *context)
{
  if (!g_game_paused)
  {
    set_player_direction(get_direction_to_the_left(g_player->direction));
  }
}

/******************************************************************************
   Function: graphics_down_single_repeating_click

Description: The graphics window's single-click handler for the "down" button.
             Moves the player one cell backward.

     Inputs: recognizer - The click recognizer.
             context    - Pointer to the associated context.

    Outputs: None.
******************************************************************************/
void graphics_down_single_repeating_click(ClickRecognizerRef recognizer,
                                          void *context)
{
  if (!g_game_paused)
  {
    move_player(get_opposite_direction(g_player->direction));
  }
}

/******************************************************************************
   Function: graphics_down_multi_click

Description: The graphics window's multi-click handler for the "down" button.
             Turns the player to the right.

     Inputs: recognizer - The click recognizer.
             context    - Pointer to the associated context.

    Outputs: None.
******************************************************************************/
void graphics_down_multi_click(ClickRecognizerRef recognizer, void *context)
{
  if (!g_game_paused)
  {
    set_player_direction(get_direction_to_the_right(g_player->direction));
  }
}

/******************************************************************************
   Function: graphics_select_single_repeating_click

Description: The graphics window's single repeating click handler for the
             "select" button. Activate's the player's laser gun.

     Inputs: recognizer - The click recognizer.
             context    - Pointer to the associated context.

    Outputs: None.
******************************************************************************/
void graphics_select_single_repeating_click(ClickRecognizerRef recognizer,
                                            void *context)
{
  GPoint cell;
  npc_t *npc;

  if (!g_game_paused &&
      g_player->stats[CURRENT_ENERGY] >= ENERGY_LOSS_PER_SHOT)
  {
    // Deplete the player's ammo and set up the player's laser animation:
    adjust_player_current_ammo(ENERGY_LOSS_PER_SHOT);
    g_player_animation_mode = NUM_PLAYER_ANIMATIONS;
    g_laser_base_width      = MAX_LASER_BASE_WIDTH;
    g_player_timer          = app_timer_register(PLAYER_TIMER_DURATION,
                                                 player_timer_callback,
                                                 NULL);

    // Check for a damaged NPC or cell:
    cell = get_cell_farther_away(g_player->position, g_player->direction, 1);
    while (get_cell_type(cell) < SOLID)
    {
      npc = get_npc_at(cell);
      if (npc != NULL)
      {
        damage_npc(npc, g_player->stats[POWER]);
        return;
      }
      cell = get_cell_farther_away(cell, g_player->direction, 1);
    }
    damage_cell(cell, g_player->stats[POWER]);

    layer_mark_dirty(window_get_root_layer(g_graphics_window));
  }
}

/******************************************************************************
   Function: graphics_click_config_provider

Description: Button-click configuration provider for the graphics window.

     Inputs: context - Pointer to the associated context.

    Outputs: None.
******************************************************************************/
void graphics_click_config_provider(void *context)
{
  // "Up" button:
  window_single_repeating_click_subscribe(BUTTON_ID_UP,
                                          MOVEMENT_REPEAT_INTERVAL,
                                          graphics_up_single_repeating_click);
  window_multi_click_subscribe(BUTTON_ID_UP,
                               MULTI_CLICK_MIN,
                               MULTI_CLICK_MAX,
                               MULTI_CLICK_TIMEOUT,
                               LAST_CLICK_ONLY,
                               graphics_up_multi_click);

  // "Down" button:
  window_single_repeating_click_subscribe(BUTTON_ID_DOWN,
                                         MOVEMENT_REPEAT_INTERVAL,
                                         graphics_down_single_repeating_click);
  window_multi_click_subscribe(BUTTON_ID_DOWN,
                               MULTI_CLICK_MIN,
                               MULTI_CLICK_MAX,
                               MULTI_CLICK_TIMEOUT,
                               LAST_CLICK_ONLY,
                               graphics_down_multi_click);

  // "Select" button:
  window_single_repeating_click_subscribe(BUTTON_ID_SELECT,
                                       ATTACK_REPEAT_INTERVAL,
                                       graphics_select_single_repeating_click);
}

/******************************************************************************
   Function: narration_select_single_click

Description: The narration window's single-click handler for the "select"
             button. Either the next page of narration text will be displayed
             or the narration window will be closed.

     Inputs: recognizer - The click recognizer.
             context    - Pointer to the associated context.

    Outputs: None.
******************************************************************************/
void narration_select_single_click(ClickRecognizerRef recognizer,
                                   void *context)
{
  if (g_current_narration == INTRO_NARRATION_1 ||
      g_current_narration == INTRO_NARRATION_2)
  {
    g_current_narration++;
    show_narration();
  }
  else
  {
    // Remove the narration screen:
    if (window_stack_get_top_window() == g_narration_window)
    {
      window_stack_pop(NOT_ANIMATED);
    }

    // If it was a new mission description, go to the graphics window:
    if (g_current_narration < NUM_MISSION_TYPES)
    {
      show_window(g_graphics_window);
    }

    // Otherwise, go to the main menu:
    else
    {
      show_window(g_main_menu_window);
    }
  }
}

/******************************************************************************
   Function: narration_click_config_provider

Description: Button-click configurations for the narration window.

     Inputs: context - Pointer to the associated context.

    Outputs: None.
******************************************************************************/
void narration_click_config_provider(void *context)
{
  window_single_click_subscribe(BUTTON_ID_SELECT,
                                narration_select_single_click);
}

/******************************************************************************
   Function: tick_handler

Description: Handles changes to the game world every second while in active
             gameplay.

     Inputs: tick_time     - Pointer to the relevant time struct.
             units_changed - Indicates which time unit changed.

    Outputs: None.
******************************************************************************/
static void tick_handler(struct tm *tick_time, TimeUnits units_changed)
{
  npc_t *npc_pointer;

  if (!g_game_paused)
  {
    // Handle NPC behavior:
    npc_pointer = g_mission->npcs;
    while (npc_pointer != NULL)
    {
      determine_npc_behavior(npc_pointer);
      if (g_player->stats[CURRENT_HP] <= 0)
      {
        return;
      }
      npc_pointer = npc_pointer->next;
    }

    // Periodically generate new NPCs:
    if (g_mission->type != EXCAVATE &&
        g_mission->kills < g_mission->num_npcs &&
        rand() % 5 == 0)
    {
      add_new_npc(get_random_npc_type(), get_npc_spawn_point());
    }

    // Handle player stat recovery:
    adjust_player_current_hp(HP_RECOVERY_RATE);
    adjust_player_current_ammo(ENERGY_RECOVERY_RATE);

    layer_mark_dirty(window_get_root_layer(g_graphics_window));
  }
}

/******************************************************************************
   Function: app_focus_handler

Description: Handles SpaceMerc going out of, or coming back into, focus (e.g.,
             when a notification window temporarily hides this app).

     Inputs: in_focus - "True" if SpaceMerc is now in focus.

    Outputs: None.
******************************************************************************/
void app_focus_handler(const bool in_focus)
{
  if (!in_focus)
  {
    g_game_paused = true;
  }
  else
  {
    if (window_stack_get_top_window() == g_graphics_window)
    {
      g_game_paused = false;
    }
  }
}

/******************************************************************************
   Function: get_opposite_direction

Description: Returns the opposite of a given direction value (i.e., given the
             argument "NORTH", "SOUTH" will be returned).

     Inputs: direction - The direction whose opposite is desired.

    Outputs: Integer representing the opposite of the given direction.
******************************************************************************/
int16_t get_opposite_direction(const int16_t direction)
{
  switch (direction)
  {
    case NORTH:
      return SOUTH;
    case SOUTH:
      return NORTH;
    case EAST:
      return WEST;
    default: // case WEST:
      return EAST;
  }
}

/******************************************************************************
   Function: strcat_npc_name

Description: Concatenates the SINGULAR or PLURAL name of a given NPC type onto
             the end of a given string.

     Inputs: dest_str - Pointer to the destination string.
             npc_type - Integer representing the NPC of interest.
             plural   - If PLURAL (i.e., "true"), a plural form of the name is
                        concatenated.

    Outputs: None.
******************************************************************************/
void strcat_npc_name(char *dest_str,
                     const int16_t npc_type,
                     const bool plural)
{
  switch(npc_type)
  {
    case ALIEN_SOLDIER:
      strcat(dest_str, "the Fim");
      return; // Fim is both singular and plural, so it doesn't need an "s".
    case ROBOT:
      strcat(dest_str, "robot");
      break;
    case BEAST:
    case OOZE:
    case FLOATING_MONSTROSITY:
      strcat(dest_str, "creature");
      break;
  }
  if (plural)
  {
    strcat(dest_str, "s");
  }
}

/******************************************************************************
   Function: strcat_location_name

Description: Concatenates the SINGULAR or PLURAL name of a random location type
             onto the end of a given string.

     Inputs: dest_str      - Pointer to the destination string.
             location_type - Integer representing the location of interest.
             plural        - If PLURAL (i.e., "true"), a plural form of the
                             name is concatenated.

    Outputs: None.
******************************************************************************/
void strcat_location_name(char *dest_str,
                          const int16_t location_type,
                          const bool plural)
{
  int16_t i;

  switch(location_type)
  {
    case COLONY:
      strcat(dest_str, "colony");
      break;
    case CITY:
      strcat(dest_str, "city");
      break;
    case FACTORY:
      strcat(dest_str, "factory");
      break;
    case LABORATORY:
      strcat(dest_str, "laboratory");
      break;
    case BASE:
      strcat(dest_str, "base");
      break;
    case MINE:
      strcat(dest_str, "mine");
      break;
    case STARSHIP:
      strcat(dest_str, "starship");
      break;
    case SPACEPORT:
      strcat(dest_str, "spaceport");
      break;
    case SPACE_STATION:
      strcat(dest_str, "space station");
      break;
  }
  if (plural)
  {
    i = strlen(dest_str);
    if (dest_str[i - 1] == 'y')
    {
      dest_str[i - 1] = 'i';
      strcat(dest_str, "es");
    }
    else
    {
      strcat(dest_str, "s");
    }
  }
}

/******************************************************************************
   Function: strcat_int

Description: Concatenates a "large" integer value onto the end of a string. The
             absolute value of the integer may not exceed MAX_LARGE_INT_VALUE
             (if it does, MAX_LARGE_INT_VALUE will be used in its place). If
             the integer is negative, a minus sign will be included.

     Inputs: dest_str - Pointer to the destination string.
             integer  - Integer value to be converted to characters and
                        appended to the string.

    Outputs: None.
******************************************************************************/
void strcat_int(char *dest_str, int32_t integer)
{
  int16_t i, j;
  static char int_str[MAX_LARGE_INT_DIGITS + 1];
  bool negative = false;

  int_str[0] = '\0';
  if (integer < 0)
  {
    negative = true;
    integer *= -1;
  }
  if (integer > MAX_LARGE_INT_VALUE)
  {
    integer = MAX_LARGE_INT_VALUE;
  }
  if (integer == 0)
  {
    strcpy(int_str, "0");
  }
  else
  {
    for (i = 0; integer != 0; integer /= 10)
    {
      j            = integer % 10;
      int_str[i++] = '0' + j;
    }
    int_str[i] = '\0';
  }

  i = strlen(dest_str) + strlen(int_str);
  if (negative)
  {
    ++i;
  }
  dest_str[i--] = '\0';
  j             = 0;
  while (int_str[j] != '\0')
  {
    dest_str[i--] = int_str[j++];
  }
  if (negative)
  {
    dest_str[i--] = '-';
  }
}

/******************************************************************************
   Function: init_player

Description: Initializes the global player character struct according to
             default values.

     Inputs: None.

    Outputs: None.
******************************************************************************/
void init_player(void)
{
  g_player->stats[POWER]      = DEFAULT_PLAYER_POWER;
  g_player->stats[ARMOR]      = DEFAULT_PLAYER_DEFENSE;
  g_player->stats[MAX_HP]     = DEFAULT_PLAYER_MAX_HP;
  g_player->stats[MAX_ENERGY] = DEFAULT_PLAYER_MAX_AMMO;
  g_player->money             = DEFAULT_PLAYER_MONEY;
  g_player->damage_vibes_on   = DEFAULT_VIBES_SETTING;
}

/******************************************************************************
   Function: deinit_player

Description: Deinitializes the global player character struct, freeing
             associated memory.

     Inputs: None.

    Outputs: None.
******************************************************************************/
void deinit_player(void)
{
  if (g_player != NULL)
  {
    free(g_player);
    g_player = NULL;
  }
}

/******************************************************************************
   Function: init_npc

Description: Initializes a non-player character (NPC) struct according to a
             given NPC type.

     Inputs: npc      - Pointer to the NPC struct to be initialized.
             type     - Integer indicating the desired NPC type.
             position - The NPC's starting position.

    Outputs: None.
******************************************************************************/
void init_npc(npc_t *npc, const int16_t type, const GPoint position)
{
  npc->type     = type;
  npc->position = position;
  npc->next     = NULL;

  // NPC stats are based on the player's in an effort to maintain balance:
  npc->power = (g_player->stats[ARMOR] + g_player->stats[MAX_HP]) / 5;
  npc->hp    = g_player->stats[POWER] + g_player->stats[MAX_ENERGY];

  // Some NPCs have extra power or HP (or both):
  if (type == ALIEN_OFFICER ||
      type == ALIEN_ELITE   ||
      type == BEAST         ||
      type == FLOATING_MONSTROSITY)
  {
    npc->power *= 1.5;
  }
  if (type == ALIEN_OFFICER ||
      type == ROBOT         ||
      type == OOZE          ||
      type == FLOATING_MONSTROSITY)
  {
    npc->hp *= 1.5;
  }
}

/******************************************************************************
   Function: init_wall_coords

Description: Initializes the global "back_wall_coords" array so that it
             contains the top-left and bottom-right coordinates for every
             potential back wall location on the screen. (This establishes the
             field of view and sense of perspective while also facilitating
             convenient drawing of the 3D environment.)

     Inputs: None.

    Outputs: None.
******************************************************************************/
void init_wall_coords(void)
{
  int16_t i, j, wall_width;
  const float perspective_modifier = 2.0; // Helps determine FOV, etc.

  for (i = 0; i < MAX_VISIBILITY_DEPTH - 1; ++i)
  {
    for (j = 0; j < (STRAIGHT_AHEAD * 2) + 1; ++j)
    {
      g_back_wall_coords[i][j][TOP_LEFT]     = GPoint(0, 0);
      g_back_wall_coords[i][j][BOTTOM_RIGHT] = GPoint(0, 0);
    }
  }
  for (i = 0; i < MAX_VISIBILITY_DEPTH - 1; ++i)
  {
    g_back_wall_coords[i][STRAIGHT_AHEAD][TOP_LEFT] =
      GPoint(FIRST_WALL_OFFSET - i * perspective_modifier,
             FIRST_WALL_OFFSET - i * perspective_modifier);
    if (i > 0)
    {
      g_back_wall_coords[i][STRAIGHT_AHEAD][TOP_LEFT].x +=
        g_back_wall_coords[i - 1][STRAIGHT_AHEAD][TOP_LEFT].x;
      g_back_wall_coords[i][STRAIGHT_AHEAD][TOP_LEFT].y +=
        g_back_wall_coords[i - 1][STRAIGHT_AHEAD][TOP_LEFT].y;
    }
    g_back_wall_coords[i][STRAIGHT_AHEAD][BOTTOM_RIGHT].x =
      GRAPHICS_FRAME_WIDTH - g_back_wall_coords[i][STRAIGHT_AHEAD][TOP_LEFT].x;
    g_back_wall_coords[i][STRAIGHT_AHEAD][BOTTOM_RIGHT].y =
      GRAPHICS_FRAME_HEIGHT -
        g_back_wall_coords[i][STRAIGHT_AHEAD][TOP_LEFT].y;
    wall_width = g_back_wall_coords[i][STRAIGHT_AHEAD][BOTTOM_RIGHT].x -
                   g_back_wall_coords[i][STRAIGHT_AHEAD][TOP_LEFT].x;
    for (j = 1; j <= STRAIGHT_AHEAD; ++j)
    {
      g_back_wall_coords[i][STRAIGHT_AHEAD - j][TOP_LEFT]       =
        g_back_wall_coords[i][STRAIGHT_AHEAD][TOP_LEFT];
      g_back_wall_coords[i][STRAIGHT_AHEAD - j][TOP_LEFT].x     -= wall_width *
                                                                     j;
      g_back_wall_coords[i][STRAIGHT_AHEAD - j][BOTTOM_RIGHT]   =
        g_back_wall_coords[i][STRAIGHT_AHEAD][BOTTOM_RIGHT];
      g_back_wall_coords[i][STRAIGHT_AHEAD - j][BOTTOM_RIGHT].x -= wall_width *
                                                                     j;
      g_back_wall_coords[i][STRAIGHT_AHEAD + j][TOP_LEFT]       =
        g_back_wall_coords[i][STRAIGHT_AHEAD][TOP_LEFT];
      g_back_wall_coords[i][STRAIGHT_AHEAD + j][TOP_LEFT].x     += wall_width *
                                                                     j;
      g_back_wall_coords[i][STRAIGHT_AHEAD + j][BOTTOM_RIGHT]   =
        g_back_wall_coords[i][STRAIGHT_AHEAD][BOTTOM_RIGHT];
      g_back_wall_coords[i][STRAIGHT_AHEAD + j][BOTTOM_RIGHT].x += wall_width *
                                                                     j;
    }
  }
}

/******************************************************************************
   Function: init_mission

Description: Initializes the global mission struct according to a given mission
             type.

     Inputs: type - The type of mission to initialize.

    Outputs: None.
******************************************************************************/
void init_mission(const int16_t type)
{
  g_mission->type          = type;
  g_mission->location_type = rand() % NUM_LOCATION_TYPES;
  g_mission->completed     = false;
  g_mission->num_npcs      = 5 * (rand() % 6 + 1); // 5-30 enemies.
  g_mission->npcs          = NULL;
  g_mission->kills         = 0;
  if (type == RETALIATE)
  {
    g_mission->primary_npc_type = rand() % NUM_PRIMARY_NPC_TYPES;
    g_mission->reward           = 100 * (rand() % 4 + 2); // $200-$500 per kill
  }
  else // type == EXPROPRIATE, EXTRICATE, ASSASSINATE, or OBLITERATE
  {
    g_mission->primary_npc_type = ALIEN_SOLDIER;
    g_mission->reward           = 1000 * (rand() % 11 + 5); // $5,000-$15,000
  }
  init_mission_location();

  // Move and orient the player and restore his/her HP and ammo:
  set_player_direction(get_opposite_direction(g_mission->entrance_direction));
  g_player->position              = g_mission->starting_point;
  g_player->stats[CURRENT_HP]     = g_player->stats[MAX_HP];
  g_player->stats[CURRENT_ENERGY] = g_player->stats[MAX_ENERGY];

  // Finally, present mission information:
  g_current_narration = type;
  show_narration();
}

/******************************************************************************
   Function: init_mission_location

Description: Initializes the current mission's location (i.e., its 2D "cells"
             array).

     Inputs: None.

    Outputs: None.
******************************************************************************/
void init_mission_location(void)
{
  int16_t i, j, builder_direction;
  GPoint builder_position, end_point;

  // First, set each cell to full HP (i.e., "fully solid"):
  for (i = 0; i < LOCATION_WIDTH; ++i)
  {
    for (j = 0; j < LOCATION_HEIGHT; ++j)
    {
      g_mission->cells[i][j] = DEFAULT_CELL_HP;
    }
  }

  // Next, set starting and exit points:
  switch (g_mission->entrance_direction = rand() % NUM_DIRECTIONS)
  {
    case NORTH:
      g_mission->starting_point = RANDOM_POINT_NORTH;
      end_point                 = RANDOM_POINT_SOUTH;
      break;
    case SOUTH:
      g_mission->starting_point = RANDOM_POINT_SOUTH;
      end_point                 = RANDOM_POINT_NORTH;
      break;
    case EAST:
      g_mission->starting_point = RANDOM_POINT_EAST;
      end_point                 = RANDOM_POINT_WEST;
      break;
    default: // case WEST:
      g_mission->starting_point = RANDOM_POINT_WEST;
      end_point                 = RANDOM_POINT_EAST;
      break;
  }

  // Now, carve a path between the starting and end points:
  builder_position  = g_mission->starting_point;
  builder_direction = get_opposite_direction(g_mission->entrance_direction);
  while (!gpoint_equal(&builder_position, &end_point))
  {
    set_cell_type(builder_position, EMPTY);
    switch (builder_direction)
    {
      case NORTH:
        if (builder_position.y > 0)
        {
          builder_position.y--;
        }
        break;
      case SOUTH:
        if (builder_position.y < LOCATION_HEIGHT - 1)
        {
          builder_position.y++;
        }
        break;
      case EAST:
        if (builder_position.x < LOCATION_WIDTH - 1)
        {
          builder_position.x++;
        }
        break;
      default: // case WEST:
        if (builder_position.x > 0)
        {
          builder_position.x--;
        }
        break;
    }
    if (rand() % NUM_DIRECTIONS == 0) // 25% chance of turning.
    {
      builder_direction = rand() % NUM_DIRECTIONS;
    }
  }
  set_cell_type(builder_position, EMPTY);

  // Finally, add special NPCs, etc., if applicable:
  if (g_mission->type == ASSASSINATE)
  {
    add_new_npc(ALIEN_OFFICER, end_point);
  }
  else if (g_mission->type == EXPROPRIATE)
  {
    set_cell_type(end_point, ITEM);
  }
  else if (g_mission->type == EXTRICATE)
  {
    set_cell_type(end_point, HUMAN);
  }
}

/******************************************************************************
   Function: deinit_mission

Description: Deinitializes the global mission struct, freeing associated
             memory.

     Inputs: None.

    Outputs: None.
******************************************************************************/
void deinit_mission(void)
{
  if (g_mission != NULL)
  {
    while (g_mission->npcs != NULL)
    {
      remove_npc(g_mission->npcs);
    }
    free(g_mission);
    g_mission = NULL;
  }
}

/******************************************************************************
   Function: init_narration

Description: Initializes the narration window.

     Inputs: None.

    Outputs: None.
******************************************************************************/
void init_narration(void)
{
  g_narration_window = window_create();
  window_set_background_color(g_narration_window, GColorBlack);
  window_set_click_config_provider(g_narration_window,
                                   narration_click_config_provider);
  g_narration_text_layer = text_layer_create(NARRATION_TEXT_LAYER_FRAME);
  text_layer_set_background_color(g_narration_text_layer, GColorBlack);
  text_layer_set_text_color(g_narration_text_layer, GColorWhite);
  text_layer_set_font(g_narration_text_layer, NARRATION_FONT);
  text_layer_set_text_alignment(g_narration_text_layer, GTextAlignmentLeft);
  layer_add_child(window_get_root_layer(g_narration_window),
                  text_layer_get_layer(g_narration_text_layer));
}

/******************************************************************************
   Function: deinit_narration

Description: Deinitializes the narration window.

     Inputs: None.

    Outputs: None.
******************************************************************************/
void deinit_narration(void)
{
  text_layer_destroy(g_narration_text_layer);
  window_destroy(g_narration_window);
}

/******************************************************************************
   Function: init_graphics

Description: Initializes the graphics window.

     Inputs: None.

    Outputs: None.
******************************************************************************/
void init_graphics(void)
{
  // Graphics window:
  g_graphics_window = window_create();
  window_set_background_color(g_graphics_window, GColorBlack);
  window_set_window_handlers(g_graphics_window, (WindowHandlers)
  {
    .appear    = graphics_window_appear,
    .disappear = graphics_window_disappear,
  });
  window_set_click_config_provider(g_graphics_window,
                                   (ClickConfigProvider)
                                   graphics_click_config_provider);
  layer_set_update_proc(window_get_root_layer(g_graphics_window), draw_scene);

  // Graphics frame inverter (for the "flash" effect):
  g_inverter_layer = inverter_layer_create(GRAPHICS_FRAME);
  layer_add_child(window_get_root_layer(g_graphics_window),
                  inverter_layer_get_layer(g_inverter_layer));

  // Tick timer subscription:
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
}

/******************************************************************************
   Function: deinit_graphics

Description: Deinitializes the graphics window.

     Inputs: None.

    Outputs: None.
******************************************************************************/
void deinit_graphics(void)
{
  tick_timer_service_unsubscribe();
  inverter_layer_destroy(g_inverter_layer);
  window_destroy(g_graphics_window);
}

/******************************************************************************
   Function: init_upgrade_menu

Description: Initializes the upgrade menu.

     Inputs: None.

    Outputs: None.
******************************************************************************/
void init_upgrade_menu(void)
{
  g_upgrade_menu_window = window_create();
  g_upgrade_menu        = menu_layer_create(FULL_SCREEN_FRAME);
  menu_layer_set_callbacks(g_upgrade_menu, NULL, (MenuLayerCallbacks)
  {
    .get_num_sections  = menu_get_num_sections_callback,
    .get_num_rows      = menu_get_num_rows_callback,
    .get_header_height = menu_get_header_height_callback,
    .draw_header       = upgrade_menu_draw_header_callback,
    .draw_row          = upgrade_menu_draw_row_callback,
    .select_click      = upgrade_menu_select_callback,
  });
  menu_layer_set_click_config_onto_window(g_upgrade_menu,
                                          g_upgrade_menu_window);
  layer_add_child(window_get_root_layer(g_upgrade_menu_window),
                  menu_layer_get_layer(g_upgrade_menu));
}

/******************************************************************************
   Function: deinit_upgrade_menu

Description: Deinitializes the upgrade menu.

     Inputs: None.

    Outputs: None.
******************************************************************************/
void deinit_upgrade_menu(void)
{
  menu_layer_destroy(g_upgrade_menu);
  window_destroy(g_upgrade_menu_window);
}

/******************************************************************************
   Function: init_main_menu

Description: Initializes the main menu.

     Inputs: None.

    Outputs: None.
******************************************************************************/
void init_main_menu(void)
{
  g_main_menu_window = window_create();
  g_main_menu = menu_layer_create(FULL_SCREEN_FRAME);
  menu_layer_set_callbacks(g_main_menu, NULL, (MenuLayerCallbacks)
  {
    .get_num_rows = menu_get_num_rows_callback,
    .draw_row = main_menu_draw_row_callback,
    .select_click = main_menu_select_callback,
  });
  menu_layer_set_click_config_onto_window(g_main_menu, g_main_menu_window);
  layer_add_child(window_get_root_layer(g_main_menu_window),
                  menu_layer_get_layer(g_main_menu));
}

/******************************************************************************
   Function: deinit_main_menu

Description: Deinitializes the main menu.

     Inputs: None.

    Outputs: None.
******************************************************************************/
void deinit_main_menu(void)
{
  menu_layer_destroy(g_main_menu);
  window_destroy(g_main_menu_window);
}

/******************************************************************************
   Function: init

Description: Initializes the SpaceMerc app then displays the main menu (after
             an introductory narration if no saved data are detected).

     Inputs: None.

    Outputs: None.
******************************************************************************/
void init(void)
{
  srand(time(NULL));
  g_game_paused = true;
  g_mission     = NULL;
  app_focus_service_subscribe(app_focus_handler);
  init_main_menu();
  init_upgrade_menu();
  init_narration();
  init_graphics();
  init_wall_coords();
  g_compass_path = gpath_create(&COMPASS_PATH_INFO);
  gpath_move_to(g_compass_path, GPoint(SCREEN_CENTER_POINT_X,
                                       GRAPHICS_FRAME_HEIGHT +
                                         STATUS_BAR_HEIGHT / 2));

  show_window(g_main_menu_window);

  // Check for saved data and initialize the player struct:
  g_player = malloc(sizeof(player_t));
  if (persist_exists(STORAGE_KEY))
  {
    persist_read_data(STORAGE_KEY, g_player, sizeof(player_t));
  }
  else
  {
    init_player();
    g_current_narration = INTRO_NARRATION_1;
    show_narration();
  }
}

/******************************************************************************
   Function: deinit

Description: Deinitializes the SpaceMerc app.

     Inputs: None.

    Outputs: None.
******************************************************************************/
void deinit(void)
{
  persist_write_data(STORAGE_KEY, g_player, sizeof(player_t));
  app_focus_service_unsubscribe();
  deinit_upgrade_menu();
  deinit_narration();
  deinit_graphics();
  deinit_main_menu();
  deinit_mission();
  deinit_player();
}

/******************************************************************************
   Function: main

Description: Main function for the SpaceMerc app.

     Inputs: None.

    Outputs: Number of errors encountered.
******************************************************************************/
int main(void)
{
  init();
  app_event_loop();
  deinit();

  return 0;
}
