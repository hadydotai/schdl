#ifndef SCROLLABLE_H
#define SCROLLABLE_H

#include "raylib.h"

typedef struct scrollable
{
  Rectangle bounds;     // visible area
  float content_height; // content total height
  float scroll_offset;  // scroll position
  bool is_dragging;
  bool is_hovering;
  Vector2 drag_start;
  Color handle_color;
  float handle_width;
  float handle_padding;
  float handle_radius;
  bool content_measuring; // Flag for content measurement pass
} scrollable_t;

scrollable_t *create_scrollable(Vector2 startPos);
void destroy_scrollable(scrollable_t *scrollable);
void update_scrollable(scrollable_t *scrollable);
void draw_scrollable(scrollable_t *scrollable);
void resize_scrollable(scrollable_t *scrollable, Vector2 newSize);
void begin_measure_content(scrollable_t *scrollable);
void end_measure_content(scrollable_t *scrollable, float content_height);
void begin_scrollable_content(scrollable_t *scrollable);
void end_scrollable_content(scrollable_t *scrollable);

#endif // SCROLLABLE_H