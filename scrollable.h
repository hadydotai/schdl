#ifndef SCROLLABLE_H
#define SCROLLABLE_H

#include "raylib.h"

typedef struct
{
  Rectangle base_bounds;
  Rectangle bounds;
  float content_height;
  float scroll_offset;
  bool is_dragging;
  Vector2 drag_start;
  float last_y_pos;
} scrollable_t;

scrollable_t *create_scrollable(Rectangle bounds);
void destroy_scrollable(scrollable_t *scrollable);
void begin_scrollable(scrollable_t *scrollable);
void end_scrollable(scrollable_t *scrollable);

#endif