#ifndef SCROLLABLE_H
#define SCROLLABLE_H

#include "raylib.h"

typedef struct scrollable
{
  Rectangle bounds;     // Visible area bounds
  float content_height; // Total height of content
  float scroll_offset;  // Current scroll position
  bool is_dragging;
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
// Content rendering helpers
void begin_measure_content(scrollable_t *scrollable);
void end_measure_content(scrollable_t *scrollable, float content_height);
void begin_scrollable_content(scrollable_t *scrollable);
void end_scrollable_content(scrollable_t *scrollable);

#endif // SCROLLABLE_H