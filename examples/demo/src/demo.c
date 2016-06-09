#include <pebble.h>
#include <pebble-activity-indicator-layer/activity-indicator-layer.h>

static Window *s_window;
static ActivityIndicatorLayer *s_activity_indicator_layer;

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  bool animating = activity_indicator_layer_get_animating(s_activity_indicator_layer);
  activity_indicator_layer_set_animating(s_activity_indicator_layer, !animating);
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  uint8_t thickness = activity_indicator_layer_get_thickness(s_activity_indicator_layer);
  if (thickness >= 10) {
    return;
  }

  activity_indicator_layer_set_thickness(s_activity_indicator_layer, thickness + 1);
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  uint8_t thickness = activity_indicator_layer_get_thickness(s_activity_indicator_layer);
  if (thickness <= 1) {
    return;
  }

  activity_indicator_layer_set_thickness(s_activity_indicator_layer, thickness - 1);
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  const GRect bounds = layer_get_bounds(window_layer);

  GRect frame = GRect(0, 0, 50, 50);
  grect_align(&frame, &bounds, GAlignCenter, false);

  s_activity_indicator_layer = activity_indicator_layer_create(frame);
  activity_indicator_layer_set_animating(s_activity_indicator_layer, true);
  layer_add_child(window_layer, (Layer *)s_activity_indicator_layer);
}

static void window_unload(Window *window) {
  activity_indicator_layer_destroy(s_activity_indicator_layer);
}

static void init(void) {
  s_window = window_create();
  window_set_click_config_provider(s_window, click_config_provider);
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(s_window, animated);
}

static void deinit(void) {
  window_destroy(s_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
