#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_uptime_layer;

static double full_day = 86400;

static time_t day_start_time;

static time_t day_end_time;

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  // Use a long-lived buffer
  static char s_uptime_buffer[32];

  day_start_time = clock_to_timestamp(TODAY, 8, 30);
  day_end_time = clock_to_timestamp(TODAY, 17, 30);
  
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
  s_uptime_layer = text_layer_create(GRect(0, 56, window_bounds.size.w, window_bounds.size.h));
  
  text_layer_set_text_alignment(s_uptime_layer, GTextAlignmentCenter);
  text_layer_set_background_color(s_uptime_layer, GColorClear);
  text_layer_set_text_color(s_uptime_layer, GColorWhite);
  //text_layer_set_text(s_uptime_layer, "Uptime: 0h 0m 0s");
  
  text_layer_set_font(s_uptime_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(s_uptime_layer));
}

static void main_window_unload(Window *window) {
  // Destroy output TextLayer
  text_layer_destroy(s_uptime_layer);
}

void out_sent_handler(DictionaryIterator *sent, void *context) {
   // outgoing message was delivered
}

void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
  // outgoing message failed
}

void in_dropped_handler(AppMessageResult reason, void *context) {
  // incoming message dropped
}

void in_received_handler(DictionaryIterator *iter, void *context) {
  // incoming message received

  Tuple *number_tuple = dict_find(iter, 0);

  int headerLength = strlen(number_tuple->value->cstring) + 1;
  char header[headerLength];
  strncpy(header, number_tuple->value->cstring, headerLength);

  // Check for fields you expect to receive
  Tuple *text_tuple = dict_find(iter, 1);

  //APP_LOG(APP_LOG_LEVEL_DEBUG, "Header: %s", header);

  if (strcmp(header, "set_date_format") == 0) {
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "Setting Date Format: %d", text_tuple->value->uint8);
    //set_date_format(text_tuple->value->uint8);
  }
  else if (strcmp(header, "set_signal") == 0) {
  	int returned_value = text_tuple->value->uint8;
    //set_signal(returned_value == 1 ? true : false);
  }
  else if (strcmp(header, "set_vibrate_on_bt") == 0) {
  	int returned_value = text_tuple->value->uint8;
  	//set_vibrate_on_bt(returned_value == 1 ? true : false);
  }
  else if (strcmp(header, "error") == 0) {
  	///text_layer_set_text(status_layer, text_tuple->value->cstring);
  }
}

static void init(void) {
  // Create main Window
  s_main_window = window_create();
  window_set_background_color(s_main_window, GColorBlack);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_stack_push(s_main_window, true);
  
  // Subscribe to TickTimerService
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
  
  app_message_register_inbox_received(in_received_handler);
  app_message_register_inbox_dropped(in_dropped_handler);
  app_message_register_outbox_sent(out_sent_handler);
  app_message_register_outbox_failed(out_failed_handler);
  
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
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