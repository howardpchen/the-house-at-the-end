#include <pebble.h>

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "house_state.h"

#define PERSIST_KEY_STATE 1
#define MIN_SUPPORTED_STATE_SCHEMA 1
#define NOTICE_DURATION_MS 4200
#define SEARCH_DURATION_MS 2000
#define SEARCH_UPDATE_MS 100
#define DEBUG_LONG_CLICK_MS 1000
#define DEBUG_RESOURCE_STEP 10

typedef enum {
  VIEW_HOME = 0,
  VIEW_HEARTH,
  VIEW_WORKSHOP,
  VIEW_GUESTS,
  VIEW_DOOR,
  VIEW_EXPEDITION,
  VIEW_ENCOUNTER,
  VIEW_CHRONICLE,
  VIEW_DEBUG,
  VIEW_DEBUG_EDIT,
  VIEW_DEBUG_RESET
} AppView;

typedef enum {
  HOME_HEARTH = 0,
  HOME_WORKSHOP,
  HOME_GUESTS,
  HOME_DOOR,
  HOME_CHRONICLE
} HomeItem;

typedef enum {
  DEBUG_KINDLING = 0,
  DEBUG_REMNANTS,
  DEBUG_RATIONS,
  DEBUG_CLARITY,
  DEBUG_FIRE,
  DEBUG_GUESTS,
  DEBUG_GATHERERS,
  DEBUG_RESET,
  DEBUG_ITEM_COUNT
} DebugItem;

typedef struct {
  uint16_t schema;
  uint16_t state_size;
  HouseState state;
  uint32_t checksum;
} PersistedState;

_Static_assert(sizeof(HouseState) == 40,
               "In-place migrations require the original state size");
_Static_assert(sizeof(PersistedState) <= 256,
               "The save record must fit one Pebble persistence value");

static Window *s_window;
static Layer *s_canvas;
static AppTimer *s_notice_timer;
static AppTimer *s_search_timer;
static HouseState s_state;
static AppView s_view;
static AppView s_debug_return_view;
static DebugItem s_debug_item;
static int16_t s_selected;
static uint16_t s_search_progress_ms;
static bool s_searching;
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
  const bool is_supported = record.schema >= MIN_SUPPORTED_STATE_SCHEMA &&
                            record.schema <= HOUSE_STATE_SCHEMA;
  const bool needs_migration = record.schema != HOUSE_STATE_SCHEMA;
  if (!is_supported ||
      record.state_size != sizeof(HouseState) ||
      record.checksum != expected) {
    APP_LOG(APP_LOG_LEVEL_WARNING, "Ignoring invalid state record");
    return false;
  }

  if (record.schema == 1) {
    record.state.hearth_elapsed = 0;
  }
  if (needs_migration) {
    record.state.gather_progress %= HOUSE_KINDLING_PER_REMNANT;
  }
  if (!house_state_is_valid(&record.state)) {
    APP_LOG(APP_LOG_LEVEL_WARNING, "Ignoring invalid game state");
    return false;
  }

  s_state = record.state;
  if (needs_migration) {
    prv_save();
  }
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

static void prv_clear_notice(void) {
  if (s_notice_timer) {
    app_timer_cancel(s_notice_timer);
    s_notice_timer = NULL;
  }
  s_notice[0] = '\0';
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

static const char *prv_hearth_name(void) {
  switch (s_state.hearth_level) {
    case 0:
      return "Cold";
    case HOUSE_HEARTH_LIT:
      return "Lit";
    case HOUSE_HEARTH_SEEN:
      return "Seen";
    case HOUSE_HEARTH_HELD:
    case 4:
      return "Held";
    case HOUSE_HEARTH_SHARED:
      return "Shared";
  }
  return "Cold";
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
    case VIEW_DEBUG_EDIT:
    case VIEW_DEBUG_RESET:
      return 1;
    case VIEW_DEBUG:
      return DEBUG_ITEM_COUNT;
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
    case VIEW_DEBUG:
      return "TEST MENU";
    case VIEW_DEBUG_EDIT:
      return "TEST EDIT";
    case VIEW_DEBUG_RESET:
      return "RESET GAME";
  }
  return "THE HOUSE";
}

static void prv_item_text(int16_t index, char *label, size_t label_size,
                          char *detail, size_t detail_size, bool *enabled) {
  *enabled = true;
  label[0] = '\0';
  detail[0] = '\0';

  if (s_view == VIEW_DEBUG) {
    switch ((DebugItem)index) {
      case DEBUG_KINDLING:
        snprintf(label, label_size, "Kindling: %d", s_state.kindling);
        break;
      case DEBUG_REMNANTS:
        snprintf(label, label_size, "Remnants: %d", s_state.remnants);
        break;
      case DEBUG_RATIONS:
        snprintf(label, label_size, "Rations: %d", s_state.rations);
        break;
      case DEBUG_CLARITY:
        snprintf(label, label_size, "Clarity: %d", s_state.clarity);
        break;
      case DEBUG_FIRE:
        snprintf(label, label_size, "Fire: %u", s_state.hearth_level);
        break;
      case DEBUG_GUESTS:
        snprintf(label, label_size, "Guests: %u", s_state.residents);
        break;
      case DEBUG_GATHERERS:
        snprintf(label, label_size, "Gatherers: %u", s_state.gatherers);
        break;
      case DEBUG_RESET:
        snprintf(label, label_size, "Reset game");
        break;
      case DEBUG_ITEM_COUNT:
        break;
    }
    if (index <= DEBUG_CLARITY) {
      snprintf(detail, detail_size, "SELECT edits in steps of 10.");
    } else if (index == DEBUG_GATHERERS) {
      snprintf(detail, detail_size, "Listeners adjust automatically.");
    } else if (index == DEBUG_RESET) {
      snprintf(detail, detail_size, "Requires a second SELECT.");
    } else {
      snprintf(detail, detail_size, "SELECT edits one at a time.");
    }
    return;
  }

  if (s_view == VIEW_DEBUG_EDIT) {
    int value = 0;
    const char *name = "Value";
    int step = 1;
    switch (s_debug_item) {
      case DEBUG_KINDLING:
        name = "Kindling";
        value = s_state.kindling;
        step = DEBUG_RESOURCE_STEP;
        break;
      case DEBUG_REMNANTS:
        name = "Remnants";
        value = s_state.remnants;
        step = DEBUG_RESOURCE_STEP;
        break;
      case DEBUG_RATIONS:
        name = "Rations";
        value = s_state.rations;
        step = DEBUG_RESOURCE_STEP;
        break;
      case DEBUG_CLARITY:
        name = "Clarity";
        value = s_state.clarity;
        step = DEBUG_RESOURCE_STEP;
        break;
      case DEBUG_FIRE:
        name = "Fire";
        value = s_state.hearth_level;
        break;
      case DEBUG_GUESTS:
        name = "Guests";
        value = s_state.residents;
        break;
      case DEBUG_GATHERERS:
        name = "Gatherers";
        value = s_state.gatherers;
        break;
      case DEBUG_RESET:
      case DEBUG_ITEM_COUNT:
        break;
    }
    snprintf(label, label_size, "%s: %d", name, value);
    snprintf(detail, detail_size, "UP/DOWN %d. BACK saves.", step);
    return;
  }

  if (s_view == VIEW_DEBUG_RESET) {
    snprintf(label, label_size, "Erase all progress");
    snprintf(detail, detail_size, "SELECT confirms. BACK cancels.");
    return;
  }

  if (s_view == VIEW_HOME) {
    switch (prv_home_item_at(index)) {
      case HOME_HEARTH:
        snprintf(label, label_size, "Hearth");
        snprintf(detail, detail_size, "Search, feed, and prepare.");
        return;
      case HOME_WORKSHOP:
        snprintf(label, label_size, "Workshop");
        if (s_state.hearth_level < HOUSE_HEARTH_HELD) {
          snprintf(detail, detail_size, "Needs Held Fire (3).");
          *enabled = false;
        } else {
          snprintf(detail, detail_size, "Make the house more real.");
        }
        return;
      case HOME_GUESTS:
        snprintf(label, label_size, "Guests");
        if (s_state.hearth_level < HOUSE_HEARTH_SHARED) {
          snprintf(detail, detail_size, "Needs Shared Fire (5).");
          *enabled = false;
        } else {
          snprintf(detail, detail_size, "%u here: %u gather, %u listen.",
                   s_state.residents, s_state.gatherers, s_state.listeners);
        }
        return;
      case HOME_DOOR:
        snprintf(label, label_size, "Front door");
        if (!s_state.expedition_active &&
            s_state.hearth_level < HOUSE_HEARTH_SHARED) {
          snprintf(detail, detail_size, "Needs Shared Fire (5).");
          *enabled = false;
        } else {
          snprintf(detail, detail_size,
                   "The anchor line disappears outward.");
        }
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
      snprintf(detail, detail_size, "2 kindling. %s Fire %u/5.",
               prv_hearth_name(), s_state.hearth_level);
      *enabled = s_state.hearth_level < HOUSE_HEARTH_MAX &&
                 s_state.kindling >= 2;
    } else {
      snprintf(label, label_size, "Prepare ration");
      if (s_state.hearth_level < HOUSE_HEARTH_HELD) {
        snprintf(detail, detail_size, "Needs Held Fire (3).");
        *enabled = false;
      } else {
        snprintf(detail, detail_size, "Costs 1 kindling and 1 remnant.");
        *enabled = house_has_build(&s_state, HOUSE_BUILD_WORKTABLE) &&
                   s_state.kindling >= 1 && s_state.remnants >= 1;
      }
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
    const bool is_built = house_has_build(&s_state, build);
    snprintf(label, label_size, "%s", names[index]);
    if (is_built) {
      snprintf(detail, detail_size, "Built.");
    } else {
      snprintf(detail, detail_size, "Costs %d kindling, %d remnants.",
               kindling, remnants);
    }
    *enabled = !is_built &&
               s_state.kindling >= kindling &&
               s_state.remnants >= remnants;
    if (s_state.hearth_level < HOUSE_HEARTH_HELD) {
      *enabled = false;
      snprintf(detail, detail_size, "Needs Held Fire (3).");
    } else if (build == HOUSE_BUILD_ANCHOR_LINE &&
               s_state.hearth_level < HOUSE_HEARTH_SHARED) {
      *enabled = false;
      snprintf(detail, detail_size, "Needs Shared Fire (5).");
    } else if (build == HOUSE_BUILD_ANCHOR_LINE &&
        (!house_has_build(&s_state, HOUSE_BUILD_GUEST_ROOM) ||
         !house_has_build(&s_state, HOUSE_BUILD_WORKTABLE))) {
      *enabled = false;
      snprintf(detail, detail_size, "Needs guest room and worktable.");
    }
    return;
  }

  if (s_view == VIEW_GUESTS) {
    if (s_state.hearth_level < HOUSE_HEARTH_SHARED) {
      snprintf(label, label_size, "The guests wait");
      snprintf(detail, detail_size, "Needs Shared Fire (5).");
      *enabled = false;
      return;
    }
    if (index == 0) {
      snprintf(label, label_size, "More gatherers");
      snprintf(detail, detail_size, "%u gathering; kindling + remnants.",
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
      if (s_state.hearth_level < HOUSE_HEARTH_SHARED) {
        snprintf(detail, detail_size, "Needs Shared Fire (5).");
        *enabled = false;
      } else {
        snprintf(detail, detail_size, "Load 2 rations and 4 clarity.");
        *enabled = house_can_expedition(&s_state);
      }
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
  const int16_t row_height = is_large ? 34 : 32;
  const bool showing_notice = s_notice[0] != '\0';
  const int16_t visible = is_large ? (showing_notice ? 2 : 3) : 1;
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
        ? FONT_KEY_GOTHIC_24_BOLD : FONT_KEY_GOTHIC_24);
    prv_draw_text(ctx, label, font, color,
                  GRect(9, y - 1, bounds.size.w - 18, row_height),
                  GTextAlignmentLeft);
  }

  if (s_searching) {
    const int16_t bar_width = bounds.size.w - 20;
    const GRect bar = GRect(10, bounds.size.h - 14, bar_width, 8);
    const int16_t fill_width =
        (int16_t)((bar_width - 4) * s_search_progress_ms /
                  SEARCH_DURATION_MS);
    prv_draw_text(ctx, "Searching rooms...",
                  fonts_get_system_font(FONT_KEY_GOTHIC_18),
                  s_color_accent,
                  GRect(5, bounds.size.h - 42, bounds.size.w - 10, 26),
                  GTextAlignmentCenter);
    graphics_context_set_stroke_color(ctx, s_color_accent);
    graphics_draw_rect(ctx, bar);
    if (fill_width > 0) {
      graphics_context_set_fill_color(ctx, s_color_accent);
      graphics_fill_rect(ctx,
                         GRect(bar.origin.x + 2, bar.origin.y + 2,
                               fill_width, bar.size.h - 4),
                         0, GCornerNone);
    }
    return;
  }

  char label[32];
  char detail[64];
  bool enabled = false;
  prv_item_text(s_selected, label, sizeof(label), detail, sizeof(detail),
                &enabled);
  (void)enabled;
  const char *footer = showing_notice ? s_notice : detail;
  const int16_t footer_height = showing_notice
      ? (is_large ? 64 : 90) : (is_large ? 43 : 42);
  const GFont footer_font = fonts_get_system_font(showing_notice
      ? FONT_KEY_GOTHIC_24 : FONT_KEY_GOTHIC_18);
  prv_draw_text(ctx, footer, footer_font,
                showing_notice ? s_color_accent : s_color_muted,
                GRect(5, bounds.size.h - footer_height,
                      bounds.size.w - 10, footer_height),
                GTextAlignmentCenter);
}

static void prv_canvas_update(Layer *layer, GContext *ctx) {
  const GRect bounds = layer_get_bounds(layer);
  graphics_context_set_fill_color(ctx, s_color_background);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  prv_draw_text(ctx, prv_title(),
                fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD),
                s_color_accent, GRect(4, 0, bounds.size.w - 8, 30),
                GTextAlignmentCenter);

  const bool is_large = bounds.size.w >= 200;
  char status[96];
  if (s_view == VIEW_EXPEDITION || s_view == VIEW_ENCOUNTER) {
    if (is_large) {
      snprintf(status, sizeof(status),
               "Step %u/4   C:%u   R:%u\nResolve:%u   Carry:%u",
               s_state.expedition_step, s_state.expedition_clarity,
               s_state.expedition_rations, s_state.expedition_resolve,
               s_state.cargo_remnants);
    } else {
      snprintf(status, sizeof(status),
               "Step %u/4\nC:%u R:%u Resolve:%u\nCarry:%u",
               s_state.expedition_step, s_state.expedition_clarity,
               s_state.expedition_rations, s_state.expedition_resolve,
               s_state.cargo_remnants);
    }
  } else if (s_view == VIEW_CHRONICLE) {
    snprintf(status, sizeof(status),
             "Red door: a buried name.\n"
             "Mark below the hearth.");
  } else {
    if (is_large) {
      snprintf(status, sizeof(status),
               "Fire %u/5   Guests %u\nK:%d  M:%d  R:%d  C:%d",
               s_state.hearth_level, s_state.residents, s_state.kindling,
               s_state.remnants, s_state.rations, s_state.clarity);
    } else {
      snprintf(status, sizeof(status),
               "Fire %u/5  Guests %u\nK:%d  M:%d\nR:%d  C:%d",
               s_state.hearth_level, s_state.residents, s_state.kindling,
               s_state.remnants, s_state.rations, s_state.clarity);
    }
  }

  const bool compact_notice = !is_large && s_notice[0] != '\0';
  if (!compact_notice) {
    const int16_t status_height = is_large ? 46 : 62;
    prv_draw_text(ctx, status, fonts_get_system_font(FONT_KEY_GOTHIC_18),
                  s_color_text,
                  GRect(5, 28, bounds.size.w - 10, status_height),
                  GTextAlignmentCenter);
  }
  const int16_t list_top = compact_notice ? 34 : (is_large ? 78 : 90);
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

static void prv_search_tick(void *context) {
  (void)context;
  s_search_timer = NULL;
  const uint16_t next = s_search_progress_ms + SEARCH_UPDATE_MS;
  s_search_progress_ms = next > SEARCH_DURATION_MS
      ? SEARCH_DURATION_MS : next;

  if (s_search_progress_ms >= SEARCH_DURATION_MS) {
    s_searching = false;
    const int16_t before = s_state.remnants;
    const HouseResult result = house_search(&s_state);
    prv_show_result(result, s_state.remnants > before
        ? "A remnant beneath the boards." : "Dry wood. Still useful.");
    prv_save();
    return;
  }

  layer_mark_dirty(s_canvas);
  s_search_timer = app_timer_register(SEARCH_UPDATE_MS, prv_search_tick, NULL);
  if (!s_search_timer) {
    s_searching = false;
    prv_show_notice("The room will not hold still.");
  }
}

static void prv_start_search(void) {
  if (s_searching) {
    return;
  }
  prv_clear_notice();
  s_search_progress_ms = 0;
  s_searching = true;
  s_search_timer = app_timer_register(SEARCH_UPDATE_MS, prv_search_tick, NULL);
  if (!s_search_timer) {
    s_searching = false;
    prv_show_notice("The room will not hold still.");
    return;
  }
  layer_mark_dirty(s_canvas);
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
    prv_start_search();
    return;
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

static int16_t prv_adjusted_value(int16_t value, int32_t amount,
                                  int16_t minimum, int16_t maximum) {
  const int32_t adjusted = value + amount;
  if (adjusted < minimum) {
    return minimum;
  }
  if (adjusted > maximum) {
    return maximum;
  }
  return (int16_t)adjusted;
}

static void prv_debug_adjust(int direction) {
  const int32_t resource_amount = direction * DEBUG_RESOURCE_STEP;
  switch (s_debug_item) {
    case DEBUG_KINDLING:
      s_state.kindling = prv_adjusted_value(
          s_state.kindling, resource_amount, 0, HOUSE_RESOURCE_MAX);
      break;
    case DEBUG_REMNANTS:
      s_state.remnants = prv_adjusted_value(
          s_state.remnants, resource_amount, 0, HOUSE_RESOURCE_MAX);
      break;
    case DEBUG_RATIONS:
      s_state.rations = prv_adjusted_value(
          s_state.rations, resource_amount, 0, HOUSE_RESOURCE_MAX);
      break;
    case DEBUG_CLARITY:
      s_state.clarity = prv_adjusted_value(
          s_state.clarity, resource_amount, 0, HOUSE_RESOURCE_MAX);
      break;
    case DEBUG_FIRE:
      s_state.hearth_level = (uint8_t)prv_adjusted_value(
          s_state.hearth_level, direction, 0, HOUSE_HEARTH_MAX);
      s_state.hearth_elapsed = 0;
      break;
    case DEBUG_GUESTS: {
      const uint8_t residents = (uint8_t)prv_adjusted_value(
          s_state.residents, direction, 0, 2);
      if (residents > s_state.residents) {
        s_state.gatherers += residents - s_state.residents;
        s_state.story_flags |= HOUSE_STORY_FIRST_GUEST;
      } else if (s_state.gatherers > residents) {
        s_state.gatherers = residents;
      }
      s_state.residents = residents;
      s_state.listeners = residents - s_state.gatherers;
      break;
    }
    case DEBUG_GATHERERS:
      s_state.gatherers = (uint8_t)prv_adjusted_value(
          s_state.gatherers, direction, 0, s_state.residents);
      s_state.listeners = s_state.residents - s_state.gatherers;
      break;
    case DEBUG_RESET:
    case DEBUG_ITEM_COUNT:
      return;
  }
  prv_save();
  layer_mark_dirty(s_canvas);
}

static void prv_activate_debug(void) {
  s_debug_item = (DebugItem)s_selected;
  if (s_selected == DEBUG_RESET) {
    prv_set_view(VIEW_DEBUG_RESET);
    return;
  }
  prv_set_view(VIEW_DEBUG_EDIT);
}

static void prv_return_to_debug(void) {
  s_view = VIEW_DEBUG;
  s_selected = s_debug_item;
  layer_mark_dirty(s_canvas);
}

static void prv_confirm_debug_reset(void) {
  house_state_init(&s_state, time(NULL));
  prv_save();
  s_debug_return_view = VIEW_HOME;
  prv_set_view(VIEW_HOME);
  prv_show_notice("The house forgets everything.");
}

static void prv_select_click(ClickRecognizerRef recognizer, void *context) {
  (void)recognizer;
  (void)context;
  if (s_searching) {
    return;
  }
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
    case VIEW_DEBUG:
      prv_activate_debug();
      break;
    case VIEW_DEBUG_EDIT:
      break;
    case VIEW_DEBUG_RESET:
      prv_confirm_debug_reset();
      break;
  }
}

static void prv_up_click(ClickRecognizerRef recognizer, void *context) {
  (void)recognizer;
  (void)context;
  if (s_searching) {
    return;
  }
  if (s_view == VIEW_DEBUG_EDIT) {
    prv_debug_adjust(1);
    return;
  }
  if (s_selected > 0) {
    s_selected--;
    layer_mark_dirty(s_canvas);
  }
}

static void prv_down_click(ClickRecognizerRef recognizer, void *context) {
  (void)recognizer;
  (void)context;
  if (s_searching) {
    return;
  }
  if (s_view == VIEW_DEBUG_EDIT) {
    prv_debug_adjust(-1);
    return;
  }
  if (s_selected + 1 < prv_item_count()) {
    s_selected++;
    layer_mark_dirty(s_canvas);
  }
}

static void prv_back_click(ClickRecognizerRef recognizer, void *context) {
  (void)recognizer;
  (void)context;
  if (s_searching) {
    return;
  }
  if (s_view == VIEW_DEBUG_EDIT || s_view == VIEW_DEBUG_RESET) {
    prv_return_to_debug();
  } else if (s_view == VIEW_DEBUG) {
    prv_set_view(s_debug_return_view);
  } else if (s_view == VIEW_HOME) {
    window_stack_pop(true);
  } else if (s_view == VIEW_ENCOUNTER) {
    prv_set_view(VIEW_EXPEDITION);
  } else if (s_view == VIEW_EXPEDITION) {
    prv_set_view(VIEW_DOOR);
  } else {
    prv_set_view(VIEW_HOME);
  }
}

static void prv_select_long_click(ClickRecognizerRef recognizer,
                                  void *context) {
  (void)recognizer;
  (void)context;
  if (s_searching || s_view == VIEW_DEBUG || s_view == VIEW_DEBUG_EDIT ||
      s_view == VIEW_DEBUG_RESET) {
    return;
  }
  s_debug_return_view = s_view;
  prv_clear_notice();
  prv_set_view(VIEW_DEBUG);
}

static void prv_click_config(void *context) {
  (void)context;
  window_single_repeating_click_subscribe(BUTTON_ID_UP, 180, prv_up_click);
  window_single_click_subscribe(BUTTON_ID_SELECT, prv_select_click);
  window_long_click_subscribe(BUTTON_ID_SELECT, DEBUG_LONG_CLICK_MS,
                              prv_select_long_click, NULL);
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
  if (s_search_timer) {
    app_timer_cancel(s_search_timer);
    s_search_timer = NULL;
  }
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
