#ifndef FLEXBOX_H
#define FLEXBOX_H

#include "raylib.h"

typedef enum fb_direction
{
  FB_DIRECTION_ROW,
  FB_DIRECTION_COLUMN
} fb_direction_t;

typedef enum fb_align
{
  FB_ALIGN_START,
  FB_ALIGN_CENTER,
  FB_ALIGN_END,
  FB_ALIGN_SPACE_BETWEEN,
  FB_ALIGN_SPACE_AROUND
} fb_align_t;

typedef struct fb_context
{
  Rectangle bounds;
  fb_direction_t direction;
  fb_align_t main_align;
  fb_align_t cross_align;
  float gap;
  float padding;
  int expected_items;
  Vector2 *item_sizes;
  int item_count;
  int max_items;
  float current_pos;
} fb_context_t;

fb_context_t fb_create(Rectangle bounds, fb_direction_t direction);
void fb_set_main_align(fb_context_t *ctx, fb_align_t align);
void fb_set_cross_align(fb_context_t *ctx, fb_align_t align);
void fb_set_gap(fb_context_t *ctx, float gap);
void fb_set_padding(fb_context_t *ctx, float padding);
void fb_set_expected_items(fb_context_t *ctx, int count);

float fb_get_content_height(fb_context_t *ctx);
float fb_get_content_width(fb_context_t *ctx);

Rectangle fb_next(fb_context_t *ctx, Vector2 size);
#endif // FLEXBOX_H