/*******************************************************************************
   Filename: space_merc.h

     Author: David C. Drake (https://davidcdrake.com)

Description: Header file for SpaceMerc, a 3D first-person shooter developed for
             the Pebble smartwatch (SDK 3). More information available online:
             https://davidcdrake.com/spacemerc
*******************************************************************************/

#ifndef SPACE_MERC_H_
#define SPACE_MERC_H_

#include <pebble.h>

/*******************************************************************************
  Enumerations
*******************************************************************************/

// Mission types:
enum {
  RETALIATE,    // Goal: Kill all enemies.
  OBLITERATE,   // Goal: Kill all enemies.
  EXPROPRIATE,  // Goal: Steal an item.
  EXTRICATE,    // Goal: Rescue a person.
  ASSASSINATE,  // Goal: Kill a Fim officer.
  NUM_MISSION_TYPES
};

// Location types:
enum {
  COLONY,
  CITY,
  LABORATORY,
  BASE,
  MINE,
  STARSHIP,
  SPACEPORT,
  SPACE_STATION,
  NUM_LOCATION_TYPES
};

// Cell types:
enum {
  HUMAN = -2,
  ITEM,
  EMPTY,
  SOLID,  // "Solid" runs from 1 to DEFAULT_CELL_HP.
  NUM_CELL_TYPES
};

// NPC types:
enum {
  NONE = -1,
  FLOATING_MONSTROSITY,
  OOZE,
  BEAST,
  ROBOT,
  ALIEN_SOLDIER,
  ALIEN_ELITE,
  ALIEN_OFFICER,
  NUM_NPC_TYPES
};

// Narration types:
enum {
  MISSION_CONCLUSION_NARRATION = NUM_MISSION_TYPES,
  DEATH_NARRATION,
  GAME_INFO_NARRATION_1,
  GAME_INFO_NARRATION_2,
  INTRO_NARRATION_1,
  INTRO_NARRATION_2,
  INTRO_NARRATION_3,
  INSTRUCTIONS_NARRATION_1,
  INSTRUCTIONS_NARRATION_2,
  NUM_NARRATION_TYPES
};

// Player stats (the order here matters for the upgrade menu):
enum {
  ARMOR,
  MAX_HP,
  POWER,
  MAX_ENERGY,
  CURRENT_HP,
  CURRENT_ENERGY,
  NUM_PLAYER_STATS
};

// Directions:
enum {
  NORTH,
  SOUTH,
  EAST,
  WEST,
  NUM_DIRECTIONS
};

/*******************************************************************************
  Other Constants
*******************************************************************************/

#define NARRATION_STR_LEN                110
#define UPGRADE_MENU_HEADER_STR_LEN      17
#define UPGRADE_TITLE_STR_LEN            13
#define UPGRADE_SUBTITLE_STR_LEN         21
#define SCREEN_WIDTH                     144
#define SCREEN_HEIGHT                    168
#define SCREEN_CENTER_POINT_X            (SCREEN_WIDTH / 2)
#define SCREEN_CENTER_POINT_Y            (SCREEN_HEIGHT / 2 - STATUS_BAR_HEIGHT * 0.75)
#define SCREEN_CENTER_POINT              GPoint(SCREEN_CENTER_POINT_X, SCREEN_CENTER_POINT_Y)
#define STATUS_BAR_HEIGHT                16  // Applies to top and bottom status bars.
#define FULL_SCREEN_FRAME                GRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT - STATUS_BAR_HEIGHT)
#define STATUS_BAR_FRAME                 GRect(0, GRAPHICS_FRAME_HEIGHT + STATUS_BAR_HEIGHT, GRAPHICS_FRAME_WIDTH, STATUS_BAR_HEIGHT)
#define GRAPHICS_FRAME                   GRect(0, STATUS_BAR_HEIGHT, GRAPHICS_FRAME_WIDTH, GRAPHICS_FRAME_HEIGHT)
#define NARRATION_TEXT_LAYER_FRAME       GRect(2, STATUS_BAR_HEIGHT, SCREEN_WIDTH - 4, SCREEN_HEIGHT)
#define COMPASS_RADIUS                   5
#define STATUS_METER_PADDING             4
#define STATUS_METER_WIDTH               (GRAPHICS_FRAME_WIDTH / 2 - COMPASS_RADIUS - 2 * STATUS_METER_PADDING)
#define STATUS_METER_HEIGHT              (STATUS_BAR_HEIGHT - STATUS_METER_PADDING * 2)
#define NO_CORNER_RADIUS                 0
#define SMALL_CORNER_RADIUS              3
#define NINETY_DEGREES                   (TRIG_MAX_ANGLE / 4)
#define DEFAULT_ROTATION_RATE            (TRIG_MAX_ANGLE / 30)  // 12 degrees
#define MULTI_CLICK_MIN                  2
#define MULTI_CLICK_MAX                  2  // We only care about double-clicks.
#define MULTI_CLICK_TIMEOUT              0
#define LAST_CLICK_ONLY                  true
#define MOVEMENT_REPEAT_INTERVAL         250  // milliseconds
#define ATTACK_REPEAT_INTERVAL           250  // milliseconds
#define PLAYER_TIMER_DURATION            20  // milliseconds
#define MAX_SMALL_INT_VALUE              9999
#define MAX_SMALL_INT_DIGITS             4
#define MAX_LARGE_INT_VALUE              999999999
#define MAX_LARGE_INT_DIGITS             9
#define MAX_INT8_VALUE                   127
#define FIRST_WALL_OFFSET                STATUS_BAR_HEIGHT
#define MIN_WALL_HEIGHT                  STATUS_BAR_HEIGHT
#define GRAPHICS_FRAME_WIDTH             SCREEN_WIDTH
#define GRAPHICS_FRAME_HEIGHT            (SCREEN_HEIGHT - 2 * STATUS_BAR_HEIGHT)
#define LOCATION_WIDTH                   15
#define LOCATION_HEIGHT                  LOCATION_WIDTH
#define MAX_VISIBILITY_DEPTH             6  // Helps determine no. of cells visible in a given line of sight.
#define STRAIGHT_AHEAD                   (MAX_VISIBILITY_DEPTH - 1)  // Index value for "g_back_wall_coords".
#define TOP_LEFT                         0  // Index value for "g_back_wall_coords".
#define BOTTOM_RIGHT                     1  // Index value for "g_back_wall_coords".
#define RANDOM_POINT_NORTH               GPoint(rand() % LOCATION_WIDTH, 0)
#define RANDOM_POINT_SOUTH               GPoint(rand() % LOCATION_WIDTH, LOCATION_HEIGHT - 1)
#define RANDOM_POINT_EAST                GPoint(LOCATION_WIDTH - 1, rand() % LOCATION_HEIGHT)
#define RANDOM_POINT_WEST                GPoint(0, rand() % LOCATION_HEIGHT)
#define NARRATION_FONT                   fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD)
#define MAIN_MENU_NUM_ROWS               5
#define UPGRADE_MENU_NUM_ROWS            4
#define DEFAULT_VIBES_SETTING            true
#define DEFAULT_PLAYER_MONEY             0
#define DEFAULT_PLAYER_POWER             5
#define DEFAULT_PLAYER_DEFENSE           5
#define DEFAULT_PLAYER_MAX_HP            10
#define DEFAULT_PLAYER_MAX_AMMO          10
#define DEFAULT_CELL_HP                  50
#define STAT_BOOST_PER_UPGRADE           5
#define UPGRADE_COST_MULTIPLIER          250
#define NUM_PLAYER_ANIMATIONS            2  // No. of steps in the player's attack animation.
#define MIN_LASER_BASE_WIDTH             8
#define MAX_LASER_BASE_WIDTH             12
#define HP_RECOVERY_RATE                 1  // HP per second.
#define ENERGY_RECOVERY_RATE             1  // Energy (ammo) per second.
#define MIN_DAMAGE                       (HP_RECOVERY_RATE + 1)
#define ENERGY_LOSS_PER_SHOT             (ENERGY_RECOVERY_RATE + 1)
#define PLAYER_STORAGE_KEY               417
#define MISSION_STORAGE_KEY              (PLAYER_STORAGE_KEY + 1)
#define MAX_NPCS_AT_ONE_TIME             2
#define ANIMATED                         true
#define NOT_ANIMATED                     false
#define RANDOM_NPC_TYPE                  (rand() % (NUM_NPC_TYPES - 1))  // Excludes ALIEN_OFFICER.
#ifdef PBL_COLOR
#define NUM_BACKGROUND_COLOR_SCHEMES     8
#define NUM_BACKGROUND_COLORS_PER_SCHEME 10
#define RANDOM_COLOR                     GColorFromRGB(rand() % 256, rand() % 256, rand() % 256)
#define RANDOM_DARK_COLOR                GColorFromRGB(rand() % 128, rand() % 128, rand() % 128)
#define RANDOM_BRIGHT_COLOR              GColorFromRGB(rand() % 128 + 128, rand() % 128 + 128, rand() % 128 + 128)
#define NPC_LASER_COLOR                  (rand() % 2 ? GColorSunsetOrange : GColorDarkCandyAppleRed)
#endif

static const GPathInfo COMPASS_PATH_INFO = {
  .num_points = 4,
  .points = (GPoint []) {{-3, -3},
                         {3, -3},
                         {0, 6},
                         {-3, -3}}
};

static const char *const g_narration_strings[] = {
  "You fell in battle, but your body was found and resuscitated. Soldier on!",
  "SpaceMerc v1.9, designed and programmed by David C. Drake:\n\ndavidcdrake.com",
  "Thanks for playing! And special thanks to Team Pebble for creating these wonderful, fun, and useful devices!",
  "Humankind is at war with a hostile alien race known as the Fim.",
  "As an elite interstellar mercenary, your skills are in high demand.",
  "Fame and fortune await as you risk life and limb for humanity's future!",
  "    INSTRUCTIONS\nForward: \"Up\"\nBack: \"Down\"\nLeft: \"Up\" x 2\nRight: \"Down\" x 2\nShoot: \"Select\"",
  "    INSTRUCTIONS\nTo end a mission, walk out through the door where the mission began.",
};

static const char *const g_location_strings[] = {
  "colony",
  "city",
  "laboratory",
  "base",
  "mine",
  "starship",
  "spaceport",
  "space station",
};

/*******************************************************************************
  Structures
*******************************************************************************/

typedef struct PlayerCharacter {
  GPoint position;
  int16_t direction,
          stats[NUM_PLAYER_STATS];
  int32_t money;
  bool damage_vibes_on;
} __attribute__((__packed__)) player_t;

typedef struct NonPlayerCharacter {
  GPoint position;
  int8_t type,
         power,
         hp;
} __attribute__((__packed__)) npc_t;

typedef struct Mission {
  int8_t type,
         cells[LOCATION_WIDTH][LOCATION_HEIGHT],
#ifdef PBL_COLOR
         floor_color_scheme,
         wall_color_scheme,
#endif
         entrance_direction,
         total_num_npcs,
         kills;
  int32_t reward;
  GPoint entrance;
  npc_t npcs[MAX_NPCS_AT_ONE_TIME];
  bool completed;
} __attribute__((__packed__)) mission_t;

/*******************************************************************************
  Global Variables
*******************************************************************************/

Window *g_graphics_window,
       *g_narration_window,
       *g_main_menu_window,
       *g_upgrade_menu_window;
MenuLayer *g_main_menu,
          *g_upgrade_menu;
TextLayer *g_narration_text_layer;
StatusBarLayer *g_status_bar;
AppTimer *g_player_timer;
GPoint g_back_wall_coords[MAX_VISIBILITY_DEPTH - 1]
                         [(STRAIGHT_AHEAD * 2) + 1]
                         [2];
bool g_game_paused;
int8_t g_current_narration,
       g_player_animation_mode,
       g_laser_base_width;
GPath *g_compass_path;
mission_t *g_mission;
player_t *g_player;
#ifdef PBL_COLOR
GColor g_background_colors[NUM_BACKGROUND_COLOR_SCHEMES]
                          [NUM_BACKGROUND_COLORS_PER_SCHEME];
#endif

/*******************************************************************************
  Function Declarations
*******************************************************************************/

void set_player_direction(const int8_t new_direction);
void move_player(const int8_t direction);
void move_npc(npc_t *npc, const int8_t direction);
void determine_npc_behavior(npc_t *npc);
void damage_player(int16_t damage);
void damage_npc(npc_t *npc, const int16_t damage);
void damage_cell(GPoint cell, const int16_t damage);
bool adjust_player_money(const int32_t amount);
void adjust_player_current_hp(const int16_t amount);
void adjust_player_current_ammo(const int16_t amount);
void add_new_npc(const int8_t npc_type, const GPoint position);
GPoint get_npc_spawn_point(void);
GPoint get_floor_center_point(const int8_t depth, const int8_t position);
GPoint get_cell_farther_away(const GPoint reference_point,
                             const int8_t direction,
                             const int8_t distance);
int8_t get_pursuit_direction(const GPoint pursuer, const GPoint pursuee);
int8_t get_direction_to_the_left(const int8_t reference_direction);
int8_t get_direction_to_the_right(const int8_t reference_direction);
int8_t get_opposite_direction(const int8_t direction);
int16_t get_upgraded_stat_value(const int8_t stat_index);
int32_t get_upgrade_cost(const int16_t upgraded_stat_value);
int8_t get_cell_type(const GPoint cell);
void set_cell_type(GPoint cell, const int8_t type);
npc_t *get_npc_at(const GPoint cell);
bool out_of_bounds(const GPoint cell);
bool occupiable(const GPoint cell);
bool touching(const GPoint cell, const GPoint cell_2);
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
                     const int8_t depth,
                     const int8_t position);
void draw_cell_contents(GContext *ctx,
                        const GPoint cell,
                        const int8_t depth,
                        const int8_t position);
void draw_floating_monstrosity(GContext *ctx,
                               GPoint center,
                               const int8_t radius,
                               int8_t shading_offset);
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
void draw_status_meter(GContext *ctx,
                       GPoint origin,
                       const float ratio);
static void player_timer_callback(void *data);
static void main_menu_window_appear(Window *window);
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
void narration_single_click(ClickRecognizerRef recognizer, void *context);
void narration_click_config_provider(void *context);
static void tick_handler(struct tm *tick_time, TimeUnits units_changed);
void app_focus_handler(const bool in_focus);
void init_player(void);
void deinit_player(void);
void init_npc(npc_t *npc, const int8_t type, const GPoint position);
void init_wall_coords(void);
void init_mission(const int8_t type);
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

#endif  // SPACE_MERC_H_
