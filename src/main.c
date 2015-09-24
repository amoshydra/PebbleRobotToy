#include <pebble.h>

// watch screen constant
#ifdef PBL_ROUND
  const int SCREEN_HEIGHT = 180;
  const int SCREEN_WIDTH = 180;
#else
  const int SCREEN_HEIGHT = 168;
  const int SCREEN_WIDTH = 144;
#endif
  
// watch layer geometry
const int EYE_RADIUS = 28;
const int EYE_BORDER_SIZE = 4;
const int EYES_LEVEL = 45;
const int EYES_DISTANCE = 10;

const int MOUTH_BORDER_SIZE = 4;
const int MOUTH_LEVEL = 135;
const int MOUTH_HEIGHT = 37;
const int MOUTH_WIDTH = 130;

const int NOSE_LEVEL = 100;
const int NOSE_SIZE = 30;
const int NOSE_BORDER_SIZE = 3;

// color constant
const int  COLOR_MULTIPLING_FACTOR = 85;
const int COLOR_MULTIPLING_RANGE = 4;
const int COLOR_MAX_COMBINATION = 3;
const int COLOR_MAX_VALUE = 255;
const int  RED = 0;
const int  GREEN = 1;
const int  BLUE = 2;
const int  COLOR_PRIMARY = 0;
const int  COLOR_SECONDARY = 1;
const int  COLOR_BACKGROUND = 2;
const int COLOR_SWITCH_LEVEL = 320;
GColor COLOR_TEMPLATE[3];

// font constant
const int FONT_TIME = 0;
const int FONT_TEXT = 1;
GFont FONT_TEMPLATE[2];

// static pointer to Window variable
static Window *s_main_window;

// Layer pointers
static TextLayer *s_time_hour_layer;
static TextLayer *s_time_minute_layer;
static TextLayer *s_date_layer;
static Layer *s_canvas_layer;


static void canvas_update_proc(Layer *this_layer, GContext *ctx);
static void initialise_color_template();


#ifdef PBL_COLOR
static void get_random_color(GColor *colorPrimary, GColor *colorSecondary, GColor *colorBackground) {
  srand(time(NULL));
  int colorRGB[COLOR_MAX_COMBINATION];
  
  for (int i = 0; i < COLOR_MAX_COMBINATION; i++) {
    colorRGB[i] = rand() % COLOR_MULTIPLING_RANGE * COLOR_MULTIPLING_FACTOR;
  }
  *colorBackground = GColorFromRGB(colorRGB[RED], colorRGB[GREEN], colorRGB[BLUE]);
  
  for (int i = 0; i < COLOR_MAX_COMBINATION; i++) {
    colorRGB[i] = rand() % COLOR_MULTIPLING_RANGE * COLOR_MULTIPLING_FACTOR;
  }
  *colorPrimary = GColorFromRGB(colorRGB[RED], colorRGB[GREEN], colorRGB[BLUE]);  
  
  int colorLightness = colorRGB[RED] + colorRGB[GREEN] + colorRGB[BLUE];
  
  if (colorLightness < COLOR_SWITCH_LEVEL) {
    *colorSecondary = GColorFromRGB(COLOR_MAX_VALUE, COLOR_MAX_VALUE, COLOR_MAX_VALUE);
  } else {
    *colorSecondary = GColorFromRGB(0, 0, 0);
  }
}

static void colorChanger() {
  
  initialise_color_template();

  GColor colorSecondary = COLOR_TEMPLATE[COLOR_SECONDARY];
  GColor colorBackground = COLOR_TEMPLATE[COLOR_BACKGROUND];
  
  window_set_background_color(s_main_window, colorBackground);
  text_layer_set_text_color(s_time_hour_layer, colorSecondary);
  text_layer_set_text_color(s_time_minute_layer, colorSecondary);
  text_layer_set_text_color(s_date_layer, colorSecondary);
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);
}
#endif

static void initialise_font_template() {
  GFont fontTime;
  GFont fontText;
  
#ifdef PBL_COLOR
  fontTime = fonts_get_system_font(FONT_KEY_LECO_28_LIGHT_NUMBERS);
  fontText = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
#else
  fontTime = fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK);
  fontText = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
#endif
  
  FONT_TEMPLATE[FONT_TIME] = fontTime;
  FONT_TEMPLATE[FONT_TEXT] = fontText;
}
  
static void initialise_color_template() {
  GColor colorPrimary;
  GColor colorSecondary;
  GColor colorBackground;
  
#ifdef PBL_COLOR
  get_random_color(&colorPrimary, &colorSecondary, &colorBackground);
#else
  colorPrimary = GColorBlack;
  colorSecondary = GColorWhite;
  colorBackground = GColorBlack;
#endif
  
  COLOR_TEMPLATE[COLOR_PRIMARY] = colorPrimary;
  COLOR_TEMPLATE[COLOR_SECONDARY] = colorSecondary;
  COLOR_TEMPLATE[COLOR_BACKGROUND] = colorBackground;
}
  
// Time updater
static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Create a long-lived buffer
  static char buffer_time_hour[sizeof("00")];
  static char buffer_time_minute[sizeof("00")];
  static char buffer_date[sizeof("Wednesday")];

  // Write the current hours and minutes into the buffer 
  if(clock_is_24h_style())    strftime(buffer_time_hour, sizeof(buffer_time_hour), "%H", tick_time);
  else                        strftime(buffer_time_hour, sizeof(buffer_time_hour), "%I", tick_time);
  strftime(buffer_time_minute, sizeof(buffer_time_minute), "%M", tick_time);
  strftime(buffer_date, sizeof(buffer_date), "%A", tick_time);
  
  for (unsigned int i = 1; i < sizeof(buffer_date); i++) {
    if (buffer_date[i] > 0) {
      buffer_date[i] -= 32;
    } else {
      break;
    }
  }

  // Display this time on the TextLayer
  text_layer_set_text(s_time_hour_layer, buffer_time_hour);
  text_layer_set_text(s_time_minute_layer, buffer_time_minute);
  text_layer_set_text(s_date_layer, buffer_date);

#ifdef PBL_COLOR
  if (buffer_time_minute[1] == '0' && 
      (buffer_time_minute[0] == '3' || buffer_time_minute[0] == '0')) {
    colorChanger();
  }
#endif
}


// Layer constructor
static void text_layer_insert(Window *window, TextLayer *s_layer, GColor GCFore, GColor GCBack, GFont GFFont) {
  text_layer_set_background_color(s_layer, GCBack);
  text_layer_set_text_color(s_layer, GCFore);  
  text_layer_set_font(s_layer, GFFont);
  text_layer_set_text_alignment(s_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_layer));  
}
static void text_layer_insert_s(Window *window, TextLayer *s_layer, GFont GFFont, bool bBlackBack) {
  GColor GCFore = (bBlackBack) ? COLOR_TEMPLATE[COLOR_SECONDARY] : GColorBlack;
  text_layer_insert(window, s_layer, GCFore, GColorClear, GFFont);
}

static void canvas_update_proc(Layer *this_layer, GContext *ctx) {
  
  const int LEFT_EYE_CENTER = SCREEN_WIDTH / 2 - (EYE_RADIUS + EYES_DISTANCE);
  const int RIGHT_EYE_CENTER = SCREEN_WIDTH / 2 + (EYE_RADIUS + EYES_DISTANCE);
  
  GPoint posOfLeftEye = GPoint(LEFT_EYE_CENTER, EYES_LEVEL);
  GPoint posOfRightEye = GPoint(RIGHT_EYE_CENTER, EYES_LEVEL);

  GColor colorPrimary = COLOR_TEMPLATE[COLOR_PRIMARY];
  GColor colorSecondary = COLOR_TEMPLATE[COLOR_SECONDARY];
  
  const int MOUTH_LEFT = (SCREEN_WIDTH - MOUTH_WIDTH) / 2;
  const int MOUTH_TOP = MOUTH_LEVEL - (MOUTH_HEIGHT / 2);
  
  // Draw Geometry Border
  graphics_context_set_fill_color(ctx, colorSecondary);
  graphics_fill_circle(ctx, posOfLeftEye, EYE_RADIUS);
  graphics_fill_circle(ctx, posOfRightEye, EYE_RADIUS);
  graphics_fill_rect(ctx, GRect(MOUTH_LEFT,
                                MOUTH_TOP,
                                MOUTH_WIDTH,
                                MOUTH_HEIGHT), MOUTH_BORDER_SIZE, GCornersAll);
 
  
  // Draw nose (Triangle)
  int NOSE_STARTING_POINT = (SCREEN_WIDTH - NOSE_SIZE) / 2;
  GPoint NOSE_P1 = GPoint(NOSE_STARTING_POINT, NOSE_LEVEL);
  GPoint NOSE_P2 = GPoint(NOSE_STARTING_POINT + NOSE_SIZE, NOSE_LEVEL);
  GPoint NOSE_P3 = GPoint(SCREEN_WIDTH / 2, NOSE_LEVEL - (NOSE_SIZE * 26 / 30));
  
  graphics_context_set_stroke_color(ctx, colorSecondary);
#ifdef PBL_COLOR  
  graphics_context_set_stroke_width(ctx, NOSE_BORDER_SIZE);
#endif
  graphics_draw_line(ctx, NOSE_P1, NOSE_P2);
  graphics_draw_line(ctx, NOSE_P2, NOSE_P3);
  graphics_draw_line(ctx, NOSE_P3, NOSE_P1);
  
  
  
   // Draw nose (Triangle)
  int NOSE_LEVEL_new = NOSE_LEVEL;
  int NOSE_SIZE_new = NOSE_SIZE;
  
  NOSE_LEVEL_new -= NOSE_BORDER_SIZE;
  NOSE_SIZE_new -= NOSE_BORDER_SIZE * 4;
  
  NOSE_STARTING_POINT = (SCREEN_WIDTH - NOSE_SIZE_new) / 2;
  NOSE_P1 = GPoint(NOSE_STARTING_POINT, NOSE_LEVEL_new);
  NOSE_P2 = GPoint(NOSE_STARTING_POINT + NOSE_SIZE_new, NOSE_LEVEL_new);
  NOSE_P3 = GPoint(SCREEN_WIDTH / 2, NOSE_LEVEL_new - (NOSE_SIZE_new * 26 / 30));
  
  graphics_context_set_stroke_color(ctx, colorPrimary);
#ifdef PBL_COLOR  
  graphics_context_set_stroke_width(ctx, NOSE_BORDER_SIZE);
#endif
  graphics_draw_line(ctx, NOSE_P1, NOSE_P2);
  graphics_draw_line(ctx, NOSE_P2, NOSE_P3);
  graphics_draw_line(ctx, NOSE_P3, NOSE_P1);
  
  
  // Fill Geometry Border
  graphics_context_set_fill_color(ctx, colorPrimary);
  graphics_fill_circle(ctx, posOfLeftEye, EYE_RADIUS - EYE_BORDER_SIZE);
  graphics_fill_circle(ctx, posOfRightEye, EYE_RADIUS - EYE_BORDER_SIZE);
  graphics_fill_rect(ctx, GRect(MOUTH_LEFT + MOUTH_BORDER_SIZE,
                                MOUTH_TOP + MOUTH_BORDER_SIZE,
                                MOUTH_WIDTH - MOUTH_BORDER_SIZE * 2,
                                MOUTH_HEIGHT - MOUTH_BORDER_SIZE * 2), MOUTH_BORDER_SIZE, GCornersAll);
}

// Load and Unload
static void main_window_load(Window *window) {
  
  Layer *window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);
  
  // Create Layer
  s_canvas_layer = layer_create(GRect(0, 0, window_bounds.size.w, window_bounds.size.h));
  layer_add_child(window_layer, s_canvas_layer);

  // Set the update_proc
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);
  
  int fontSize = 30;
  int heightAdjustment = 3;
  const int LEFT_EYE_CENTER = SCREEN_WIDTH / 2 - (EYE_RADIUS + EYES_DISTANCE);
  const int RIGHT_EYE_CENTER = SCREEN_WIDTH / 2 + (EYE_RADIUS + EYES_DISTANCE);
  
  // Create time TextLayer
  s_time_hour_layer = text_layer_create(GRect(LEFT_EYE_CENTER - EYE_RADIUS, EYES_LEVEL - fontSize / 2 - heightAdjustment, EYE_RADIUS * 2, fontSize));
  text_layer_insert_s(window, s_time_hour_layer, FONT_TEMPLATE[FONT_TIME], 1);
  
  s_time_minute_layer = text_layer_create(GRect(RIGHT_EYE_CENTER - EYE_RADIUS, EYES_LEVEL - fontSize / 2 - heightAdjustment, EYE_RADIUS * 2, fontSize));
  text_layer_insert_s(window, s_time_minute_layer, FONT_TEMPLATE[FONT_TIME], 1);
  
  // Create date TextLayer
  s_date_layer = text_layer_create(GRect(0, 118, 144, 50));
  text_layer_insert_s(window, s_date_layer, FONT_TEMPLATE[FONT_TEXT], 1);
}
  
static void main_window_unload(Window *window) {
  // Destroy TextLayer
  layer_destroy(s_canvas_layer);
  text_layer_destroy(s_time_hour_layer);
  text_layer_destroy(s_time_minute_layer);
  text_layer_destroy(s_date_layer);
  
}

//// Event Handler

// Time Event Service Handler
static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  
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

// Initialise Pebble App
static void init() {
  
  // Color assignment
  initialise_color_template();
  
  // Font assignment
  initialise_font_template();
  
  // Create main Window element and assign to pointer
  s_main_window = window_create();
  
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  
  
  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  
  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());

  window_set_background_color(s_main_window, COLOR_TEMPLATE[COLOR_BACKGROUND]);
  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);

  update_time(); // Display time immediately after the window is loaded
}

// Deinitialise Pebble App
static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

// Main Function
int main(void) {
  init();
  
  // Enable app to wait for system events 
  app_event_loop();
  
  deinit();
}