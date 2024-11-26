#include <stdio.h>
#include <stdlib.h>

// raylib headers
#include "raylib.h"
#include "data.h"
#include "scrollable.h"
#include "flexbox.h"
#include "scaling.h"

#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define LIGHT_BLUE \
  (Color) { 235, 240, 255, 255 }
#define LIGHT_PURPLE \
  (Color) { 245, 235, 255, 255 }
#define LIGHT_GRAY \
  (Color) { 220, 220, 220, 255 }

float get_item_completion(schedule_item_t *item)
{
  time_t now = time(NULL);
  if (now < item->start)
    return 0.0f;
  if (now > item->end)
    return 100.0f;

  float duration = item->end - item->start;
  float elapsed = now - item->start;
  return (elapsed / duration) * 100.0f;
}

static char *format_percentage(float percentage)
{
  char *buffer = malloc(10);
  sprintf(buffer, "%d%%", (int)percentage);
  return buffer;
}

void draw_header()
{
  fbox_context_t header_fbox = fbox_create((Rectangle){0, 0, GetScreenWidth(), scaling_apply_y(50)},
                                           fbox_DIRECTION_ROW,
                                           NULL);
  fbox_set_main_align(&header_fbox, fbox_ALIGN_SPACE_BETWEEN);
  fbox_set_cross_align(&header_fbox, fbox_ALIGN_CENTER);
  fbox_set_expected_items(&header_fbox, 2);
  fbox_set_padding(&header_fbox, 14);
  fbox_set_size_mode(&header_fbox, fbox_SIZE_FIXED);

  int titleWidth = MeasureText("Schedule", scaling_apply_y(20));
  Rectangle titleRect = fbox_next(&header_fbox, (Vector2){titleWidth, scaling_apply_y(20)});
  DrawText("Schedule", titleRect.x, titleRect.y, scaling_apply_y(20), BLACK);

  char *timeText = format_time_12hr(time(NULL));
  int timeWidth = MeasureText(timeText, scaling_apply_y(20));
  Rectangle timeRect = fbox_next(&header_fbox, (Vector2){timeWidth, scaling_apply_y(20)});
  DrawText(timeText, timeRect.x, timeRect.y, scaling_apply_y(20), BLACK);
  free_formatted_time(timeText);

  DrawLine(0,
           header_fbox.bounds.y + header_fbox.bounds.height,
           GetScreenWidth(),
           header_fbox.bounds.y + header_fbox.bounds.height,
           (Color){GRAY.r, GRAY.g, GRAY.b, 100});
  fbox_destroy(&header_fbox);
}

void draw_schedule(schedule_t *schedule, scrollable_t *scrollable)
{
  schedule_iterator_t *iterator = create_iterator(schedule);
  schedule_item_t *item = get_current_item(iterator);

  fbox_context_t items_fbox = fbox_create(
      (Rectangle){0, scaling_apply_y(50), scrollable->bounds.width, scrollable->bounds.height},
      fbox_DIRECTION_COLUMN,
      scrollable);
  fbox_set_main_align(&items_fbox, fbox_ALIGN_START);
  fbox_set_cross_align(&items_fbox, fbox_ALIGN_START);
  fbox_set_gap(&items_fbox, 14);
  fbox_set_padding(&items_fbox, 14);
  fbox_set_expected_items(&items_fbox, schedule->count);
  fbox_set_size_mode(&items_fbox, fbox_SIZE_STRETCH);

  while (item != NULL)
  {
    Vector2 size = (Vector2){
        0,
        scaling_apply_y(100)};

    bool is_current = is_item_current(item);
    bool is_past = is_item_past(item);

    Rectangle itemRect = fbox_next(&items_fbox, size);
    Color color = item->type == SCHEDULE_ITEM_TYPE_BREAK ? LIGHT_BLUE : LIGHT_PURPLE;
    DrawRectangleRec(itemRect, color);
    Color lineColor = is_current ? PURPLE : is_past ? LIGHT_GRAY
                                                    : DARKGRAY;
    DrawRectangleRoundedLinesEx(itemRect, 0.1f, 8, 3, lineColor);

    float completion = get_item_completion(item);
    Rectangle progressRect = itemRect;
    progressRect.width = (progressRect.width * completion) / 100.0f;
    DrawRectangleRounded(progressRect, 0.1f, 8, (Color){lineColor.r, lineColor.g, lineColor.b, 40});

    fbox_context_t item_content_fbox = fbox_create_nested(&items_fbox, itemRect);
    fbox_set_direction(&item_content_fbox, fbox_DIRECTION_ROW);
    fbox_set_main_align(&item_content_fbox, fbox_ALIGN_SPACE_BETWEEN);
    fbox_set_padding(&item_content_fbox, 10);
    fbox_set_expected_items(&item_content_fbox, 2);
    fbox_set_size_mode(&item_content_fbox, fbox_SIZE_STRETCH);
    fbox_set_flex_weights(&item_content_fbox, (float[]){0.7f, 0.3f});

    { // Title and time range
      Rectangle item_info_rect = fbox_next(&item_content_fbox, (Vector2){0, itemRect.height});
      fbox_context_t item_info = fbox_create_nested(&item_content_fbox, item_info_rect);
      fbox_set_direction(&item_info, fbox_DIRECTION_COLUMN);
      fbox_set_gap(&item_info, 5);
      fbox_set_expected_items(&item_info, 2);

      Rectangle titleRect = fbox_next(&item_info, (Vector2){0, scaling_apply_y(20)});
      DrawText(item->title,
               titleRect.x,
               titleRect.y,
               scaling_apply_y(20),
               BLACK);

      Rectangle timeRect = fbox_next(&item_info, (Vector2){0, scaling_apply_y(20)});
      char *duration_text = format_duration_12hr(item->start, item->end);

      DrawText(duration_text,
               timeRect.x,
               timeRect.y,
               scaling_apply_y(20),
               BLACK);
      free_formatted_duration(duration_text);

      item = get_next_item(iterator);
      fbox_destroy(&item_info);
    }

    { // Percentage progress and current marker
      Rectangle item_progress_rect = fbox_next(&item_content_fbox, (Vector2){0, itemRect.height});
      fbox_context_t item_progress = fbox_create_nested(&item_content_fbox, item_progress_rect);
      fbox_set_direction(&item_progress, fbox_DIRECTION_COLUMN);
      fbox_set_main_align(&item_progress, fbox_ALIGN_START);
      fbox_set_cross_align(&item_progress, fbox_ALIGN_END);
      fbox_set_expected_items(&item_progress, 2);

      char *percentage_text = format_percentage(completion);
      int percentage_width = MeasureText(percentage_text, scaling_apply_y(13));
      Rectangle progressRect = fbox_next(&item_progress,
                                         (Vector2){percentage_width, scaling_apply_y(20)});

      DrawText(percentage_text, progressRect.x, progressRect.y,
               scaling_apply_y(13), BLACK);
      free(percentage_text);

      if (is_current)
      {
        int markerWidth = MeasureText("Current", scaling_apply_y(13));
        Rectangle markerRect = fbox_next(&item_progress,
                                         (Vector2){markerWidth + 10, scaling_apply_y(20)});
        float textY = markerRect.y + (scaling_apply_y(20) - scaling_apply_y(13)) / 2;
        float textX = markerRect.x + (markerWidth + 10 - markerWidth) / 2;
        DrawText("Current", textX, textY, scaling_apply_y(13), BLACK);
        DrawRectangleRoundedLinesEx((Rectangle){markerRect.x, markerRect.y, markerWidth + 10, scaling_apply_y(20)}, 0.1f, 8, 3, (Color){PURPLE.r, PURPLE.g, PURPLE.b, 200});
        DrawRectangleRounded((Rectangle){markerRect.x, markerRect.y, markerWidth + 10, scaling_apply_y(20)}, 0.1f, 8, (Color){PURPLE.r, PURPLE.g, PURPLE.b, 100});
      }

      fbox_destroy(&item_progress);
    }
    fbox_destroy(&item_content_fbox);
  }

  destroy_iterator(iterator);
  fbox_destroy(&items_fbox);
}

int main()
{
  SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
  SetTraceLogLevel(LOG_WARNING);
  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Daily Schedule");
  SetTargetFPS(60);

  scaling_init(WINDOW_WIDTH, WINDOW_HEIGHT);

  schedule_t *schedule = create_schedule();
  add_item(schedule, (schedule_item_t){.title = "Breakfast", .start = make_time(8, 0), .end = make_time(9, 0), .type = SCHEDULE_ITEM_TYPE_BREAK});
  add_item(schedule, (schedule_item_t){.title = "SR Work", .start = make_time(9, 0), .end = make_time(12, 0), .type = SCHEDULE_ITEM_TYPE_EVENT});
  add_item(schedule, (schedule_item_t){.title = "Lunch", .start = make_time(12, 0), .end = make_time(13, 0), .type = SCHEDULE_ITEM_TYPE_BREAK});
  add_item(schedule, (schedule_item_t){.title = "Personal Work", .start = make_time(13, 0), .end = make_time(18, 0), .type = SCHEDULE_ITEM_TYPE_EVENT});
  add_item(schedule, (schedule_item_t){.title = "Dinner", .start = make_time(18, 0), .end = make_time(19, 0), .type = SCHEDULE_ITEM_TYPE_BREAK});
  add_item(schedule, (schedule_item_t){.title = "Supper", .start = make_time(19, 0), .end = make_time(20, 0), .type = SCHEDULE_ITEM_TYPE_BREAK});

  scrollable_t *scrollable = create_scrollable((Rectangle){
      0, scaling_apply_y(50),
      WINDOW_WIDTH,
      WINDOW_HEIGHT - scaling_apply_y(50)});

  while (!WindowShouldClose())
  {
    BeginDrawing();

    ClearBackground(RAYWHITE);
    scaling_update();

    begin_scrollable(scrollable);
    draw_schedule(schedule, scrollable);
    end_scrollable(scrollable);

    draw_header();

    EndDrawing();
  }

  // Cleanup in correct order
  destroy_schedule(schedule);
  destroy_scrollable(scrollable);
  scaling_cleanup();
  CloseWindow();

  return 0;
}
