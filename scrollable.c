#include <stdlib.h>
#include <math.h>
#include "raylib.h"
#include "rlgl.h"
#include "scrollable.h"

static float Clamp(float value, float min, float max)
{
  if (value < min)
    return min;
  if (value > max)
    return max;
  return value;
}

scrollable_t *create_scrollable(Vector2 startPos)
{
  scrollable_t *scrollable = (scrollable_t *)malloc(sizeof(scrollable_t));
  scrollable->bounds = (Rectangle){startPos.x, startPos.y, 0, 0};
  scrollable->content_height = 0;
  scrollable->scroll_offset = 0;
  scrollable->is_dragging = false;
  scrollable->drag_start = (Vector2){0, 0};
  scrollable->handle_color = (Color){100, 100, 100, 200};
  scrollable->handle_width = 10;
  scrollable->handle_padding = 2;
  scrollable->handle_radius = 5;
  scrollable->content_measuring = false;
  return scrollable;
}

void destroy_scrollable(scrollable_t *scrollable)
{
  free(scrollable);
}

void update_scrollable(scrollable_t *scrollable)
{
  if (scrollable->content_measuring)
    return;

  float visible_height = scrollable->bounds.height;
  float max_scroll = fmaxf(0, scrollable->content_height - visible_height);

  // Handle mouse wheel scrolling
  Vector2 mouse = GetMousePosition();
  if (CheckCollisionPointRec(mouse, scrollable->bounds))
  {
    float wheel = GetMouseWheelMove();
    if (wheel != 0)
    {
      scrollable->scroll_offset -= wheel * 50; // Increased scroll speed
    }
  }

  // Handle scrollbar dragging
  Rectangle scrollbar_bounds = {
      scrollable->bounds.x + scrollable->bounds.width - scrollable->handle_width - scrollable->handle_padding,
      scrollable->bounds.y,
      scrollable->handle_width,
      scrollable->bounds.height};

  if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mouse, scrollbar_bounds))
  {
    scrollable->is_dragging = true;
    scrollable->drag_start = mouse;
  }

  if (scrollable->is_dragging && IsMouseButtonDown(MOUSE_BUTTON_LEFT))
  {
    float delta = mouse.y - scrollable->drag_start.y;
    float scroll_ratio = delta / scrollable->bounds.height;
    scrollable->scroll_offset += scroll_ratio * max_scroll; // Use max_scroll instead of content_height
    scrollable->drag_start = mouse;
  }
  else
  {
    scrollable->is_dragging = false;
  }

  // Clamp scroll offset
  scrollable->scroll_offset = Clamp(scrollable->scroll_offset, 0, max_scroll);
}

void draw_scrollable(scrollable_t *scrollable)
{
  if (scrollable->content_measuring)
    return;

  float visible_height = scrollable->bounds.height;

  // Only draw scrollbar if content exceeds visible area
  if (scrollable->content_height > visible_height)
  {
    float content_ratio = visible_height / scrollable->content_height;
    float handle_height = fmaxf(visible_height * content_ratio, 20);
    float scroll_percent = scrollable->scroll_offset / (scrollable->content_height - visible_height);
    float handle_y = scrollable->bounds.y + (scrollable->bounds.height - handle_height) * scroll_percent;

    // Draw scrollbar background
    DrawRectangle(
        scrollable->bounds.x + scrollable->bounds.width - scrollable->handle_width - scrollable->handle_padding,
        scrollable->bounds.y,
        scrollable->handle_width,
        scrollable->bounds.height,
        (Color){200, 200, 200, 100});

    // Draw handle
    DrawRectangleRounded(
        (Rectangle){
            scrollable->bounds.x + scrollable->bounds.width - scrollable->handle_width - scrollable->handle_padding,
            handle_y,
            scrollable->handle_width,
            handle_height},
        scrollable->handle_radius,
        8,
        scrollable->handle_color);
  }
}

void resize_scrollable(scrollable_t *scrollable, Vector2 newSize)
{
  scrollable->bounds.width = newSize.x;
  scrollable->bounds.height = newSize.y;
}

void begin_measure_content(scrollable_t *scrollable)
{
  scrollable->content_measuring = true;
  BeginScissorMode(0, -10000, GetScreenWidth(), 10000); // Render off-screen
}

void end_measure_content(scrollable_t *scrollable, float content_height)
{
  scrollable->content_height = content_height;
  scrollable->content_measuring = false;
  EndScissorMode();
}

void begin_scrollable_content(scrollable_t *scrollable)
{
  BeginScissorMode(
      scrollable->bounds.x,
      scrollable->bounds.y,
      scrollable->bounds.width - scrollable->handle_width - (scrollable->handle_padding * 2),
      scrollable->bounds.height);

  rlPushMatrix();
  rlTranslatef(0, -scrollable->scroll_offset, 0);
}

void end_scrollable_content(scrollable_t *scrollable)
{
  rlPopMatrix();
  EndScissorMode();
}