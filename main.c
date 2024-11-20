#include <stdio.h>
#include "raylib.h"
#include "data.h"
#include "scrollable.h"
#include "flexbox.h"

#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

static float scale_x = 1.0f;
static float scale_y = 1.0f;

void debug_print_schedule(schedule_t *schedule)
{
  for (int i = 0; i < schedule->count; i++)
  {
    struct tm start_tm, end_tm;
    localtime_r(&schedule->items[i].start, &start_tm);
    localtime_r(&schedule->items[i].end, &end_tm);

    printf("Item %d: %s (%02d:%02d - %02d:%02d)\n",
           i,
           schedule->items[i].title,
           start_tm.tm_hour, start_tm.tm_min,
           end_tm.tm_hour, end_tm.tm_min);
  }
}

void debug_print_schedule_items_iter(schedule_t *schedule)
{
  schedule_iterator_t *iterator = create_iterator(schedule);
  schedule_item_t *item = get_current_item(iterator);
  while (item != NULL)
  {
    printf("%s\n", item->title);
    item = get_next_item(iterator);
  }
  destroy_iterator(iterator);
}

float draw_schedule(schedule_t *schedule)
{
  schedule_iterator_t *iterator = create_iterator(schedule);
  schedule_item_t *item = get_current_item(iterator);

  fbox_context_t fb = fbox_create(
      (Rectangle){0, 0, GetScreenWidth(), GetScreenHeight()},
      fbox_DIRECTION_COLUMN);
  fbox_set_main_align(&fb, fbox_ALIGN_START);
  fbox_set_cross_align(&fb, fbox_ALIGN_START);
  fbox_set_gap(&fb, 14 * scale_y);
  fbox_set_padding(&fb, 30 * scale_x);
  fbox_set_expected_items(&fb, schedule->count);
  fbox_set_size_mode(&fb, fbox_SIZE_STRETCH);

  while (item != NULL)
  {
    Vector2 size = (Vector2){
        0,
        100 * scale_y};

    Rectangle itemRect = fbox_next(&fb, size);
    DrawRectangleRec(itemRect, LIGHTGRAY);
    DrawRectangleLinesEx(itemRect, 2 * scale_x, DARKGRAY);

    fbox_context_t item_fb = fbox_create_nested(&fb, itemRect);
    fbox_set_direction(&item_fb, fbox_DIRECTION_COLUMN);
    fbox_set_main_align(&item_fb, fbox_ALIGN_START);
    fbox_set_cross_align(&item_fb, fbox_ALIGN_START);
    fbox_set_gap(&item_fb, 5 * scale_y);
    fbox_set_padding(&item_fb, 10 * scale_x);
    fbox_set_expected_items(&item_fb, 2);

    Rectangle titleRect = fbox_next(&item_fb, (Vector2){0, 20 * scale_y});
    DrawText(item->title,
             titleRect.x,
             titleRect.y,
             20 * scale_y,
             BLACK);

    Rectangle timeRect = fbox_next(&item_fb, (Vector2){0, 20 * scale_y});
    char *timeText = format_duration(item->start, item->end);

    DrawText(timeText,
             timeRect.x,
             timeRect.y,
             20 * scale_y,
             BLACK);

    item = get_next_item(iterator);
    fbox_destroy(&item_fb);
  }

  destroy_iterator(iterator);
  fbox_destroy(&fb);
  return fbox_get_content_height(&fb);
}

void update_scaling(void)
{
  scale_x = (float)GetScreenWidth() / WINDOW_WIDTH;
  scale_y = (float)GetScreenHeight() / WINDOW_HEIGHT;
}

int main()
{
  SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
  SetTraceLogLevel(LOG_WARNING);
  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Daily Schedule");
  SetTargetFPS(60);

  schedule_t *schedule = create_schedule();
  add_item(schedule, (schedule_item_t){.title = "Breakfast", .start = make_time(8, 0), .end = make_time(9, 0)});
  add_item(schedule, (schedule_item_t){.title = "SR Work", .start = make_time(9, 0), .end = make_time(12, 0)});
  add_item(schedule, (schedule_item_t){.title = "Lunch", .start = make_time(12, 0), .end = make_time(13, 0)});
  add_item(schedule, (schedule_item_t){.title = "Personal Work", .start = make_time(13, 0), .end = make_time(18, 0)});
  add_item(schedule, (schedule_item_t){.title = "Dinner", .start = make_time(18, 0), .end = make_time(19, 0)});
  add_item(schedule, (schedule_item_t){.title = "Supper", .start = make_time(19, 0), .end = make_time(20, 0)});

  debug_print_schedule_items_iter(schedule);

  scrollable_t *scrollable = create_scrollable((Vector2){
      0,
      0,
  });

  while (!WindowShouldClose())
  {
    BeginDrawing();
    ClearBackground(RAYWHITE);

    update_scaling();
    resize_scrollable(scrollable, (Vector2){WINDOW_WIDTH * scale_x, WINDOW_HEIGHT * scale_y});

    // First pass: measure content height only
    begin_measure_content(scrollable);
    float content_height = draw_schedule(schedule);
    end_measure_content(scrollable, content_height);

    // Update scrolling before drawing
    update_scrollable(scrollable);

    // Second pass: actually draw content
    begin_scrollable_content(scrollable);
    draw_schedule(schedule);
    end_scrollable_content(scrollable);

    draw_scrollable(scrollable);

    EndDrawing();
  }

  CloseWindow();
  destroy_schedule(schedule);
  destroy_scrollable(scrollable);
  return 0;
}
