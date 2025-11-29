#include "render.h"

const int number_dx = 12;
const int number_dy = 18;

static inline void set_pixel(uint8_t *data, int x, int y, uint16_t stride, uint8_t color) {
    int index = y * stride + (x >> 2);
    int shift = 6 - (x << 1 & 6);
    int mask = ~(0b11 << shift);
    data[index] = (data[index] & mask) | (color << shift);
}

static inline uint8_t get_font_pixel(GBitmap *font, int n, int x, int y) {
    GRect bounds = gbitmap_get_bounds(font);
    uint8_t *data = gbitmap_get_data(font);
    uint16_t stride = gbitmap_get_bytes_per_row(font);

    x += n * bounds.size.w / 10;

    int index = y * stride + (x >> 3);
    int shift = x & 7;
    return data[index] >> shift & 1;
}

void render_time(BitmapLayer *layer, GBitmap *font) {
    GRect font_bounds = gbitmap_get_bounds(font);
    int font_width = font_bounds.size.w / 10;
    int font_height = font_bounds.size.h;

    time_t now_ = time(NULL);
    struct tm *now = localtime(&now_);

    const GBitmap *bitmap = bitmap_layer_get_bitmap(layer);
    GRect bounds = gbitmap_get_bounds(bitmap);
    uint8_t *data = gbitmap_get_data(bitmap);
    uint16_t stride = gbitmap_get_bytes_per_row(bitmap);

    for (int y = 0; y < bounds.size.h; y++) {
        for (int x = 0; x < bounds.size.w; x++) {
            uint8_t p = 0;
            if (y < font_height) {
                if (x < font_width) {
                    p |= get_font_pixel(font, now->tm_hour / 10, x, y) << 1;
                } else if (x >= font_width + number_dx && x < font_width * 2 + number_dx) {
                    p |= get_font_pixel(font, now->tm_hour % 10, x - font_width - number_dx, y) << 1;
                }
            }
            if (y >= number_dy) {
                if (x >= number_dx && x < font_width + number_dx) {
                    p |= get_font_pixel(font, now->tm_min / 10, x - number_dx, y - number_dy);
                } else if (x >= font_width + number_dx * 2) {
                    p |= get_font_pixel(font, now->tm_min % 10, x - font_width - number_dx * 2, y - number_dy);
                }
            }

            set_pixel(data, x, y, stride, p);
        }
    }

    layer_mark_dirty(bitmap_layer_get_layer(layer));
}
