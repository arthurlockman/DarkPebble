#include <pebble.h>

static Window *window;	
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static TextLayer *s_temp_layer;

//Images
static BitmapLayer *s_weather_image;
static GBitmap *s_weather_clearday;
static GBitmap *s_weather_clearnight;

//App sync variables
static AppSync s_sync;
static uint8_t s_sync_buffer[32];

enum {
	STATUS_KEY = 0,	
	TEMP_KEY = 1,
	ICON_KEY = 2
};

static void sync_changed_handler(const uint32_t key, const Tuple *new_tuple, const Tuple *old_tuple, void *context) {
	switch (key) {
	case STATUS_KEY:
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Counter (%d): %d", (int)key, (int)new_tuple->value->int32);
		break;
	case TEMP_KEY:
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Temp %d", (int)new_tuple->value->int32);
		static char buffer[] = "100°";
		snprintf (buffer, sizeof(buffer), "%d°", (int)new_tuple->value->int32);
		text_layer_set_text(s_temp_layer, buffer);
		
		if ((int)new_tuple->value->int32 > 10) {
			bitmap_layer_set_bitmap(s_weather_image, s_weather_clearnight);
		}
		break;
	}
}

static void sync_error_handler(DictionaryResult dict_error, AppMessageResult app_message_error, void *context) {
	//Error occurred.
}

static void update_time(){
	time_t temp = time(NULL);
	struct tm *tick_time = localtime(&temp);
	
	static char buffer[] = "00:00";
	strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
	text_layer_set_text(s_time_layer, buffer);
	
	static char datebuffer[] = "Jan 31, 2014";
	strftime(datebuffer, sizeof("Jan 31, 2014"), "%b %e, %Y", tick_time);
	text_layer_set_text(s_date_layer, datebuffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
	update_time();
}

static void main_window_load(Window *window) {
	//144x168
	s_time_layer = text_layer_create(GRect(0, 5, 144, 50));
	text_layer_set_background_color(s_time_layer, GColorClear);
	text_layer_set_text_color(s_time_layer, GColorWhite);
	text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
	text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
	
	s_date_layer = text_layer_create(GRect(0, 50, 144, 30));
	text_layer_set_background_color(s_date_layer, GColorClear);
	text_layer_set_text_color(s_date_layer, GColorWhite);
	text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
	text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));
	
	s_temp_layer = text_layer_create(GRect(0, 70, 144, 50));
	text_layer_set_background_color(s_temp_layer, GColorClear);
	text_layer_set_text_color(s_temp_layer, GColorWhite);
	text_layer_set_font(s_temp_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
	text_layer_set_text_alignment(s_temp_layer, GTextAlignmentCenter);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_temp_layer));
	
	//Initialize Weather Images
	s_weather_clearday = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CLEARDAY);
	s_weather_clearnight = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CLEARNIGHT);
	
	//Initialize Weather Image Display
	s_weather_image = bitmap_layer_create(GRect(0, 120, 144, 35));
	bitmap_layer_set_bitmap(s_weather_image, s_weather_clearday);
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_weather_image));
	
	update_time();
}

static void main_window_unload(Window *window) {

}

void init(void) {
	window = window_create();
	window_set_background_color(window, GColorBlack);
	window_set_window_handlers(window, (WindowHandlers) {
		.load = main_window_load,
		.unload = main_window_unload
	});
	window_stack_push(window, true);
	
	// Register AppMessage handlers
	app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
		
	// Setup initial values
	Tuplet initial_values[] = {
		TupletInteger(STATUS_KEY, 0),
		TupletInteger(TEMP_KEY, 0),
		TupletInteger(ICON_KEY, 0),
	};

	// Begin using AppSync
	app_sync_init(&s_sync, s_sync_buffer, sizeof(s_sync_buffer), initial_values, ARRAY_LENGTH(initial_values), sync_changed_handler, sync_error_handler, NULL);

	tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

void deinit(void) {
	window_destroy(window);
	
	text_layer_destroy(s_time_layer);
	text_layer_destroy(s_date_layer);
	
	gbitmap_destroy(s_weather_clearday);
	bitmap_layer_destroy(s_weather_image);
	
	app_sync_deinit(&s_sync);
}

int main( void ) {
	init();
	app_event_loop();
	deinit();
}