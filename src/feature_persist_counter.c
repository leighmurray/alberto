#include <pebble.h>
#include <sys/time.h>
  
#define KEY_START_TIME_1 0
#define KEY_END_TIME_1 1
#define KEY_TITLE_1 2
#define KEY_START_TIME_2 3
#define KEY_END_TIME_2 4
#define KEY_TITLE_2 5
  
static Window *s_main_window;
static TextLayer *time_layer;
static TextLayer *title_layer;

static double full_day = 86400;

time_t day_start_time;

time_t day_end_time;

bool using_google_time = false;
bool showPercentage = false;

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  // Use a long-lived buffer
  static char s_uptime_buffer[32];
  
  if (using_google_time) {
    
    if (difftime(day_start_time, day_end_time) == 0) {
      char* rest_text = "Now";
      snprintf(s_uptime_buffer, sizeof(rest_text) +  1, "%s", rest_text);
    } else {
    
      time_t current_time = time(NULL);
      
      double event_progress = difftime(current_time, day_start_time);
      
      APP_LOG(APP_LOG_LEVEL_INFO, "current stamp: %ld - start stamp: %ld - diff: %d", current_time, day_start_time, (int)event_progress);
      
      if (event_progress < 0) {
        event_progress *= -1;
        //char* rest_text = "Rest";
        //snprintf(s_uptime_buffer, sizeof(rest_text) +  1, "%s", rest_text);
        int seconds = (int)event_progress % 60;
        int minutes = (int)event_progress / 60 % 60;
        int hours = (int)event_progress / 3600;
        if (hours < 1) {
          snprintf(s_uptime_buffer, 6, "%02d:%02d", minutes, seconds);
        } else {
          snprintf(s_uptime_buffer, 9, "%02d:%02d\n%02d", hours, minutes, seconds);
        }
      } else {
        double event_total_duration = difftime(day_end_time, day_start_time);
        if (showPercentage) {
          double percent =  (event_progress / event_total_duration) * 100;
          snprintf(s_uptime_buffer, sizeof(percent), "%d%%", (int)percent);
        } else {
          double remaining_time = event_total_duration - event_progress;
          int seconds = (int)remaining_time % 60;
          int minutes = (int)remaining_time / 60 % 60;
          int hours = (int)remaining_time / 3600;
          
          if(hours < 1) {
  		      snprintf(s_uptime_buffer, 6, "%02d:%02d", minutes, seconds);
        	} else {
        		snprintf(s_uptime_buffer, 9, "%02d:%02d\n%02d", hours, minutes, seconds);
        	}
  
        }   
      }
    }
  } else {
    double work_day_diff = difftime(day_start_time, day_end_time);
  
    if (work_day_diff < 0 ) { // the start time is before the end time. Only possible if we are not within the work day. that is, we should rest.
      char* rest_text = "Rest";
      snprintf(s_uptime_buffer, sizeof(rest_text) +  1, "%s", rest_text);
      //snprintf(s_uptime_buffer, sizeof(work_day_diff), "%d%%", (int)work_day_diff);
    } else { // else we are within the work day because we have passed the start time but are yet to pass the end time.
      
      double work_day_total = full_day - work_day_diff; // This gives us the duration of the work day in seconds. coz I'm awesome.
      
      double day_progress = full_day + difftime(time(NULL), day_start_time); // this gives us how far into the day we are.
      
      double percent = (day_progress / work_day_total) * 100;
    
      // Update the TextLayer
      snprintf(s_uptime_buffer, sizeof(percent), "%d%%", (int)percent);
    }
  }
  
  text_layer_set_text(time_layer, s_uptime_buffer);
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);
  
  // Create output TextLayer
  time_layer = text_layer_create(GRect(0, 56, window_bounds.size.w, window_bounds.size.h));
  
  text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
  text_layer_set_background_color(time_layer, GColorClear);
  text_layer_set_text_color(time_layer, GColorWhite);
  //text_layer_set_text(time_layer, "Uptime: 0h 0m 0s");
  
  text_layer_set_font(time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(time_layer));
  
  title_layer = text_layer_create(GRect(0, 0, window_bounds.size.w, window_bounds.size.h / 3));
  text_layer_set_text_alignment(title_layer, GTextAlignmentCenter);
  text_layer_set_background_color(title_layer, GColorClear);
  text_layer_set_text_color(title_layer, GColorWhite);
  text_layer_set_font(title_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(title_layer));
}

static void main_window_unload(Window *window) {
  // Destroy output TextLayer
  text_layer_destroy(time_layer);
  text_layer_destroy(title_layer);
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  Tuple *t = dict_read_first(iterator);
  
  static char title_buffer[32];
  
  //APP_LOG(APP_LOG_LEVEL_INFO, "Recieved some shizz!");
  
  // For all items
  while(t != NULL) {
    //APP_LOG(APP_LOG_LEVEL_INFO, "IT'S NOT NULL");
    // Which key was received?
    switch(t->key) {
      case KEY_TITLE_1:
        //APP_LOG(APP_LOG_LEVEL_INFO, "Setting Title");
        snprintf(title_buffer, sizeof(title_buffer), "%s", t->value->cstring);
        text_layer_set_text(title_layer, title_buffer);
        break;
      case KEY_START_TIME_1:
          //APP_LOG(APP_LOG_LEVEL_INFO, "Setting Start Time");
          day_start_time = (time_t)t->value->int32;
          //APP_LOG(APP_LOG_LEVEL_INFO, "Set Start Time to: %ld", day_start_time);
          break;
      case KEY_END_TIME_1:
          //APP_LOG(APP_LOG_LEVEL_INFO, "Setting End Time");
          day_end_time = (time_t)t->value->int32;
          //APP_LOG(APP_LOG_LEVEL_INFO, "Set End Time");
        break;
      default:
        APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
        break;
    }

    // Look for next item
    t = dict_read_next(iterator);
  }
  using_google_time = true;
}

void request_events_update () {
    // Begin dictionary
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  
  // Add a key-value pair
  dict_write_uint8(iter, 0, 0);
  
  // Send the message!
  app_message_outbox_send();
}

void down_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  request_events_update();
}

void config_provider(Window *window) {
 // single click / repeat-on-hold config:
  window_single_click_subscribe(BUTTON_ID_SELECT, down_single_click_handler);
  //window_single_repeating_click_subscribe(BUTTON_ID_SELECT, 1000, select_single_click_handler);

  // multi click config:
  //window_multi_click_subscribe(BUTTON_ID_SELECT, 2, 10, 0, true, select_multi_click_handler);

  // long click config:
  //window_long_click_subscribe(BUTTON_ID_SELECT, 700, select_long_click_handler, select_long_click_release_handler);
}

static void init(void) {
  day_start_time = clock_to_timestamp(TODAY, 8, 30);
  day_end_time = clock_to_timestamp(TODAY, 17, 30);
  
  // Create main Window
  s_main_window = window_create();
  window_set_fullscreen(s_main_window, true);
  window_set_background_color(s_main_window, GColorBlack);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_stack_push(s_main_window, true);
  
  window_set_click_config_provider(s_main_window, (ClickConfigProvider) config_provider);

  // Subscribe to TickTimerService
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
  
  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  
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