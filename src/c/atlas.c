#include <pebble.h>

typedef struct {
    char character;
    GRect rect;
} FontAtlasEntry;

#define WIDTH_TINY 5
#define WIDTH_NARROW 8
#define WIDTH 15
#define WIDTH_WIDE 21
#define HEIGHT 20

// Font atlas definitions for the text font
const FontAtlasEntry TEXT_FONT_ATLAS[] = {
    {'A', {{0, 0}, {WIDTH, HEIGHT}}},
    {'B', {{WIDTH, 0}, {WIDTH, HEIGHT}}},
    {'C', {{WIDTH * 2, 0}, {WIDTH, HEIGHT}}},
    {'D', {{WIDTH * 3, 0}, {WIDTH, HEIGHT}}},
    {'E', {{WIDTH * 4, 0}, {WIDTH, HEIGHT}}},
    {'F', {{WIDTH * 5, 0}, {WIDTH, HEIGHT}}},
    {'G', {{WIDTH * 6, 0}, {WIDTH, HEIGHT}}},
    {'H', {{WIDTH * 7, 0}, {WIDTH, HEIGHT}}},
    {'I', {{WIDTH * 8, 0}, {WIDTH_TINY, HEIGHT}}},
    {'J', {{WIDTH * 9, 0}, {WIDTH, HEIGHT}}},
    {'K', {{0, HEIGHT}, {WIDTH, HEIGHT}}},
    {'L', {{WIDTH, HEIGHT}, {WIDTH, HEIGHT}}},
    {'M', {{WIDTH * 2, HEIGHT}, {WIDTH_WIDE, HEIGHT}}},
    {'N', {{WIDTH * 4, HEIGHT}, {WIDTH, HEIGHT}}},
    {'O', {{WIDTH * 5, HEIGHT}, {WIDTH, HEIGHT}}},
    {'P', {{WIDTH * 6, HEIGHT}, {WIDTH, HEIGHT}}},
    {'Q', {{WIDTH * 7, HEIGHT}, {WIDTH, HEIGHT}}},
    {'R', {{WIDTH * 8, HEIGHT}, {WIDTH, HEIGHT}}},
    {'S', {{WIDTH * 9, HEIGHT}, {WIDTH, HEIGHT}}},
    {'T', {{0, HEIGHT * 2}, {WIDTH, HEIGHT}}},
    {'U', {{WIDTH, HEIGHT * 2}, {WIDTH, HEIGHT}}},
    {'V', {{WIDTH * 2, HEIGHT * 2}, {WIDTH, HEIGHT}}},
    {'W', {{WIDTH * 3, HEIGHT * 2}, {WIDTH_WIDE, HEIGHT}}},
    {'X', {{WIDTH * 5, HEIGHT * 2}, {WIDTH, HEIGHT}}},
    {'Y', {{WIDTH * 6, HEIGHT * 2}, {WIDTH, HEIGHT}}},
    {'Z', {{WIDTH * 7, HEIGHT * 2}, {WIDTH, HEIGHT}}},
    {'0', {{WIDTH * 8, HEIGHT * 2}, {WIDTH, HEIGHT}}},
    {'1', {{WIDTH * 9, HEIGHT * 2}, {WIDTH_NARROW, HEIGHT}}},
    {'2', {{0, HEIGHT * 3}, {WIDTH, HEIGHT}}},
    {'3', {{WIDTH, HEIGHT * 3}, {WIDTH, HEIGHT}}},
    {'4', {{WIDTH * 2, HEIGHT * 3}, {WIDTH, HEIGHT}}},
    {'5', {{WIDTH * 3, HEIGHT * 3}, {WIDTH, HEIGHT}}},
    {'6', {{WIDTH * 4, HEIGHT * 3}, {WIDTH, HEIGHT}}},
    {'7', {{WIDTH * 5, HEIGHT * 3}, {WIDTH, HEIGHT}}},
    {'8', {{WIDTH * 6, HEIGHT * 3}, {WIDTH, HEIGHT}}},
    {'9', {{WIDTH * 7, HEIGHT * 3}, {WIDTH, HEIGHT}}},
    {'/', {{WIDTH * 8, HEIGHT * 3}, {WIDTH, HEIGHT}}},
    {'%', {{WIDTH * 9, HEIGHT * 3}, {WIDTH, HEIGHT}}},
};
