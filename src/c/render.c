#include "render.h"
#include "utils.h"

#ifndef PBL_COLOR
#include "settings.h"
#endif

#if PBL_DISPLAY_WIDTH > 180
#define NUMBER_DX 16
#define NUMBER_DY 24
#else
#define NUMBER_DX 12
#define NUMBER_DY 18
#endif

static GBitmap *s_outline_font;

#ifdef PBL_COLOR
static inline void set_2bpp_pixel(uint8_t *data, int x, int y, uint16_t stride, uint8_t color) {
    int index = y * stride + (x >> 2);
    int shift = 6 - (x << 1 & 6);
    int mask = ~(0b11 << shift);
    data[index] = (data[index] & mask) | (color << shift);
}
#else
static inline void set_1bpp_pixel(uint8_t *data, int x, int y, uint16_t stride, uint8_t color) {
    int index = y * stride + (x >> 3);
    int shift = x & 7;
    int mask = ~(0b1 << shift);
    data[index] = (data[index] & mask) | (color << shift);
}
#endif

static inline uint8_t get_1bpp_pixel(uint8_t *data, int x, int y, uint16_t stride) {
    int index = y * stride + (x >> 3);
    int shift = x & 7;
    return data[index] >> shift & 1;
}

void render_time(BitmapLayer *layer, GBitmap *font) {
    GRect font_bounds = gbitmap_get_bounds(font);
    int font_width = font_bounds.size.w / 10;
    int font_height = font_bounds.size.h;
    uint8_t *font_data = gbitmap_get_data(font);
    uint16_t font_stride = gbitmap_get_bytes_per_row(font);

    int digits[4];
    get_time_digits(digits);

    const GBitmap *bitmap = bitmap_layer_get_bitmap(layer);
    GRect bounds = gbitmap_get_bounds(bitmap);
    uint8_t *data = gbitmap_get_data(bitmap);
    uint16_t stride = gbitmap_get_bytes_per_row(bitmap);

    for (int y = 0; y < bounds.size.h; y++) {
        for (int x = 0; x < bounds.size.w; x++) {
            uint8_t p = 0;
            if (y < font_height) {
                if (x < font_width) {
                    int px = x;
                    int py = y;
                    p |= get_1bpp_pixel(font_data, digits[0] * font_width + px, py, font_stride) << 1;
                } else if (x >= font_width + NUMBER_DX && x < font_width * 2 + NUMBER_DX) {
                    int px = x - font_width - NUMBER_DX;
                    int py = y;
                    p |= get_1bpp_pixel(font_data, digits[1] * font_width + px, py, font_stride) << 1;
                }
            }
            if (y >= NUMBER_DY) {
                if (x >= NUMBER_DX && x < font_width + NUMBER_DX) {
                    int px = x - NUMBER_DX;
                    int py = y - NUMBER_DY;
                    p |= get_1bpp_pixel(font_data, digits[2] * font_width + px, py, font_stride);
                } else if (x >= font_width + NUMBER_DX * 2) {
                    int px = x - font_width - NUMBER_DX * 2;
                    int py = y - NUMBER_DY;
                    p |= get_1bpp_pixel(font_data, digits[3] * font_width + px, py, font_stride);
                }
            }

#ifdef PBL_COLOR
            set_2bpp_pixel(data, x, y, stride, p);
#else
            uint8_t dither = (x * 2 + (y & 1) * 3) & 3;
            uint8_t target = g_settings.dither[p];
            set_1bpp_pixel(data, x, y, stride, dither < target);
#endif
        }
    }

    layer_mark_dirty(bitmap_layer_get_layer(layer));
}

void outlines_layer_update(Layer *layer, GContext *ctx) {
    int digits[4];
    get_time_digits(digits);

    GRect font_bounds = gbitmap_get_bounds(s_outline_font);
    int font_width = font_bounds.size.w / 10;
    int font_height = font_bounds.size.h;

    graphics_context_set_compositing_mode(ctx, GCompOpSet);

    gbitmap_set_bounds(s_outline_font, GRect(digits[0] * font_width, 0, font_width, font_height));
    graphics_draw_bitmap_in_rect(ctx, s_outline_font, GRect(-1, 0, font_width, font_height));
    gbitmap_set_bounds(s_outline_font, GRect(digits[1] * font_width, 0, font_width, font_height));
    graphics_draw_bitmap_in_rect(ctx, s_outline_font, GRect(font_width + NUMBER_DX - 3, 0, font_width, font_height));
    gbitmap_set_bounds(s_outline_font, GRect(digits[2] * font_width, 0, font_width, font_height));
    graphics_draw_bitmap_in_rect(ctx, s_outline_font, GRect(NUMBER_DX - 1, NUMBER_DY, font_width, font_height));
    gbitmap_set_bounds(s_outline_font, GRect(digits[3] * font_width, 0, font_width, font_height));
    graphics_draw_bitmap_in_rect(ctx, s_outline_font, GRect(font_width + NUMBER_DX * 2 - 3, NUMBER_DY, font_width, font_height));

    gbitmap_set_bounds(s_outline_font, GRect(0, 0, font_width * 10, font_height));
}

void set_render_outline_font(GBitmap *font) {
    s_outline_font = font;
}
