#include "render.h"
#include "utils.h"

const int number_dx = 12;
const int number_dy = 18;

static GBitmap *s_outline_font;

static inline void set_2bpp_pixel(uint8_t *data, int x, int y, uint16_t stride, uint8_t color) {
    int index = y * stride + (x >> 2);
    int shift = 6 - (x << 1 & 6);
    int mask = ~(0b11 << shift);
    data[index] = (data[index] & mask) | (color << shift);
}

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
                } else if (x >= font_width + number_dx && x < font_width * 2 + number_dx) {
                    int px = x - font_width - number_dx;
                    int py = y;
                    p |= get_1bpp_pixel(font_data, digits[1] * font_width + px, py, font_stride) << 1;
                }
            }
            if (y >= number_dy) {
                if (x >= number_dx && x < font_width + number_dx) {
                    int px = x - number_dx;
                    int py = y - number_dy;
                    p |= get_1bpp_pixel(font_data, digits[2] * font_width + px, py, font_stride);
                } else if (x >= font_width + number_dx * 2) {
                    int px = x - font_width - number_dx * 2;
                    int py = y - number_dy;
                    p |= get_1bpp_pixel(font_data, digits[3] * font_width + px, py, font_stride);
                }
            }

            set_2bpp_pixel(data, x, y, stride, p);
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
    graphics_draw_bitmap_in_rect(ctx, s_outline_font, GRect(font_width + number_dx - 3, 0, font_width, font_height));
    gbitmap_set_bounds(s_outline_font, GRect(digits[2] * font_width, 0, font_width, font_height));
    graphics_draw_bitmap_in_rect(ctx, s_outline_font, GRect(number_dx - 1, number_dy, font_width, font_height));
    gbitmap_set_bounds(s_outline_font, GRect(digits[3] * font_width, 0, font_width, font_height));
    graphics_draw_bitmap_in_rect(ctx, s_outline_font, GRect(font_width + number_dx * 2 - 3, number_dy, font_width, font_height));

    gbitmap_set_bounds(s_outline_font, GRect(0, 0, font_width * 10, font_height));
}

void set_render_outline_font(GBitmap *font) {
    s_outline_font = font;
}
