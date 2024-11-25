#include <stdlib.h>
#include <math.h>
#include "raylib.h"
#include "rlgl.h"
#include "scrollable.h"

#define HANDLE_WIDTH 20
#define HANDLE_PADDING 2
#define HANDLE_RADIUS 5

static float Clamp(float value, float min, float max)
{
  if (value < min)
    return min;
  if (value > max)
    return max;
  return value;
}

scrollable_t *create_scrollable(Rectangle bounds)
{
  scrollable_t *scrollable = (scrollable_t *)malloc(sizeof(scrollable_t));
  scrollable->bounds = bounds;
  scrollable->content_height = 0;
  scrollable->scroll_offset = 0;
  scrollable->is_dragging = false;
  scrollable->drag_start = (Vector2){0, 0};
  scrollable->last_y_pos = 0;
  return scrollable;
}

void destroy_scrollable(scrollable_t *scrollable)
{
  free(scrollable);
}

void begin_scrollable(scrollable_t *scrollable)
{
  // Handle input
  Vector2 mouse = GetMousePosition();
  Rectangle scrollbar_bounds = {
      scrollable->bounds.x + scrollable->bounds.width - HANDLE_WIDTH - HANDLE_PADDING,
      scrollable->bounds.y,
      HANDLE_WIDTH,
      scrollable->bounds.height};

  // Mouse wheel scrolling
  if (CheckCollisionPointRec(mouse, scrollable->bounds))
  {
    float wheel = GetMouseWheelMove();
    if (wheel != 0)
    {
      scrollable->scroll_offset -= wheel * 50;
    }
  }

  // Handle dragging
  if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mouse, scrollbar_bounds))
  {
    scrollable->is_dragging = true;
    scrollable->drag_start = mouse;
  }

  if (scrollable->is_dragging)
  {
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
    {
      float delta = mouse.y - scrollable->drag_start.y;
      float max_scroll = fmaxf(0, scrollable->content_height - scrollable->bounds.height);
      float scroll_ratio = delta / scrollable->bounds.height;
      scrollable->scroll_offset += scroll_ratio * max_scroll * 50;
      scrollable->drag_start = mouse;
    }
    else
    {
      scrollable->is_dragging = false;
    }
  }

  // Clamp scroll offset
  float max_scroll = fmaxf(0, scrollable->content_height - scrollable->bounds.height);
  scrollable->scroll_offset = Clamp(scrollable->scroll_offset, 0, max_scroll);

  // Begin scissor mode and transform
  BeginScissorMode(
      scrollable->bounds.x,
      scrollable->bounds.y,
      scrollable->bounds.width - HANDLE_WIDTH - (HANDLE_PADDING * 2),
      scrollable->bounds.height);

  rlPushMatrix();
  rlTranslatef(0, -scrollable->scroll_offset, 0);

  scrollable->last_y_pos = 0;
}

void end_scrollable(scrollable_t *scrollable)
{
  // Update content height and restore transform
  scrollable->content_height = scrollable->last_y_pos;
  rlPopMatrix();
  EndScissorMode();

  // Draw scrollbar if needed
  if (scrollable->content_height > scrollable->bounds.height)
  {
    float content_ratio = scrollable->bounds.height / scrollable->content_height;
    float handle_height = fmaxf(scrollable->bounds.height * content_ratio, 20);
    float scroll_percent = scrollable->scroll_offset / (scrollable->content_height - scrollable->bounds.height);
    float handle_y = scrollable->bounds.y + (scrollable->bounds.height - handle_height) * scroll_percent;

    // Draw scrollbar background
    DrawRectangle(
        scrollable->bounds.x + scrollable->bounds.width - HANDLE_WIDTH - HANDLE_PADDING,
        scrollable->bounds.y,
        HANDLE_WIDTH,
        scrollable->bounds.height,
        (Color){200, 200, 200, 100});

    // Draw handle
    DrawRectangleRounded(
        (Rectangle){
            scrollable->bounds.x + scrollable->bounds.width - HANDLE_WIDTH - HANDLE_PADDING,
            handle_y,
            HANDLE_WIDTH,
            handle_height},
        HANDLE_RADIUS,
        8,
        (Color){100, 100, 100, 200});
  }
}