#include "render.h"

const int number_dx = 12;
const int number_dy = 18;

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

    time_t now_ = time(NULL);
    struct tm *now = localtime(&now_);
    int hour_tens = now->tm_hour / 10;
    int hour_ones = now->tm_hour % 10;
    int minute_tens = now->tm_min / 10;
    int minute_ones = now->tm_min % 10;

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
                    p |= get_1bpp_pixel(font_data, hour_tens * font_width + px, py, font_stride) << 1;
                } else if (x >= font_width + number_dx && x < font_width * 2 + number_dx) {
                    int px = x - font_width - number_dx;
                    int py = y;
                    p |= get_1bpp_pixel(font_data, hour_ones * font_width + px, py, font_stride) << 1;
                }
            }
            if (y >= number_dy) {
                if (x >= number_dx && x < font_width + number_dx) {
                    int px = x - number_dx;
                    int py = y - number_dy;
                    p |= get_1bpp_pixel(font_data, minute_tens * font_width + px, py, font_stride);
                } else if (x >= font_width + number_dx * 2) {
                    int px = x - font_width - number_dx * 2;
                    int py = y - number_dy;
                    p |= get_1bpp_pixel(font_data, minute_ones * font_width + px, py, font_stride);
                }
            }

            set_2bpp_pixel(data, x, y, stride, p);
        }
    }

    layer_mark_dirty(bitmap_layer_get_layer(layer));
}
