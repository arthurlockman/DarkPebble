#include <pebble.h>

static Window *window;	
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static TextLayer *s_temp_layer;
static TextLayer *s_forecast_layer;

//Images
static BitmapLayer *s_weather_image;
static GBitmap *s_weather_clearday;
static GBitmap *s_weather_clearnight;
static GBitmap *s_weather_cloudy;
static GBitmap *s_weather_fog;
static GBitmap *s_weather_pcloudyday;
static GBitmap *s_weather_pcloudynight;
static GBitmap *s_weather_rain;
static GBitmap *s_weather_snow;
static GBitmap *s_weather_wind;
static GBitmap *s_weather_sleet;

//App sync variables
static AppSync s_sync;
static uint8_t s_sync_buffer[100];

enum {
	ICON_KEY = 0,
	TEMP_KEY = 1,
	FORECAST_KEY = 2
};

enum {
	WEATHER_CLEARDAY = 1,
	WEATHER_CLEARNIGHT = 2,
	WEATHER_CLOUDY = 3,
	WEATHER_FOG = 4,
	WEATHER_PCLOUDYDAY = 5,
	WEATHER_PCLOUDYNIGHT = 6,
	WEATHER_RAIN = 7,
	WEATHER_SLEET = 8,
	WEATHER_SNOW = 9,
	WEATHER_WIND = 10
};

static void sync_changed_handler(const uint32_t key, const Tuple *new_tuple, const Tuple *old_tuple, void *context) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Key: %d", (int)key);
	switch (key) {
	case FORECAST_KEY:
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Forecast: %s", new_tuple->value->cstring);
		text_layer_set_text(s_forecast_layer, new_tuple->value->cstring);
		break;
	case TEMP_KEY:
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Temp: %d", (int)new_tuple->value->int32);
		static char buffer[] = "100°";
		snprintf (buffer, sizeof(buffer), "%d°", (int)new_tuple->value->int32);
		text_layer_set_text(s_temp_layer, buffer);
		break;
	case ICON_KEY:
		switch ((int)new_tuple->value->int32) {
		case 1:
			bitmap_layer_set_bitmap(s_weather_image, s_weather_clearday);
			break;
		case 2:
			bitmap_layer_set_bitmap(s_weather_image, s_weather_clearnight);
			break;
		case 3:
			bitmap_layer_set_bitmap(s_weather_image, s_weather_cloudy);
			break;
		case 4:
			bitmap_layer_set_bitmap(s_weather_image, s_weather_fog);
			break;
		case 5:
			bitmap_layer_set_bitmap(s_weather_image, s_weather_pcloudyday);
			break;
		case 6:
			bitmap_layer_set_bitmap(s_weather_image, s_weather_pcloudynight);
			break;
		case 7:
			bitmap_layer_set_bitmap(s_weather_image, s_weather_rain);
			break;
		case 8:
			bitmap_layer_set_bitmap(s_weather_image, s_weather_sleet);
			break;
		case 9:
			bitmap_layer_set_bitmap(s_weather_image, s_weather_snow);
			break;
		case 10:
			bitmap_layer_set_bitmap(s_weather_image, s_weather_wind);
			break;
		default:
			bitmap_layer_set_bitmap(s_weather_image, s_weather_clearday);
			break;
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
	if(clock_is_24h_style() == true) {
		strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
	} else {
		strftime(buffer, sizeof("00:00"), "%I:%M", tick_time);
	}
	text_layer_set_text(s_time_layer, buffer);
	
	//static char datebuffer[] = "Jan 31, 2014";
	//strftime(datebuffer, sizeof("Jan 31, 2014"), "%b %e, %Y", tick_time);
	//text_layer_set_text(s_date_layer, datebuffer);
}

static void update_date() {
	time_t temp = time(NULL);
	struct tm *tick_time = localtime(&temp);
	static char datebuffer[] = "Jan 31, 2014";
	strftime(datebuffer, sizeof("Jan 31, 2014"), "%b %e, %Y", tick_time);
	text_layer_set_text(s_date_layer, datebuffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
	if ((units_changed & MINUTE_UNIT) != 0) {
		update_time();
	}
	if ((units_changed & DAY_UNIT) != 0) {
		update_date();
	}
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
	
	s_temp_layer = text_layer_create(GRect(0, 74, 83, 50));
	text_layer_set_background_color(s_temp_layer, GColorClear);
	text_layer_set_text_color(s_temp_layer, GColorWhite);
	text_layer_set_font(s_temp_layer, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
	text_layer_set_text_alignment(s_temp_layer, GTextAlignmentRight);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_temp_layer));
	
	s_forecast_layer = text_layer_create(GRect(10, 115, 124, 60));
	text_layer_set_background_color(s_forecast_layer, GColorClear);
	text_layer_set_text_color(s_forecast_layer, GColorWhite);
	text_layer_set_font(s_forecast_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
	text_layer_set_text_alignment(s_forecast_layer, GTextAlignmentCenter);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_forecast_layer));
	
	//Initialize Weather Images
	s_weather_clearday = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CLEARDAY);
	s_weather_clearnight = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CLEARNIGHT);
	s_weather_fog = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_FOG);
	s_weather_pcloudyday = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_PCLOUDYDAY);
	s_weather_pcloudynight = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_PCLOUDYNIGHT);
	s_weather_rain = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_RAIN);
	s_weather_snow = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SNOW);
	s_weather_wind = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_WIND);
	s_weather_cloudy = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CLOUDY);
	s_weather_sleet = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SLEET);
	
	//Initialize Weather Image Display
	s_weather_image = bitmap_layer_create(GRect(83, 70, 40, 50));
	bitmap_layer_set_bitmap(s_weather_image, s_weather_clearday);
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_weather_image));
	
	update_time();
	update_date();
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
		TupletInteger(ICON_KEY, 0),
		TupletInteger(TEMP_KEY, 0),
		TupletCString(FORECAST_KEY, "Clear for the day."),
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