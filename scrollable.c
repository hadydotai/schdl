#include <stdlib.h>
#include <math.h>
#include "raylib.h"
#include "rlgl.h"
#include "scrollable.h"
#include "scaling.h"

#define BASE_HANDLE_WIDTH 20
#define BASE_HANDLE_PADDING 2
#define BASE_HANDLE_RADIUS 5

static float Clamp(float value, float min, float max)
{
  if (value < min)
    return min;
  if (value > max)
    return max;
  return value;
}

static void scrollable_on_scale(float scale_x, float scale_y, void *user_data)
{
  scrollable_t *scrollable = (scrollable_t *)user_data;

  // Update scaled bounds
  scrollable->bounds = (Rectangle){
      scrollable->base_bounds.x * scale_x,
      scrollable->base_bounds.y * scale_y,
      scrollable->base_bounds.width * scale_x,
      scrollable->base_bounds.height * scale_y};
}

scrollable_t *create_scrollable(Rectangle bounds)
{
  scrollable_t *scrollable = (scrollable_t *)malloc(sizeof(scrollable_t));
  scrollable->base_bounds = bounds;
  scrollable->bounds = bounds; // Will be updated by scale listener
  scrollable->content_height = 0;
  scrollable->scroll_offset = 0;
  scrollable->is_dragging = false;
  scrollable->drag_start = (Vector2){0, 0};
  scrollable->last_y_pos = 0;

  // Register scaling listener
  scaling_add_listener(scrollable_on_scale, scrollable);

  return scrollable;
}

void destroy_scrollable(scrollable_t *scrollable)
{
  free(scrollable);
}

void begin_scrollable(scrollable_t *scrollable)
{
  float handle_width = BASE_HANDLE_WIDTH;
  float handle_padding = scaling_apply_x(BASE_HANDLE_PADDING);

  // Handle input
  Vector2 mouse = GetMousePosition();
  Rectangle scrollbar_bounds = {
      scrollable->bounds.x + scrollable->bounds.width - handle_width - handle_padding,
      scrollable->bounds.y,
      handle_width,
      scrollable->bounds.height};

  // Mouse wheel scrolling
  if (CheckCollisionPointRec(mouse, scrollable->bounds))
  {
    float wheel = GetMouseWheelMove();
    if (wheel != 0)
    {
      scrollable->scroll_offset -= wheel * scaling_apply_y(50);
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
      scrollable->scroll_offset += scroll_ratio * max_scroll * scaling_apply_y(50);
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
      scrollable->bounds.width - handle_width - (handle_padding * 2),
      scrollable->bounds.height);

  rlPushMatrix();
  rlTranslatef(0, -scrollable->scroll_offset, 0);

  scrollable->last_y_pos = 0;
}

void end_scrollable(scrollable_t *scrollable)
{
  float handle_width = BASE_HANDLE_WIDTH;
  float handle_padding = scaling_apply_x(BASE_HANDLE_PADDING);
  float handle_radius = scaling_apply_x(BASE_HANDLE_RADIUS);

  // Update content height and restore transform
  scrollable->content_height = scrollable->last_y_pos;
  rlPopMatrix();
  EndScissorMode();

  // Draw scrollbar if needed
  if (scrollable->content_height > scrollable->bounds.height)
  {
    float content_ratio = scrollable->bounds.height / scrollable->content_height;
    float handle_height = fmaxf(scrollable->bounds.height * content_ratio, scaling_apply_y(20));
    float scroll_percent = scrollable->scroll_offset / (scrollable->content_height - scrollable->bounds.height);
    float handle_y = scrollable->bounds.y + (scrollable->bounds.height - handle_height) * scroll_percent;

    // Draw scrollbar background
    DrawRectangle(
        scrollable->bounds.x + scrollable->bounds.width - handle_width - handle_padding,
        scrollable->bounds.y,
        handle_width,
        scrollable->bounds.height,
        (Color){200, 200, 200, 100});

    // Draw handle
    DrawRectangleRounded(
        (Rectangle){
            scrollable->bounds.x + scrollable->bounds.width - handle_width - handle_padding,
            handle_y,
            handle_width,
            handle_height},
        handle_radius,
        8,
        (Color){100, 100, 100, 200});
  }
}