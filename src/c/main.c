#include <pebble.h>
#include <stdio.h>

#include "atlas.h"
#include "render.h"
#include "settings.h"

#ifdef PBL_RECT
#define ENABLE_TEXT
#endif

#define TEXT_HEIGHT 20
#if PBL_DISPLAY_HEIGHT > 180
#define TEXT_INSET 10
#else
#define TEXT_INSET 4
#endif

static GBitmap *s_number_font;
static GBitmap *s_outline_font;

static Window *s_window;
static GBitmap *s_time_bitmap;
static BitmapLayer *s_time;
static Layer *s_outlines;

#ifdef ENABLE_TEXT
static GBitmap *s_text_font;
static Layer *s_top_text;
static Layer *s_bottom_text;

static GRect get_atlas(char c) {
    if (c >= 'a' && c <= 'z') {
        c += 'A' - 'a';
    }
    for (unsigned int i = 0; i < ARRAY_LENGTH(TEXT_FONT_ATLAS); i++) {
        if (TEXT_FONT_ATLAS[i].character == c) {
            return TEXT_FONT_ATLAS[i].rect;
        }
    }
    return GRectZero;
}

static void draw_text(Layer *layer, GContext *ctx, char *text, bool right) {
    GRect bounds = layer_get_bounds(layer);
    int x = 0;
    size_t len = strlen(text);

    for (size_t i = 0; i < len; i++) {
        size_t j = right ? len - i - 1 : i;
        if (text[j] == ' ') {
            x += 8;
            continue;
        }

        GRect src_rect = get_atlas(text[j]);
        gbitmap_set_bounds(s_text_font, src_rect);
        GRect dest_rect = right
            ? GRect(bounds.size.w - x - src_rect.size.w, 0, src_rect.size.w, src_rect.size.h)
            : GRect(x, 0, src_rect.size.w, src_rect.size.h);
        graphics_draw_bitmap_in_rect(ctx, s_text_font, dest_rect);

        x += src_rect.size.w + 2;
    }
}

// Returned string is valid until next call.
static char *get_text(TextMode mode) {
    static char buf[32];
    time_t now_;
    struct tm *now;
    BatteryChargeState charge_state;
    int steps;

    switch (mode) {
    case TextModeNone:
        return "";
    case TextModeWeekdayDay:
        now_ = time(NULL);
        now = localtime(&now_);
        strftime(buf, sizeof(buf), "%a %d", now);
        return buf;
    case TextModeDayMonth:
        now_ = time(NULL);
        now = localtime(&now_);
        strftime(buf, sizeof(buf), "%d/%m", now);
        return buf;
    case TextModeBattery:
        charge_state = battery_state_service_peek();
        snprintf(buf, sizeof(buf), "%d%%", charge_state.charge_percent);
        return buf;
    case TextModeAMPM:
        now_ = time(NULL);
        now = localtime(&now_);
        return now->tm_hour >= 12 ? "PM" : "AM";
    case TextModeSteps:
        steps = health_service_sum_today(HealthMetricStepCount);
        snprintf(buf, sizeof(buf), "%d", steps);
        return buf;
    }

    return "";
}

static void top_text_layer_update(Layer *layer, GContext *ctx) {
    draw_text(layer, ctx, get_text(g_settings.top_text), false);
}

static void bottom_text_layer_update(Layer *layer, GContext *ctx) {
    draw_text(layer, ctx, get_text(g_settings.bottom_text), true);
}
#endif

static void handle_tick(struct tm *tick_time, TimeUnits units_changed) {
    render_time(s_time, s_number_font);
#ifdef ENABLE_TEXT
    layer_mark_dirty(s_top_text);
    layer_mark_dirty(s_bottom_text);
#endif
}

#ifdef ENABLE_TEXT
static void handle_battery_update(BatteryChargeState charge) {
    layer_mark_dirty(s_top_text);
    layer_mark_dirty(s_bottom_text);
}
#endif

static void handle_unobstructed_area_change(AnimationProgress progress, void *context) {
    GRect bounds = layer_get_unobstructed_bounds(window_get_root_layer(s_window));

    Layer *time_layer = bitmap_layer_get_layer(s_time);
    layer_set_bounds(time_layer, bounds);

    GRect outline_frame = GRect(0, 0, 144, 110);
    grect_align(&outline_frame, &bounds, GAlignCenter, false);
    layer_set_frame(s_outlines, outline_frame);
}

void apply_settings(void) {
#ifdef ENABLE_TEXT
    static GColor text_palette[2];
#ifdef PBL_COLOR
    text_palette[0] = GColorClear;
    text_palette[1] = g_settings.text_color;
#else
    text_palette[0] = g_settings.dither[0] ? GColorWhite : GColorBlack;
    text_palette[1] = g_settings.dither[0] ? GColorBlack : GColorWhite;
#endif // PBL_COLOR
#endif // ENABLE_TEXT

    static GColor outline_palette[2];
    outline_palette[0] = GColorClear;
    outline_palette[1] = g_settings.outline_color;

#ifdef PBL_COLOR
    gbitmap_set_palette(s_time_bitmap, g_settings.palette, false);
    window_set_background_color(s_window, g_settings.palette[0]);
#else
    window_set_background_color(s_window, g_settings.dither[0] ? GColorWhite : GColorBlack);
#endif
#ifdef ENABLE_TEXT
    gbitmap_set_palette(s_text_font, text_palette, false);
#endif
    gbitmap_set_palette(s_outline_font, outline_palette, false);

    if (g_settings.enable_outlines) {
        layer_set_hidden(s_outlines, false);
        layer_mark_dirty(s_outlines);
    } else {
        layer_set_hidden(s_outlines, true);
    }
#ifdef PBL_COLOR
    // Color screens use a paletted bitmap, no need to do a full redraw
    layer_mark_dirty(bitmap_layer_get_layer(s_time));
#ifdef ENABLE_TEXT
    layer_mark_dirty(s_top_text);
    layer_mark_dirty(s_bottom_text);
#endif // ENABLE_TEXT
#else
    // Black/white should do a full redraw
    handle_tick(NULL, MINUTE_UNIT);
#endif
}

static void window_load(Window *window) {
    Layer *root = window_get_root_layer(window);
    GRect bounds = layer_get_unobstructed_bounds(root);

#ifdef ENABLE_TEXT
    s_top_text = layer_create(GRect(TEXT_INSET, TEXT_INSET, PBL_DISPLAY_WIDTH - TEXT_INSET * 2, TEXT_HEIGHT));
    layer_set_update_proc(s_top_text, top_text_layer_update);
    layer_add_child(root, s_top_text);

    s_bottom_text = layer_create(GRect(TEXT_INSET, PBL_DISPLAY_HEIGHT - TEXT_HEIGHT - TEXT_INSET, PBL_DISPLAY_WIDTH - TEXT_INSET * 2, TEXT_HEIGHT));
    layer_set_update_proc(s_bottom_text, bottom_text_layer_update);
    layer_add_child(root, s_bottom_text);
#endif

    s_time_bitmap = gbitmap_create_blank(
        PBL_DISPLAY_WIDTH > 180 ? GSize(192, 144) : GSize(144, 108),
        PBL_IF_COLOR_ELSE(GBitmapFormat2BitPalette, GBitmapFormat1Bit)
    );

#ifdef PBL_COLOR
    static GColor palette[4];
    palette[0] = GColorBlack;
    palette[1] = GColorOrange;
    palette[2] = GColorBrightGreen;
    palette[3] = GColorYellow;
    gbitmap_set_palette(s_time_bitmap, palette, false);
#endif

    s_time = bitmap_layer_create(bounds);
    bitmap_layer_set_bitmap(s_time, s_time_bitmap);
    layer_add_child(root, bitmap_layer_get_layer(s_time));

    s_outlines = layer_create(GRect(0, 30, 144, 110));
    layer_set_update_proc(s_outlines, outlines_layer_update);
    layer_add_child(root, s_outlines);

    tick_timer_service_subscribe(MINUTE_UNIT, handle_tick);
    handle_tick(NULL, MINUTE_UNIT);
#ifdef ENABLE_TEXT
    battery_state_service_subscribe(handle_battery_update);
#endif
    unobstructed_area_service_subscribe((UnobstructedAreaHandlers){.change = handle_unobstructed_area_change}, NULL);

    handle_unobstructed_area_change(0, NULL);

    apply_settings();
}

static void window_unload(Window *window) {
#ifdef ENABLE_TEXT
    layer_destroy(s_top_text);
    layer_destroy(s_bottom_text);
#endif
    bitmap_layer_destroy(s_time);
    gbitmap_destroy(s_time_bitmap);
    layer_destroy(s_outlines);
    tick_timer_service_unsubscribe();
    battery_state_service_unsubscribe();
    unobstructed_area_service_unsubscribe();
}

static void app_init(void) {
    settings_init();

    s_number_font = gbitmap_create_with_resource(RESOURCE_ID_NUMBER_FONT);
    s_outline_font = gbitmap_create_with_resource(RESOURCE_ID_OUTLINE_FONT);
#ifdef ENABLE_TEXT
    s_text_font = gbitmap_create_with_resource(RESOURCE_ID_TEXT_FONT);
#endif

    set_render_outline_font(s_outline_font);

    s_window = window_create();
    window_set_window_handlers(
        s_window,
        (WindowHandlers){
            .load = window_load,
            .unload = window_unload,
        }
    );
    window_stack_push(s_window, true);
}

static void app_deinit(void) {
    window_destroy(s_window);
    gbitmap_destroy(s_number_font);
    gbitmap_destroy(s_outline_font);
#ifdef ENABLE_TEXT
    gbitmap_destroy(s_text_font);
#endif
}

int main(void) {
    app_init();
    app_event_loop();
    app_deinit();
}
