#pragma once

#include <pebble.h>

typedef struct {
    char character;
    GRect rect;
} FontAtlasEntry;

extern FontAtlasEntry TEXT_FONT_ATLAS[38];
