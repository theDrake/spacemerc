/******************************************************************************
   Filename: space_merc.h

     Author: David C. Drake (http://davidcdrake.com)

Description: Header file for SpaceMerc, a 3D first-person shooter developed for
             the Pebble smartwatch (SDK 2.0). Copyright 2014, David C. Drake.
             More info available online: http://davidcdrake.com/spacemerc
******************************************************************************/

#ifndef SPACE_MERC_H_
#define SPACE_MERC_H_

#include <pebble.h>

/******************************************************************************
  Constants
******************************************************************************/

#define NARRATION_STR_LEN               85
#define UPGRADE_MENU_HEADER_STR_LEN     17
#define UPGRADE_TITLE_STR_LEN           13
#define UPGRADE_SUBTITLE_STR_LEN        21
#define SCREEN_WIDTH                    144
#define SCREEN_HEIGHT                   168
#define SCREEN_CENTER_POINT_X           (SCREEN_WIDTH / 2)
#define SCREEN_CENTER_POINT_Y           (SCREEN_HEIGHT / 2 - STATUS_BAR_HEIGHT * 0.75)
#define SCREEN_CENTER_POINT             GPoint(SCREEN_CENTER_POINT_X, SCREEN_CENTER_POINT_Y)
#define STATUS_BAR_HEIGHT               16 // Applies to top and bottom status bars.
#define FULL_SCREEN_FRAME               GRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT - STATUS_BAR_HEIGHT)
#define STATUS_BAR_FRAME                GRect(0, GRAPHICS_FRAME_HEIGHT, GRAPHICS_FRAME_WIDTH, STATUS_BAR_HEIGHT)
#define COMPASS_RADIUS                  5
#define STATUS_METER_PADDING            4
#define STATUS_METER_WIDTH              (GRAPHICS_FRAME_WIDTH / 2 - COMPASS_RADIUS - 2 * STATUS_METER_PADDING)
#define STATUS_METER_HEIGHT             (STATUS_BAR_HEIGHT - STATUS_METER_PADDING * 2)
#define NO_CORNER_RADIUS                0
#define SMALL_CORNER_RADIUS             3
#define NINETY_DEGREES                  (TRIG_MAX_ANGLE / 4)
#define DEFAULT_ROTATION_RATE           (TRIG_MAX_ANGLE / 30) // 12 degrees
#define MULTI_CLICK_MIN                 2
#define MULTI_CLICK_MAX                 2 // We only care about double-clicks.
#define MULTI_CLICK_TIMEOUT             0
#define LAST_CLICK_ONLY                 true
#define MOVEMENT_REPEAT_INTERVAL        250 // milliseconds
#define ATTACK_REPEAT_INTERVAL          250 // milliseconds
#define PLAYER_TIMER_DURATION           20  // milliseconds
#define FLASH_TIMER_DURATION            20  // milliseconds
#define MAX_SMALL_INT_VALUE             9999
#define MAX_SMALL_INT_DIGITS            4
#define MAX_LARGE_INT_VALUE             999999999
#define MAX_LARGE_INT_DIGITS            9
#define FIRST_WALL_OFFSET               STATUS_BAR_HEIGHT
#define MIN_WALL_HEIGHT                 STATUS_BAR_HEIGHT
#define GRAPHICS_FRAME_WIDTH            SCREEN_WIDTH
#define GRAPHICS_FRAME_HEIGHT           (SCREEN_HEIGHT - 2 * STATUS_BAR_HEIGHT)
#define GRAPHICS_FRAME                  GRect(0, 0, GRAPHICS_FRAME_WIDTH, GRAPHICS_FRAME_HEIGHT)
#define LOCATION_WIDTH                  15
#define LOCATION_HEIGHT                 LOCATION_WIDTH
#define MAX_VISIBILITY_DEPTH            6 // Helps determine no. of cells visible in a given line of sight.
#define STRAIGHT_AHEAD                  (MAX_VISIBILITY_DEPTH - 1) // Index value for "g_back_wall_coords".
#define TOP_LEFT                        0 // Index value for "g_back_wall_coords".
#define BOTTOM_RIGHT                    1 // Index value for "g_back_wall_coords".
#define RANDOM_POINT_NORTH              GPoint(rand() % LOCATION_WIDTH, 0)
#define RANDOM_POINT_SOUTH              GPoint(rand() % LOCATION_WIDTH, LOCATION_HEIGHT - 1)
#define RANDOM_POINT_EAST               GPoint(LOCATION_WIDTH - 1, rand() % LOCATION_HEIGHT)
#define RANDOM_POINT_WEST               GPoint(0, rand() % LOCATION_HEIGHT)
#define NARRATION_TEXT_LAYER_FRAME      GRect(2, 0, SCREEN_WIDTH - 4, SCREEN_HEIGHT)
#define NARRATION_FONT                  fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD)
#define MAIN_MENU_NUM_ROWS              5
#define UPGRADE_MENU_NUM_ROWS           4
#define DEFAULT_VIBES_SETTING           false
#define DEFAULT_PLAYER_MONEY            0
#define DEFAULT_PLAYER_POWER            10
#define DEFAULT_PLAYER_DEFENSE          10
#define DEFAULT_PLAYER_MAX_HP           30
#define DEFAULT_PLAYER_MAX_AMMO         30
#define DEFAULT_CELL_HP                 50
#define STAT_BOOST_PER_UPGRADE          5
#define UPGRADE_COST_MULTIPLIER         100
#define NUM_PLAYER_ANIMATIONS           2 // No. of steps in the player's attack animation.
#define MAX_LASER_BASE_WIDTH            12
#define MIN_LASER_BASE_WIDTH            8
#define HP_RECOVERY_RATE                1 // HP per second.
#define ENERGY_RECOVERY_RATE            1 // Energy (ammo) per second.
#define MIN_DAMAGE                      2
#define ENERGY_LOSS_PER_SHOT            -2
#define STORAGE_KEY                     417
#define MAX_NPCS_AT_ONE_TIME            3
#define MIN_NPCS_PER_MISSION            10
#define MAX_NPCS_PER_MISSION            30
#define ANIMATED                        true
#define NOT_ANIMATED                    false

/******************************************************************************
  Enumerations (replaced with #defines to save memory)
******************************************************************************/

// Mission types:
#define EXCAVATE          0 // Blast as many walls as you wish.
#define RETALIATE         1 // Kill enemies, protect walls.
#define EXPROPRIATE       2 // Obtain an object.
#define EXTRICATE         3 // Rescue a person.
#define ASSASSINATE       4 // Kill a specific enemy.
#define OBLITERATE        5 // Blast as many enemies/walls as you wish.
#define NUM_MISSION_TYPES 6

// Narration types:
#define DEATH_NARRATION                NUM_MISSION_TYPES
#define MISSION_FAILED_NARRATION       7
#define MISSION_ACCOMPLISHED_NARRATION 8
#define CONTROLS_NARRATION             9
#define GAME_INFO_NARRATION            10
#define INTRO_NARRATION_1              11
#define INTRO_NARRATION_2              12
#define INTRO_NARRATION_3              13
#define NUM_NARRATION_TYPES            14

// Cell types:
#define SOLID 1 // "Solid" runs from 1 to DEFAULT_CELL_HP.
#define EMPTY 0
#define ITEM  -1
#define HUMAN -2

// NPC types:
#define BEAST                0
#define OOZE                 1
#define FLOATING_MONSTROSITY 2
#define ROBOT                3
#define ALIEN_SOLDIER        4
#define ALIEN_ELITE          5
#define ALIEN_OFFICER        6
#define NUM_NPC_TYPES        7
#define NUM_RANDOM_NPC_TYPES 6

// Player stats (the order here matters for the upgrade menu):
#define ARMOR            0
#define MAX_HP           1
#define POWER            2
#define MAX_ENERGY       3
#define CURRENT_HP       4
#define CURRENT_ENERGY   5
#define NUM_PLAYER_STATS 6

// Directions:
#define NORTH          0
#define SOUTH          1
#define EAST           2
#define WEST           3
#define NUM_DIRECTIONS 4

/******************************************************************************
  Structures
******************************************************************************/

typedef struct PlayerCharacter {
  GPoint position;
  int16_t direction,
          stats[NUM_PLAYER_STATS];
  int32_t money;
  bool damage_vibes_on;
} __attribute__((__packed__)) player_t;

typedef struct NonPlayerCharacter {
  GPoint position;
  int16_t type,
          power,
          hp;
  struct NonPlayerCharacter *next;
} __attribute__((__packed__)) npc_t;

typedef struct Mission {
  int16_t type,
          cells[LOCATION_WIDTH][LOCATION_HEIGHT],
          entrance_direction,
          num_npcs,
          kills,
          demolitions;
  int32_t reward;
  GPoint starting_point;
  npc_t *npcs;
  bool completed;
} __attribute__((__packed__)) mission_t;

/******************************************************************************
  Global Variables
******************************************************************************/

Window *g_graphics_window,
       *g_narration_window,
       *g_main_menu_window,
       *g_upgrade_menu_window;
InverterLayer *g_graphics_inverter;
MenuLayer *g_main_menu,
          *g_upgrade_menu;
TextLayer *g_narration_text_layer;
AppTimer *g_player_timer,
         *g_flash_timer;
GPoint g_back_wall_coords[MAX_VISIBILITY_DEPTH - 1]
                         [(STRAIGHT_AHEAD * 2) + 1]
                         [2];
bool g_game_paused;
int16_t g_current_narration,
        g_player_animation_mode,
        g_laser_base_width;
GPath *g_compass_path;
mission_t *g_mission;
player_t *g_player;

static const GPathInfo COMPASS_PATH_INFO = {
  .num_points = 4,
  .points = (GPoint []) {{-3, -3},
                         {3, -3},
                         {0, 6},
                         {-3, -3}}
};

/******************************************************************************
  Function Declarations
******************************************************************************/

void set_player_direction(const int16_t new_direction);
void move_player(const int16_t direction);
void move_npc(npc_t *npc, const int16_t direction);
void determine_npc_behavior(npc_t *npc);
int16_t get_pursuit_direction(const GPoint pursuer, const GPoint pursuee);
bool touching(const GPoint cell, const GPoint cell_2);
void damage_player(int16_t damage);
void damage_npc(npc_t *npc, const int16_t damage);
void damage_cell(GPoint cell, const int16_t damage);
bool adjust_player_money(const int32_t amount);
void adjust_player_current_hp(const int16_t amount);
void remove_npc(npc_t *npc);
void adjust_player_current_ammo(const int16_t amount);
void end_mission(void);
void add_new_npc(const int16_t npc_type, const GPoint position);
int16_t get_random_npc_type(void);
GPoint get_npc_spawn_point(void);
GPoint get_floor_center_point(const int16_t depth, const int16_t position);
GPoint get_cell_farther_away(const GPoint reference_point,
                             const int16_t direction,
                             const int16_t distance);
int16_t get_direction_to_the_left(const int16_t reference_direction);
int16_t get_direction_to_the_right(const int16_t reference_direction);
int16_t get_cell_type(const GPoint cell);
void set_cell_type(GPoint cell, const int16_t type);
npc_t *get_npc_at(const GPoint cell);
bool out_of_bounds(const GPoint cell);
bool occupiable(const GPoint cell);
void show_narration(void);
void show_window(Window *window);
static void main_menu_draw_row_callback(GContext *ctx,
                                        const Layer *cell_layer,
                                        MenuIndex *cell_index,
                                        void *data);
void main_menu_select_callback(MenuLayer *menu_layer,
                               MenuIndex *cell_index,
                               void *data);
static void upgrade_menu_draw_header_callback(GContext *ctx,
                                              const Layer *cell_layer,
                                              uint16_t section_index,
                                              void *data);
static void upgrade_menu_draw_row_callback(GContext *ctx,
                                           const Layer *cell_layer,
                                           MenuIndex *cell_index,
                                           void *data);
void upgrade_menu_select_callback(MenuLayer *menu_layer,
                                  MenuIndex *cell_index,
                                  void *data);
int16_t get_upgraded_stat_value(const int16_t stat_index);
int32_t get_upgrade_cost(const int16_t upgraded_stat_value);
static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer,
                                               void *data);
static int16_t menu_get_header_height_callback(MenuLayer *menu_layer,
                                               uint16_t section_index,
                                               void *data);
static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer,
                                           uint16_t section_index,
                                           void *data);
void draw_scene(Layer *layer, GContext *ctx);
void draw_player_laser_beam(GContext *ctx);
void draw_floor_and_ceiling(GContext *ctx);
void draw_cell_walls(GContext *ctx,
                     const GPoint cell,
                     const int16_t depth,
                     const int16_t position);
void draw_cell_contents(GContext *ctx,
                        const GPoint cell,
                        const int16_t depth,
                        const int16_t position);
void draw_floating_monstrosity(GContext *ctx,
                               const GPoint center,
                               const int16_t radius,
                               int16_t shading_offset);
void draw_shaded_quad(GContext *ctx,
                      const GPoint upper_left,
                      const GPoint lower_left,
                      const GPoint upper_right,
                      const GPoint lower_right,
                      const GPoint shading_ref);
void fill_quad(GContext *ctx,
               const GPoint upper_left,
               const GPoint lower_left,
               const GPoint upper_right,
               const GPoint lower_right,
               const GColor color);
void draw_status_bar(GContext *ctx);
void draw_status_meter(GContext *ctx,
                       const GPoint origin,
                       const float ratio);
void flash_screen(void);
static void flash_timer_callback(void *data);
static void player_timer_callback(void *data);
static void graphics_window_appear(Window *window);
static void graphics_window_disappear(Window *window);
void graphics_up_single_repeating_click(ClickRecognizerRef recognizer,
                                        void *context);
void graphics_up_multi_click(ClickRecognizerRef recognizer, void *context);
void graphics_down_single_repeating_click(ClickRecognizerRef recognizer,
                                          void *context);
void graphics_down_multi_click(ClickRecognizerRef recognizer, void *context);
void graphics_select_single_repeating_click(ClickRecognizerRef recognizer,
                                            void *context);
void graphics_click_config_provider(void *context);
void narration_select_single_click(ClickRecognizerRef recognizer,
                                   void *context);
void narration_click_config_provider(void *context);
static void tick_handler(struct tm *tick_time, TimeUnits units_changed);
void app_focus_handler(const bool in_focus);
int16_t get_opposite_direction(const int16_t direction);
void strcat_int(char *dest_str, int32_t integer);
void init_player(void);
void deinit_player(void);
void init_npc(npc_t *npc, const int16_t type, const GPoint position);
void init_wall_coords(void);
void init_mission(const int16_t type);
void init_mission_location(void);
void deinit_mission(void);
void init_narration(void);
void deinit_narration(void);
void init_graphics(void);
void deinit_graphics(void);
void init_upgrade_menu(void);
void deinit_upgrade_menu(void);
void init_main_menu(void);
void deinit_main_menu(void);
void init(void);
void deinit(void);
int main(void);

#endif // SPACE_MERC_H_
