/*
 * Trekkie — LCARS-inspired Pebble watchface
 *
 * Original by Zach Bruggeman (remixz), forked with minor mods:
 *   - Month number added to date line
 *   - Leading zero removed for 12h time
 *   - am/pm indicator added below the time
 *
 * Modernized for Pebble SDK 4.x:
 *   - Single pebble.h include replaces legacy triple-header pattern
 *   - All layers heap-allocated via create/destroy pattern
 *   - Window lifecycle split into load/unload handlers
 *   - Tick handler uses struct tm directly (no AppContextRef)
 *   - Entry point is main() / app_event_loop() / deinit()
 *   - Resource metadata moved to package.json
 */

#include <pebble.h>

/* ---------- Layout ---------- */

typedef struct {
  GRect date_rect;
  GRect time_rect;
  GRect ampm_rect;
} LayoutConfig;

static void layout_config_init(LayoutConfig *cfg, GRect bounds) {
  /* Compile-time constants tied to the background artwork.
     emery (200x228): x/y proportionally scaled from 144x168 originals. */
#if PBL_DISPLAY_WIDTH >= 200
  int x_chrome = 56, x_date = 88, y_date = 7;
#else
  int x_chrome = 40, x_date = 64, y_date = 5;
#endif

  /* The 60pt LCARS font renders at ~68px cap height.
     Centre the time on screen; anchor am/pm just below it. */
  int time_y    = bounds.size.h / 2 - 34;
  int content_w = bounds.size.w - x_chrome;

  cfg->date_rect = GRect(x_date,   y_date,         bounds.size.w - x_date, time_y - y_date);
  cfg->time_rect = GRect(x_chrome, time_y,          content_w,              68);
  cfg->ampm_rect = GRect(x_chrome, time_y + 68 + 4, content_w,              22);
}

/* ---------- Module-level state ---------- */

static Window      *s_window;
static BitmapLayer *s_background_layer;
static GBitmap     *s_background_bitmap;
static TextLayer   *s_date_layer;
static TextLayer   *s_time_layer;
static TextLayer   *s_ampm_layer;
static LayoutConfig s_layout;

/* ---------- Display logic ---------- */

static void update_display(struct tm *tick_time) {
  static char date_text[]     = "Xxx 00.00";
  static char new_date_text[] = "Xxx 00.00";
  static char time_text[]     = "00:00";
  static char ampm_text[]     = "  ";

  /* Date — redraws only when value changes (battery saver) */
  strftime(new_date_text, sizeof(new_date_text), "%a %m.%d", tick_time);
  if (strncmp(new_date_text, date_text, sizeof(date_text)) != 0) {
    strncpy(date_text, new_date_text, sizeof(date_text));
    text_layer_set_text(s_date_layer, date_text);
  }

  /* Time */
  const char *time_format = clock_is_24h_style() ? "%R" : "%I:%M";
  strftime(time_text, sizeof(time_text), time_format, tick_time);

  /* Strip leading zero in 12h mode */
  if (!clock_is_24h_style() && time_text[0] == '0') {
    memmove(time_text, &time_text[1], sizeof(time_text) - 1);
  }
  text_layer_set_text(s_time_layer, time_text);

  /* AM/PM */
  if (!clock_is_24h_style()) {
    strftime(ampm_text, sizeof(ampm_text), "%p", tick_time);
    strncpy(ampm_text, (ampm_text[0] == 'A') ? "am" : "pm", sizeof(ampm_text));
  }
  text_layer_set_text(s_ampm_layer, ampm_text);
}

/* ---------- Tick handler ---------- */

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_display(tick_time);
}

/* ---------- Window lifecycle ---------- */

static void window_load(Window *window) {
  Layer *root   = window_get_root_layer(window);
  GRect  bounds = layer_get_bounds(root);

  layout_config_init(&s_layout, bounds);

  /* Background — platform-specific image, sized to fill the screen */
#if PBL_DISPLAY_WIDTH >= 200
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND_EMERY);
#else
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
#endif
  s_background_layer = bitmap_layer_create(bounds);
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  layer_add_child(root, bitmap_layer_get_layer(s_background_layer));

  /* Date layer — top area, small bold font, centred within the background slot */
  s_date_layer = text_layer_create(s_layout.date_rect);
  text_layer_set_text_color(s_date_layer, GColorWhite);
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  text_layer_set_font(s_date_layer,
    fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_LCARS_BOLD_17)));
  layer_add_child(root, text_layer_get_layer(s_date_layer));

  /* Time layer — large LCARS digits, vertically centred */
  s_time_layer = text_layer_create(s_layout.time_rect);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_font(s_time_layer,
    fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_LCARS_60)));
  layer_add_child(root, text_layer_get_layer(s_time_layer));

  /* AM/PM layer — small bold, anchored below time */
  s_ampm_layer = text_layer_create(s_layout.ampm_rect);
  text_layer_set_text_color(s_ampm_layer, GColorWhite);
  text_layer_set_background_color(s_ampm_layer, GColorClear);
  text_layer_set_font(s_ampm_layer,
    fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_LCARS_BOLD_17)));
  layer_add_child(root, text_layer_get_layer(s_ampm_layer));

  /* Populate immediately so the face isn't blank on launch */
  time_t now = time(NULL);
  update_display(localtime(&now));
}

static void window_unload(Window *window) {
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_ampm_layer);
  bitmap_layer_destroy(s_background_layer);
  gbitmap_destroy(s_background_bitmap);
}

/* ---------- App lifecycle ---------- */

static void init(void) {
  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load   = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_window, true /* animated */);
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void deinit(void) {
  tick_timer_service_unsubscribe();
  window_destroy(s_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
  return 0;
}
