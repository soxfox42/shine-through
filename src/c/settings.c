#include "settings.h"

#include <pebble.h>

void apply_settings(void);

Settings g_settings;

static void default_settings(void) {
#ifdef PBL_COLOR
    g_settings.palette[0] = GColorBlack;
    g_settings.palette[1] = GColorOrange;
    g_settings.palette[2] = GColorBrightGreen;
    g_settings.palette[3] = GColorYellow;
    g_settings.text_color = GColorWhite;
#else
    g_settings.dither[0] = 0;
    g_settings.dither[1] = 2;
    g_settings.dither[2] = 3;
    g_settings.dither[3] = 4;
#endif
    g_settings.top_text = TextModeWeekdayDay;
    g_settings.bottom_text = TextModeBattery;
    g_settings.enable_outlines = false;
    g_settings.outline_color = GColorBlack;
}

static void load_settings(void) {
    default_settings();
    persist_read_data(SETTINGS_KEY, &g_settings, sizeof(g_settings));
}

static void save_settings(void) {
    persist_write_data(SETTINGS_KEY, &g_settings, sizeof(g_settings));
}

#define ColorToDither(x) (((x) >> 2 & 3) + ((x) & 1))

static void inbox_received(DictionaryIterator *iter, void *context) {
    Tuple *tuple = dict_read_first(iter);
    do {
#ifdef PBL_COLOR
        if (tuple->key == MESSAGE_KEY_BackgroundColor) {
            g_settings.palette[0] = GColorFromHEX(tuple->value->uint32);
        } else if (tuple->key == MESSAGE_KEY_MinutesColor) {
            g_settings.palette[1] = GColorFromHEX(tuple->value->uint32);
        } else if (tuple->key == MESSAGE_KEY_HoursColor) {
            g_settings.palette[2] = GColorFromHEX(tuple->value->uint32);
        } else if (tuple->key == MESSAGE_KEY_OverlapColor) {
            g_settings.palette[3] = GColorFromHEX(tuple->value->uint32);
        } else if (tuple->key == MESSAGE_KEY_TextColor) {
            g_settings.text_color = GColorFromHEX(tuple->value->uint32);
#else
        if (tuple->key == MESSAGE_KEY_BackgroundColor) {
            g_settings.dither[0] = ColorToDither(tuple->value->uint32);
        } else if (tuple->key == MESSAGE_KEY_MinutesColor) {
            APP_LOG(APP_LOG_LEVEL_DEBUG, "m %lu, %lu", tuple->value->uint32, ColorToDither(tuple->value->uint32));
            g_settings.dither[1] = ColorToDither(tuple->value->uint32);
        } else if (tuple->key == MESSAGE_KEY_HoursColor) {
            APP_LOG(APP_LOG_LEVEL_DEBUG, "h %lu, %lu", tuple->value->uint32, ColorToDither(tuple->value->uint32));
            g_settings.dither[2] = ColorToDither(tuple->value->uint32);
        } else if (tuple->key == MESSAGE_KEY_OverlapColor) {
            APP_LOG(APP_LOG_LEVEL_DEBUG, "o %lu, %lu", tuple->value->uint32, ColorToDither(tuple->value->uint32));
            g_settings.dither[3] = ColorToDither(tuple->value->uint32);
#endif
        } else if (tuple->key == MESSAGE_KEY_TopText) {
            g_settings.top_text = atoi(tuple->value->cstring);
        } else if (tuple->key == MESSAGE_KEY_BottomText) {
            g_settings.bottom_text = atoi(tuple->value->cstring);
        } else if (tuple->key == MESSAGE_KEY_EnableOutlines) {
            g_settings.enable_outlines = tuple->value->int16 != 0;
        } else if (tuple->key == MESSAGE_KEY_OutlineColor) {
            g_settings.outline_color = GColorFromHEX(tuple->value->uint32);
        }
    } while ((tuple = dict_read_next(iter)));
    apply_settings();
    save_settings();
}

void settings_init(void) {
    load_settings();

    app_message_register_inbox_received(inbox_received);
    app_message_open(128, 128);
}
