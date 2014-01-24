#include <pebble.h>

Window *window;
TextLayer *text_hour_layer;
TextLayer *text_min_layer;
TextLayer *text_date_layer;
TextLayer *text_day_layer;
Layer *line_layer;
Layer *box_layer;

void line_layer_update_callback(Layer *layer, GContext* ctx) {
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
}

void box_layer_update_callback(Layer *layer, GContext* ctx) {
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
}



// Update Time and date.
void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
	
  //Handle Date
	static char date_text[] = "XXXXXXXXX 00";
	static char day_text[] = "Mm";
	int i;
	
  strftime(date_text, sizeof(date_text), "%B %e", tick_time);
	for (i=0; date_text[i]; i++) {
  		if(date_text[i]>='a' && date_text[i]<='z') date_text[i] -= 32;
	}
  text_layer_set_text(text_date_layer, date_text);
  strftime(day_text, sizeof(day_text), "%a", tick_time);
  text_layer_set_text(text_day_layer, day_text);
	
  // Handle Time
    static char time_text[] = "00:00";
	static char min_text[] = "  ";
	static char hour_text[] = "  ";
	char *time_format;
	
  if (clock_is_24h_style()) {
    time_format = "%R";
  } else {
    time_format = "%I:%M";
  }

  strftime(time_text, sizeof(time_text), time_format, tick_time);

  // Handle lack of non-padded hour format string
  // for twelve hour clock and split time into hours/mins.
  if (!clock_is_24h_style() && (time_text[0] == '0')) {
	  int l = sizeof(hour_text)/sizeof(hour_text[0]);
	  for (int i=0; i < l; i++){
		  hour_text[i] = 0;
	  }
      memmove(time_text, &time_text[1], sizeof(time_text) - 1);
	  memmove(hour_text, &time_text[0], 1);
	  memmove(min_text, &time_text[2], 2);
  } else{
	  memmove(hour_text, &time_text[0], 2);
	  memmove(min_text, &time_text[3], 2);
  }
	
  text_layer_set_text(text_hour_layer, hour_text);
  text_layer_set_text(text_min_layer, min_text);
}

void handle_deinit(void) {
	
    tick_timer_service_unsubscribe();
	text_layer_destroy(text_date_layer);
	text_layer_destroy(text_hour_layer);
	text_layer_destroy(text_min_layer);
	layer_destroy(line_layer);
	layer_destroy(box_layer);
	window_destroy(window);
}

void handle_init(void) {
	
  // Create the window	
  window = window_create();
  window_stack_push(window, true /* Animated */);
  window_set_background_color(window, GColorBlack);

  Layer *window_layer = window_get_root_layer(window);
	
  ////Text Layers
  
  //Date Layer
  text_date_layer = text_layer_create(GRect(8, 10, 144-8, 168-10));
  text_layer_set_text_color(text_date_layer, GColorWhite);
  text_layer_set_background_color(text_date_layer, GColorClear);
  text_layer_set_font(text_date_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_HELVETICA_LIGHT_18)));
  layer_add_child(window_layer, text_layer_get_layer(text_date_layer));
	
  //Box Layer
	
  GRect box_frame = GRect(0, 91, 144, 168-91);
  box_layer = layer_create(box_frame);
  layer_set_update_proc(box_layer, box_layer_update_callback);
  layer_add_child(window_layer, box_layer);
	
  //Hour Layer
  text_hour_layer = text_layer_create(GRect(0, 91, 80, 168-91));
  text_layer_set_text_color(text_hour_layer, GColorBlack);
  text_layer_set_background_color(text_hour_layer, GColorClear);
	text_layer_set_text_alignment(text_hour_layer, GTextAlignmentRight);
  text_layer_set_font(text_hour_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_HELVETICA_LIGHT_65)));
  layer_add_child(window_layer, text_layer_get_layer(text_hour_layer));

  //Minutes Layer
  text_min_layer = text_layer_create(GRect(90, 91, 144-90, 168-91));
  text_layer_set_text_color(text_min_layer, GColorBlack);
  text_layer_set_background_color(text_min_layer, GColorClear);
	text_layer_set_text_alignment(text_min_layer, GTextAlignmentCenter);
  text_layer_set_font(text_min_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_HELVETICA_LIGHT_43)));
  layer_add_child(window_layer, text_layer_get_layer(text_min_layer));
	
  //Day of week
  text_day_layer = text_layer_create(GRect(90, 140, 144-90, 168-140));
  text_layer_set_text_color(text_day_layer, GColorBlack);
  text_layer_set_background_color(text_day_layer, GColorClear);
  text_layer_set_text_alignment(text_day_layer, GTextAlignmentCenter);
  text_layer_set_font(text_day_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_HELVETICA_LIGHT_18)));
  layer_add_child(window_layer, text_layer_get_layer(text_day_layer));

  //Line
  GRect line_frame = GRect(8, 38, 139, 2);
  line_layer = layer_create(line_frame);
  layer_set_update_proc(line_layer, line_layer_update_callback);
  layer_add_child(window_layer, line_layer);

  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
	
  // Avoids a blank screen on watch start.
  time_t now = time(NULL);
  struct tm *tick_time = localtime(&now);
  handle_minute_tick(tick_time, MINUTE_UNIT);
  
}


int main(void) {
  handle_init();

  app_event_loop();
  
  handle_deinit();
}