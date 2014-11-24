#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_uptime_layer;

static int s_uptime = 0;

static double full_day = 86400;

static time_t day_start_time;

static time_t day_end_time;

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  // Use a long-lived buffer
  static char s_uptime_buffer[32];

  day_start_time = clock_to_timestamp(TODAY, 22, 00);
  day_end_time = clock_to_timestamp(TODAY, 22, 30);
  
  double work_day_diff = difftime(day_start_time, day_end_time);
  
  if (work_day_diff < 0 ) { // the start time is before the end time. Only possible if we are not within the work day. that is, we should rest.
    char* rest_text = "Rest";
    snprintf(s_uptime_buffer, sizeof(rest_text) +  1, "%s", rest_text);
    //snprintf(s_uptime_buffer, sizeof(work_day_diff), "%d%%", (int)work_day_diff);
  } else { // else we are within the work day because we have past the start time but are yet to pass the end time.
    
    double work_day_total = full_day - work_day_diff; // This gives us the duration of the work day in seconds. coz I'm awesome.
    
    double day_progress = full_day + difftime(time(NULL), day_start_time); // this gives us how far into the day we are.
    
    double percent = (day_progress / work_day_total) * 100;
  
    // Update the TextLayer
    snprintf(s_uptime_buffer, sizeof(percent), "%d%%", (int)percent);
  }
   
  text_layer_set_text(s_uptime_layer, s_uptime_buffer);
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);

  // Create output TextLayer
  s_uptime_layer = text_layer_create(GRect(0, 45, window_bounds.size.w, window_bounds.size.h));
  
  text_layer_set_text_alignment(s_uptime_layer, GTextAlignmentCenter);
  
  //text_layer_set_text(s_uptime_layer, "Uptime: 0h 0m 0s");
  
  text_layer_set_font(s_uptime_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(s_uptime_layer));
}

static void main_window_unload(Window *window) {
  // Destroy output TextLayer
  text_layer_destroy(s_uptime_layer);
}

static void init(void) {
  // Create main Window
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_stack_push(s_main_window, true);
  
  // Subscribe to TickTimerService
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
}

static void deinit(void) {
  // Destroy main Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}