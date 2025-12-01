#include <pebble.h>
#include <stdio.h>

#include "atlas.h"
#include "render.h"
#include "settings.h"

static GBitmap *s_number_font;
static GBitmap *s_outline_font;
static GBitmap *s_text_font;

static Window *s_window;
static GBitmap *s_time_bitmap;
static BitmapLayer *s_time;
static Layer *s_outlines;
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

static void handle_tick(struct tm *tick_time, TimeUnits units_changed) {
    render_time(s_time, s_number_font);
    layer_mark_dirty(s_top_text);
    layer_mark_dirty(s_bottom_text);
}

static void handle_battery_update(BatteryChargeState charge) {
    layer_mark_dirty(s_top_text);
    layer_mark_dirty(s_bottom_text);
}

static void handle_unobstructed_area_change(AnimationProgress progress, void *context) {
    GRect bounds = layer_get_unobstructed_bounds(window_get_root_layer(s_window));

    Layer *time_layer = bitmap_layer_get_layer(s_time);
    layer_set_bounds(time_layer, bounds);

    GRect outline_frame = GRect(0, 0, 144, 110);
    grect_align(&outline_frame, &bounds, GAlignCenter, false);
    layer_set_frame(s_outlines, outline_frame);
}

void apply_settings(void) {
    static GColor text_palette[2];
    text_palette[0] = g_settings.palette[0];
    text_palette[1] = g_settings.text_color;

    static GColor outline_palette[2];
    outline_palette[0] = GColorClear;
    outline_palette[1] = g_settings.outline_color;

    window_set_background_color(s_window, g_settings.palette[0]);
    gbitmap_set_palette(s_text_font, text_palette, false);
    gbitmap_set_palette(s_time_bitmap, g_settings.palette, false);
    gbitmap_set_palette(s_outline_font, outline_palette, false);

    layer_mark_dirty(bitmap_layer_get_layer(s_time));
    layer_mark_dirty(s_top_text);
    layer_mark_dirty(s_bottom_text);
    if (g_settings.enable_outlines) {
        layer_set_hidden(s_outlines, false);
        layer_mark_dirty(s_outlines);
    } else {
        layer_set_hidden(s_outlines, true);
    }
}

static void window_load(Window *window) {
    Layer *root = window_get_root_layer(window);
    GRect bounds = layer_get_unobstructed_bounds(root);

    s_top_text = layer_create(GRect(4, 4, PBL_DISPLAY_WIDTH - 8, 20));
    layer_set_update_proc(s_top_text, top_text_layer_update);
    layer_add_child(root, s_top_text);

    s_bottom_text = layer_create(GRect(4, PBL_DISPLAY_HEIGHT - 24, PBL_DISPLAY_WIDTH - 8, 20));
    layer_set_update_proc(s_bottom_text, bottom_text_layer_update);
    layer_add_child(root, s_bottom_text);

    s_time_bitmap = gbitmap_create_blank(GSize(144, 108), GBitmapFormat2BitPalette);
    static GColor palette[4];
    palette[0] = GColorBlack;
    palette[1] = GColorOrange;
    palette[2] = GColorBrightGreen;
    palette[3] = GColorYellow;
    gbitmap_set_palette(s_time_bitmap, palette, false);

    s_time = bitmap_layer_create(bounds);
    bitmap_layer_set_bitmap(s_time, s_time_bitmap);
    layer_add_child(root, bitmap_layer_get_layer(s_time));

    s_outlines = layer_create(GRect(0, 30, 144, 110));
    layer_set_update_proc(s_outlines, outlines_layer_update);
    layer_add_child(root, s_outlines);

    tick_timer_service_subscribe(MINUTE_UNIT, handle_tick);
    handle_tick(NULL, MINUTE_UNIT);
    battery_state_service_subscribe(handle_battery_update);
    unobstructed_area_service_subscribe((UnobstructedAreaHandlers){.change = handle_unobstructed_area_change}, NULL);

    handle_unobstructed_area_change(0, NULL);

    apply_settings();
}

static void window_unload(Window *window) {
    layer_destroy(s_top_text);
    layer_destroy(s_bottom_text);
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
    s_text_font = gbitmap_create_with_resource(RESOURCE_ID_TEXT_FONT);

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
    gbitmap_destroy(s_text_font);
}

int main(void) {
    app_init();
    app_event_loop();
    app_deinit();
}
