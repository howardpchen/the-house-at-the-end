#include <pebble.h>

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "house_state.h"

#define PERSIST_KEY_STATE 1
#define NOTICE_DURATION_MS 2200

typedef enum {
  VIEW_HOME = 0,
  VIEW_HEARTH,
  VIEW_WORKSHOP,
  VIEW_GUESTS,
  VIEW_DOOR,
  VIEW_EXPEDITION,
  VIEW_ENCOUNTER,
  VIEW_CHRONICLE
} AppView;

typedef enum {
  HOME_HEARTH = 0,
  HOME_WORKSHOP,
  HOME_GUESTS,
  HOME_DOOR,
  HOME_CHRONICLE
} HomeItem;

typedef struct {
  uint16_t schema;
  uint16_t state_size;
  HouseState state;
  uint32_t checksum;
} PersistedState;

_Static_assert(sizeof(PersistedState) <= 256,
               "The save record must fit one Pebble persistence value");

static Window *s_window;
static Layer *s_canvas;
static AppTimer *s_notice_timer;
static HouseState s_state;
static AppView s_view;
static int16_t s_selected;
static char s_notice[72];

static const GColor s_color_background = PBL_IF_COLOR_ELSE(GColorOxfordBlue,
                                                            GColorBlack);
static const GColor s_color_text = GColorWhite;
static const GColor s_color_muted = PBL_IF_COLOR_ELSE(GColorLightGray,
                                                       GColorWhite);
static const GColor s_color_accent = PBL_IF_COLOR_ELSE(GColorChromeYellow,
                                                        GColorWhite);
static const GColor s_color_locked = PBL_IF_COLOR_ELSE(GColorDarkGray,
                                                        GColorWhite);

static uint32_t prv_checksum(const void *data, size_t length) {
  const uint8_t *bytes = data;
  uint32_t hash = 2166136261U;
  for (size_t i = 0; i < length; ++i) {
    hash ^= bytes[i];
    hash *= 16777619U;
  }
  return hash;
}

static void prv_save(void) {
  PersistedState record;
  memset(&record, 0, sizeof(record));
  record.schema = HOUSE_STATE_SCHEMA;
  record.state_size = sizeof(HouseState);
  record.state = s_state;
  record.checksum = prv_checksum(&record, offsetof(PersistedState, checksum));

  const int result = persist_write_data(PERSIST_KEY_STATE, &record,
                                        sizeof(record));
  if (result != (int)sizeof(record)) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "State save failed: %d", result);
  }
}

static bool prv_load(void) {
  if (!persist_exists(PERSIST_KEY_STATE) ||
      persist_get_size(PERSIST_KEY_STATE) != (int)sizeof(PersistedState)) {
    return false;
  }

  PersistedState record;
  memset(&record, 0, sizeof(record));
  if (persist_read_data(PERSIST_KEY_STATE, &record, sizeof(record)) !=
      (int)sizeof(record)) {
    return false;
  }

  const uint32_t expected =
      prv_checksum(&record, offsetof(PersistedState, checksum));
  if (record.schema != HOUSE_STATE_SCHEMA ||
      record.state_size != sizeof(HouseState) ||
      record.checksum != expected || !house_state_is_valid(&record.state)) {
    APP_LOG(APP_LOG_LEVEL_WARNING, "Ignoring invalid state record");
    return false;
  }

  s_state = record.state;
  return true;
}

static void prv_notice_timeout(void *context) {
  (void)context;
  s_notice_timer = NULL;
  s_notice[0] = '\0';
  if (s_canvas) {
    layer_mark_dirty(s_canvas);
  }
}

static void prv_show_notice(const char *text) {
  if (s_notice_timer) {
    app_timer_cancel(s_notice_timer);
  }
  snprintf(s_notice, sizeof(s_notice), "%s", text ? text : "");
  s_notice_timer = app_timer_register(NOTICE_DURATION_MS,
                                      prv_notice_timeout, NULL);
  layer_mark_dirty(s_canvas);
}

static void prv_set_view(AppView view) {
  s_view = view;
  s_selected = 0;
  layer_mark_dirty(s_canvas);
}

static int16_t prv_home_item_count(void) {
  int16_t count = 2;
  if (s_state.residents > 0) {
    count++;
  }
  if (house_has_build(&s_state, HOUSE_BUILD_ANCHOR_LINE)) {
    count++;
  }
  if (s_state.memories > 0) {
    count++;
  }
  return count;
}

static HomeItem prv_home_item_at(int16_t index) {
  if (index == 0) {
    return HOME_HEARTH;
  }
  if (index == 1) {
    return HOME_WORKSHOP;
  }

  int16_t cursor = 2;
  if (s_state.residents > 0) {
    if (index == cursor) {
      return HOME_GUESTS;
    }
    cursor++;
  }
  if (house_has_build(&s_state, HOUSE_BUILD_ANCHOR_LINE)) {
    if (index == cursor) {
      return HOME_DOOR;
    }
    cursor++;
  }
  return HOME_CHRONICLE;
}

static int16_t prv_item_count(void) {
  switch (s_view) {
    case VIEW_HOME:
      return prv_home_item_count();
    case VIEW_HEARTH:
    case VIEW_WORKSHOP:
      return 3;
    case VIEW_GUESTS:
    case VIEW_EXPEDITION:
    case VIEW_ENCOUNTER:
      return 2;
    case VIEW_DOOR:
    case VIEW_CHRONICLE:
      return 1;
  }
  return 1;
}

static const char *prv_title(void) {
  switch (s_view) {
    case VIEW_HOME:
      return "THE HOUSE";
    case VIEW_HEARTH:
      return "THE HEARTH";
    case VIEW_WORKSHOP:
      return "WORKSHOP";
    case VIEW_GUESTS:
      return "THE GUESTS";
    case VIEW_DOOR:
      return "FRONT DOOR";
    case VIEW_EXPEDITION:
      return "THE DRIFT";
    case VIEW_ENCOUNTER:
      return "AN ECHO";
    case VIEW_CHRONICLE:
      return "CROOKED HALL";
  }
  return "THE HOUSE";
}

static void prv_item_text(int16_t index, char *label, size_t label_size,
                          char *detail, size_t detail_size, bool *enabled) {
  *enabled = true;
  label[0] = '\0';
  detail[0] = '\0';

  if (s_view == VIEW_HOME) {
    switch (prv_home_item_at(index)) {
      case HOME_HEARTH:
        snprintf(label, label_size, "Hearth");
        snprintf(detail, detail_size, "Search, feed, and prepare.");
        return;
      case HOME_WORKSHOP:
        snprintf(label, label_size, "Workshop");
        snprintf(detail, detail_size, "Make the house more real.");
        return;
      case HOME_GUESTS:
        snprintf(label, label_size, "Guests");
        snprintf(detail, detail_size, "%u here: %u gather, %u listen.",
                 s_state.residents, s_state.gatherers, s_state.listeners);
        return;
      case HOME_DOOR:
        snprintf(label, label_size, "Front door");
        snprintf(detail, detail_size, "The anchor line disappears outward.");
        return;
      case HOME_CHRONICLE:
        snprintf(label, label_size, "Chronicle");
        snprintf(detail, detail_size, "%u memory held.", s_state.memories);
        return;
    }
  }

  if (s_view == VIEW_HEARTH) {
    if (index == 0) {
      snprintf(label, label_size, "Search rooms");
      snprintf(detail, detail_size, "+kindling; sometimes a remnant.");
    } else if (index == 1) {
      snprintf(label, label_size, "Feed hearth");
      snprintf(detail, detail_size, "Costs 2 kindling. Stability %u/5.",
               s_state.hearth_level);
      *enabled = s_state.hearth_level < 5 && s_state.kindling >= 2;
    } else {
      snprintf(label, label_size, "Prepare ration");
      snprintf(detail, detail_size, "Costs 1 kindling and 1 remnant.");
      *enabled = house_has_build(&s_state, HOUSE_BUILD_WORKTABLE) &&
                 s_state.kindling >= 1 && s_state.remnants >= 1;
    }
    return;
  }

  if (s_view == VIEW_WORKSHOP) {
    const HouseBuild build = (HouseBuild)index;
    static const char *const names[] = {
        "Guest room", "Worktable", "Anchor line"};
    int16_t kindling = 0;
    int16_t remnants = 0;
    house_build_cost(build, &kindling, &remnants);
    snprintf(label, label_size, "%s%s", names[index],
             house_has_build(&s_state, build) ? " [built]" : "");
    snprintf(detail, detail_size, "Costs %d kindling, %d remnants.",
             kindling, remnants);
    *enabled = !house_has_build(&s_state, build) &&
               s_state.kindling >= kindling &&
               s_state.remnants >= remnants;
    if (build == HOUSE_BUILD_ANCHOR_LINE &&
        (!house_has_build(&s_state, HOUSE_BUILD_GUEST_ROOM) ||
         !house_has_build(&s_state, HOUSE_BUILD_WORKTABLE))) {
      *enabled = false;
      snprintf(detail, detail_size, "Needs guest room and worktable.");
    }
    return;
  }

  if (s_view == VIEW_GUESTS) {
    if (index == 0) {
      snprintf(label, label_size, "More gatherers");
      snprintf(detail, detail_size, "%u gathering; yields kindling.",
               s_state.gatherers);
      *enabled = s_state.listeners > 0;
    } else {
      snprintf(label, label_size, "More listeners");
      snprintf(detail, detail_size, "%u listening; yields clarity.",
               s_state.listeners);
      *enabled = s_state.gatherers > 0;
    }
    return;
  }

  if (s_view == VIEW_DOOR) {
    if (s_state.expedition_active) {
      snprintf(label, label_size, "Continue journey");
      snprintf(detail, detail_size, "The Crooked Hall still holds.");
    } else {
      snprintf(label, label_size, "Enter the Drift");
      snprintf(detail, detail_size, "Load 2 rations and 4 clarity.");
      *enabled = house_can_expedition(&s_state);
    }
    return;
  }

  if (s_view == VIEW_EXPEDITION) {
    if (index == 0) {
      snprintf(label, label_size, "Step forward");
      snprintf(detail, detail_size, "Each step spends clarity.");
      *enabled = s_state.encounter_strength == 0;
    } else {
      snprintf(label, label_size, "Retreat");
      snprintf(detail, detail_size, "Carry half the remnants home.");
    }
    return;
  }

  if (s_view == VIEW_ENCOUNTER) {
    if (index == 0) {
      snprintf(label, label_size, "Name the detail");
      snprintf(detail, detail_size, "Make one true thing remain.");
    } else {
      snprintf(label, label_size, "Flee");
      snprintf(detail, detail_size, "Return with half of what held.");
    }
    return;
  }

  snprintf(label, label_size, "Return");
  snprintf(detail, detail_size, "Back to the house.");
}

static void prv_draw_text(GContext *ctx, const char *text, GFont font,
                          GColor color, GRect rect, GTextAlignment alignment) {
  graphics_context_set_text_color(ctx, color);
  graphics_draw_text(ctx, text, font, rect, GTextOverflowModeWordWrap,
                     alignment, NULL);
}

static void prv_draw_list(GContext *ctx, GRect bounds, int16_t top) {
  const int16_t count = prv_item_count();
  const bool is_large = bounds.size.w >= 200;
  const int16_t row_height = is_large ? 29 : 25;
  const int16_t visible = is_large ? 4 : 3;
  int16_t first = s_selected - visible / 2;
  if (first < 0) {
    first = 0;
  }
  if (first + visible > count) {
    first = count - visible;
  }
  if (first < 0) {
    first = 0;
  }

  for (int16_t row = 0; row < visible && first + row < count; ++row) {
    const int16_t index = first + row;
    char label[32];
    char detail[64];
    bool enabled = false;
    prv_item_text(index, label, sizeof(label), detail, sizeof(detail), &enabled);
    const bool selected = index == s_selected;
    const int16_t y = top + row * row_height;

    if (selected) {
      graphics_context_set_fill_color(ctx, s_color_accent);
      graphics_fill_rect(ctx, GRect(4, y, bounds.size.w - 8, row_height - 2),
                         3, GCornersAll);
    }
    const GColor color = selected ? s_color_background
                                  : (enabled ? s_color_text : s_color_locked);
    const GFont font = fonts_get_system_font(selected
        ? FONT_KEY_GOTHIC_18_BOLD : FONT_KEY_GOTHIC_18);
    prv_draw_text(ctx, label, font, color,
                  GRect(9, y - 1, bounds.size.w - 18, row_height),
                  GTextAlignmentLeft);
  }

  char label[32];
  char detail[64];
  bool enabled = false;
  prv_item_text(s_selected, label, sizeof(label), detail, sizeof(detail),
                &enabled);
  (void)enabled;
  const char *footer = s_notice[0] ? s_notice : detail;
  prv_draw_text(ctx, footer, fonts_get_system_font(FONT_KEY_GOTHIC_14),
                s_notice[0] ? s_color_accent : s_color_muted,
                GRect(5, bounds.size.h - 31, bounds.size.w - 10, 30),
                GTextAlignmentCenter);
}

static void prv_canvas_update(Layer *layer, GContext *ctx) {
  const GRect bounds = layer_get_bounds(layer);
  graphics_context_set_fill_color(ctx, s_color_background);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  prv_draw_text(ctx, prv_title(),
                fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD),
                s_color_accent, GRect(4, 0, bounds.size.w - 8, 24),
                GTextAlignmentCenter);

  char status[96];
  if (s_view == VIEW_EXPEDITION || s_view == VIEW_ENCOUNTER) {
    snprintf(status, sizeof(status),
             "Step %u/4   C:%u   R:%u\nResolve:%u   Carry:%u",
             s_state.expedition_step, s_state.expedition_clarity,
             s_state.expedition_rations, s_state.expedition_resolve,
             s_state.cargo_remnants);
  } else if (s_view == VIEW_CHRONICLE) {
    snprintf(status, sizeof(status),
             "The red door was labeled beneath the paint.\n"
             "The same mark waits below the hearth.");
  } else {
    snprintf(status, sizeof(status),
             "Fire %u/5   Guests %u\nK:%d   M:%d   R:%d   C:%d",
             s_state.hearth_level, s_state.residents, s_state.kindling,
             s_state.remnants, s_state.rations, s_state.clarity);
  }

  prv_draw_text(ctx, status, fonts_get_system_font(FONT_KEY_GOTHIC_14),
                s_color_text, GRect(5, 24, bounds.size.w - 10,
                                    s_view == VIEW_CHRONICLE ? 66 : 42),
                GTextAlignmentCenter);
  const bool is_large = bounds.size.w >= 200;
  const int16_t list_top = s_view == VIEW_CHRONICLE
      ? (is_large ? 112 : 93)
      : (is_large ? 78 : 68);
  prv_draw_list(ctx, bounds, list_top);
}

static void prv_show_result(HouseResult result, const char *success) {
  switch (result) {
    case HOUSE_RESULT_OK:
      prv_show_notice(success);
      break;
    case HOUSE_RESULT_LOCKED:
      prv_show_notice("Something else must come first.");
      break;
    case HOUSE_RESULT_NO_RESOURCES:
      prv_show_notice("Not enough remains here.");
      break;
    case HOUSE_RESULT_AT_LIMIT:
      prv_show_notice("Nothing more changes.");
      break;
    case HOUSE_RESULT_NOT_ACTIVE:
      prv_show_notice("No path is being held.");
      break;
    case HOUSE_RESULT_ENCOUNTER:
      prv_show_notice("The echo strikes back.");
      break;
    case HOUSE_RESULT_COMPLETE:
      prv_show_notice("A memory returns with you.");
      break;
    case HOUSE_RESULT_FAILED:
      prv_show_notice("The Drift unmade the path.");
      break;
  }
}

static void prv_activate_home(void) {
  switch (prv_home_item_at(s_selected)) {
    case HOME_HEARTH:
      prv_set_view(VIEW_HEARTH);
      break;
    case HOME_WORKSHOP:
      prv_set_view(VIEW_WORKSHOP);
      break;
    case HOME_GUESTS:
      prv_set_view(VIEW_GUESTS);
      break;
    case HOME_DOOR:
      prv_set_view(VIEW_DOOR);
      break;
    case HOME_CHRONICLE:
      prv_set_view(VIEW_CHRONICLE);
      break;
  }
}

static void prv_activate_hearth(void) {
  if (s_selected == 0) {
    const int16_t before = s_state.remnants;
    const HouseResult result = house_search(&s_state);
    prv_show_result(result, s_state.remnants > before
        ? "A remnant beneath the boards." : "Dry wood. Still useful.");
  } else if (s_selected == 1) {
    const uint8_t before = s_state.residents;
    const HouseResult result = house_tend_hearth(&s_state);
    prv_show_result(result, s_state.residents > before
        ? "Someone knocks at the door." : "The room holds its shape.");
  } else {
    prv_show_result(house_prepare_ration(&s_state),
                    "A ration wrapped and ready.");
  }
  prv_save();
}

static void prv_activate_workshop(void) {
  static const char *const messages[] = {
      "A second room becomes real.",
      "The worktable remembers its use.",
      "A line vanishes through the door."};
  prv_show_result(house_construct(&s_state, (HouseBuild)s_selected),
                  messages[s_selected]);
  prv_save();
}

static void prv_activate_guests(void) {
  const HouseRole role = s_selected == 0
      ? HOUSE_ROLE_GATHERER : HOUSE_ROLE_LISTENER;
  prv_show_result(house_assign(&s_state, role),
                  role == HOUSE_ROLE_GATHERER
                      ? "A guest tests the room edges."
                      : "A guest listens beyond the walls.");
  prv_save();
}

static void prv_activate_door(void) {
  if (s_state.expedition_active) {
    prv_set_view(s_state.encounter_strength > 0
        ? VIEW_ENCOUNTER : VIEW_EXPEDITION);
    return;
  }

  const HouseResult result = house_start_expedition(&s_state);
  if (result == HOUSE_RESULT_OK) {
    prv_set_view(VIEW_EXPEDITION);
    prv_show_notice("The front step hangs in white.");
  } else {
    prv_show_result(result, "");
  }
  prv_save();
}

static void prv_activate_expedition(void) {
  if (s_selected == 1) {
    prv_show_result(house_expedition_retreat(&s_state),
                    "You return with what held.");
    prv_set_view(VIEW_DOOR);
    prv_save();
    return;
  }

  const HouseResult result = house_expedition_advance(&s_state);
  if (result == HOUSE_RESULT_ENCOUNTER) {
    prv_set_view(VIEW_ENCOUNTER);
    prv_show_notice("It insists: there was never a room.");
  } else if (result == HOUSE_RESULT_COMPLETE) {
    prv_set_view(VIEW_CHRONICLE);
    prv_show_notice("Memory held: The Crooked Hall.");
  } else if (result == HOUSE_RESULT_FAILED) {
    prv_set_view(VIEW_DOOR);
    prv_show_result(result, "");
  } else if (result == HOUSE_RESULT_OK) {
    prv_show_notice(s_state.expedition_step == 1
        ? "The porch stretches into white." : "A red door waits ahead.");
  } else {
    prv_show_result(result, "");
  }
  prv_save();
}

static void prv_activate_encounter(void) {
  if (s_selected == 1) {
    house_expedition_retreat(&s_state);
    prv_set_view(VIEW_DOOR);
    prv_show_notice("The echo follows you to the threshold.");
    prv_save();
    return;
  }

  const HouseResult result = house_expedition_remember(&s_state);
  if (result == HOUSE_RESULT_OK) {
    prv_set_view(VIEW_EXPEDITION);
    prv_show_notice("Brass latch. Worn carpet. Both remain.");
  } else if (result == HOUSE_RESULT_FAILED) {
    prv_set_view(VIEW_DOOR);
    prv_show_result(result, "");
  } else {
    prv_show_result(result, "");
  }
  prv_save();
}

static void prv_select_click(ClickRecognizerRef recognizer, void *context) {
  (void)recognizer;
  (void)context;
  switch (s_view) {
    case VIEW_HOME:
      prv_activate_home();
      break;
    case VIEW_HEARTH:
      prv_activate_hearth();
      break;
    case VIEW_WORKSHOP:
      prv_activate_workshop();
      break;
    case VIEW_GUESTS:
      prv_activate_guests();
      break;
    case VIEW_DOOR:
      prv_activate_door();
      break;
    case VIEW_EXPEDITION:
      prv_activate_expedition();
      break;
    case VIEW_ENCOUNTER:
      prv_activate_encounter();
      break;
    case VIEW_CHRONICLE:
      prv_set_view(VIEW_HOME);
      break;
  }
}

static void prv_up_click(ClickRecognizerRef recognizer, void *context) {
  (void)recognizer;
  (void)context;
  if (s_selected > 0) {
    s_selected--;
    layer_mark_dirty(s_canvas);
  }
}

static void prv_down_click(ClickRecognizerRef recognizer, void *context) {
  (void)recognizer;
  (void)context;
  if (s_selected + 1 < prv_item_count()) {
    s_selected++;
    layer_mark_dirty(s_canvas);
  }
}

static void prv_back_click(ClickRecognizerRef recognizer, void *context) {
  (void)recognizer;
  (void)context;
  if (s_view == VIEW_HOME) {
    window_stack_pop(true);
  } else if (s_view == VIEW_ENCOUNTER) {
    prv_set_view(VIEW_EXPEDITION);
  } else if (s_view == VIEW_EXPEDITION) {
    prv_set_view(VIEW_DOOR);
  } else {
    prv_set_view(VIEW_HOME);
  }
}

static void prv_click_config(void *context) {
  (void)context;
  window_single_repeating_click_subscribe(BUTTON_ID_UP, 180, prv_up_click);
  window_single_click_subscribe(BUTTON_ID_SELECT, prv_select_click);
  window_single_repeating_click_subscribe(BUTTON_ID_DOWN, 180, prv_down_click);
  window_single_click_subscribe(BUTTON_ID_BACK, prv_back_click);
}

static void prv_tick(struct tm *tick_time, TimeUnits units_changed) {
  (void)tick_time;
  (void)units_changed;
  if (house_apply_elapsed(&s_state, time(NULL))) {
    prv_save();
    layer_mark_dirty(s_canvas);
  }
}

static void prv_window_load(Window *window) {
  Layer *root = window_get_root_layer(window);
  s_canvas = layer_create(layer_get_bounds(root));
  layer_set_update_proc(s_canvas, prv_canvas_update);
  layer_add_child(root, s_canvas);
}

static void prv_window_unload(Window *window) {
  (void)window;
  layer_destroy(s_canvas);
  s_canvas = NULL;
}

static void prv_init(void) {
  if (!prv_load()) {
    house_state_init(&s_state, time(NULL));
    snprintf(s_notice, sizeof(s_notice),
             "The house moves over nothing.");
  } else {
    house_apply_elapsed(&s_state, time(NULL));
  }

  s_view = VIEW_HOME;
  s_selected = 0;
  s_window = window_create();
  window_set_background_color(s_window, s_color_background);
  window_set_click_config_provider(s_window, prv_click_config);
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = prv_window_load,
    .unload = prv_window_unload
  });
  window_stack_push(s_window, true);
  tick_timer_service_subscribe(SECOND_UNIT, prv_tick);
}

static void prv_deinit(void) {
  tick_timer_service_unsubscribe();
  if (s_notice_timer) {
    app_timer_cancel(s_notice_timer);
    s_notice_timer = NULL;
  }
  house_apply_elapsed(&s_state, time(NULL));
  prv_save();
  window_destroy(s_window);
}

int main(void) {
  prv_init();
  app_event_loop();
  prv_deinit();
}
