#include <stdio.h>
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

void draw_schedule(schedule_t *schedule, scrollable_t *scrollable)
{
  schedule_iterator_t *iterator = create_iterator(schedule);
  schedule_item_t *item = get_current_item(iterator);

  fbox_context_t fb = fbox_create(
      (Rectangle){0, 0, GetScreenWidth(), GetScreenHeight()},
      fbox_DIRECTION_COLUMN,
      scrollable);
  fbox_set_main_align(&fb, fbox_ALIGN_START);
  fbox_set_cross_align(&fb, fbox_ALIGN_START);
  fbox_set_gap(&fb, 14);
  fbox_set_padding(&fb, 30);
  fbox_set_expected_items(&fb, schedule->count);
  fbox_set_size_mode(&fb, fbox_SIZE_STRETCH);

  while (item != NULL)
  {
    Vector2 size = (Vector2){
        0,
        scaling_apply_y(100)};

    Rectangle itemRect = fbox_next(&fb, size);
    DrawRectangleRec(itemRect, LIGHTGRAY);
    DrawRectangleLinesEx(itemRect, 2 * scaling_get_x(), DARKGRAY);

    fbox_context_t item_fb = fbox_create_nested(&fb, itemRect);
    fbox_set_direction(&item_fb, fbox_DIRECTION_COLUMN);
    fbox_set_main_align(&item_fb, fbox_ALIGN_START);
    fbox_set_cross_align(&item_fb, fbox_ALIGN_START);
    fbox_set_gap(&item_fb, 5);
    fbox_set_padding(&item_fb, 10);
    fbox_set_expected_items(&item_fb, 2);

    Rectangle titleRect = fbox_next(&item_fb, (Vector2){0, scaling_apply_y(20)});
    DrawText(item->title,
             titleRect.x,
             titleRect.y,
             scaling_apply_y(20),
             BLACK);

    Rectangle timeRect = fbox_next(&item_fb, (Vector2){0, scaling_apply_y(20)});
    char *timeText = format_duration(item->start, item->end);

    DrawText(timeText,
             timeRect.x,
             timeRect.y,
             scaling_apply_y(20),
             BLACK);

    item = get_next_item(iterator);
    fbox_destroy(&item_fb);
  }

  destroy_iterator(iterator);
  fbox_destroy(&fb);
}

int main()
{
  SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
  SetTraceLogLevel(LOG_WARNING);
  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Daily Schedule");
  SetTargetFPS(60);

  scaling_init(WINDOW_WIDTH, WINDOW_HEIGHT);

  schedule_t *schedule = create_schedule();
  add_item(schedule, (schedule_item_t){.title = "Breakfast", .start = make_time(8, 0), .end = make_time(9, 0)});
  add_item(schedule, (schedule_item_t){.title = "SR Work", .start = make_time(9, 0), .end = make_time(12, 0)});
  add_item(schedule, (schedule_item_t){.title = "Lunch", .start = make_time(12, 0), .end = make_time(13, 0)});
  add_item(schedule, (schedule_item_t){.title = "Personal Work", .start = make_time(13, 0), .end = make_time(18, 0)});
  add_item(schedule, (schedule_item_t){.title = "Dinner", .start = make_time(18, 0), .end = make_time(19, 0)});
  add_item(schedule, (schedule_item_t){.title = "Supper", .start = make_time(19, 0), .end = make_time(20, 0)});

  debug_print_schedule_items_iter(schedule);

  scrollable_t *scrollable = create_scrollable((Rectangle){
      0, 0,
      WINDOW_WIDTH,
      WINDOW_HEIGHT});

  while (!WindowShouldClose())
  {
    BeginDrawing();
    ClearBackground(RAYWHITE);

    scaling_update();

    begin_scrollable(scrollable);
    draw_schedule(schedule, scrollable);
    end_scrollable(scrollable);

    EndDrawing();
  }

  scaling_cleanup();
  CloseWindow();
  destroy_schedule(schedule);
  destroy_scrollable(scrollable);
  return 0;
}
