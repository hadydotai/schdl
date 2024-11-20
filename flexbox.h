#ifndef FLEXBOX_H
#define FLEXBOX_H

#include "raylib.h"

typedef enum fbox_direction
{
  fbox_DIRECTION_ROW,
  fbox_DIRECTION_COLUMN
} fbox_direction_t;

typedef enum fbox_align
{
  fbox_ALIGN_START,
  fbox_ALIGN_CENTER,
  fbox_ALIGN_END,
  fbox_ALIGN_SPACE_BETWEEN,
  fbox_ALIGN_SPACE_AROUND
} fbox_align_t;

typedef enum fbox_size_mode
{
  fbox_SIZE_FIXED,  // Use exact size provided
  fbox_SIZE_STRETCH // Stretch to fill container (respecting padding)
} fbox_size_mode_t;

typedef struct fbox_context
{
  Rectangle bounds;
  fbox_direction_t direction;
  fbox_align_t main_align;
  fbox_align_t cross_align;
  float gap;
  float padding;
  int expected_items;
  Vector2 *item_sizes;
  int item_count;
  int max_items;
  float current_pos;
  float content_height;
  float content_width;
  fbox_size_mode_t size_mode;
} fbox_context_t;

fbox_context_t fbox_create(Rectangle bounds, fbox_direction_t direction);
fbox_context_t fbox_create_nested(fbox_context_t *parent, Rectangle bounds);
void fbox_set_direction(fbox_context_t *ctx, fbox_direction_t direction);
void fbox_set_main_align(fbox_context_t *ctx, fbox_align_t align);
void fbox_set_cross_align(fbox_context_t *ctx, fbox_align_t align);
void fbox_set_gap(fbox_context_t *ctx, float gap);
void fbox_set_padding(fbox_context_t *ctx, float padding);
void fbox_set_expected_items(fbox_context_t *ctx, int count);
void fbox_set_size_mode(fbox_context_t *ctx, fbox_size_mode_t mode);
void fbox_destroy(fbox_context_t *ctx);

float fbox_get_content_height(fbox_context_t *ctx);
float fbox_get_content_width(fbox_context_t *ctx);

Rectangle fbox_next(fbox_context_t *ctx, Vector2 size);
Vector2 fbox_next_position(fbox_context_t *ctx);

#endif // FLEXBOX_H