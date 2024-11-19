#include "raylib.h"
#include <time.h>
#include <stdio.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 900
#define ITEM_HEIGHT 120
#define ITEM_PADDING 20
#define HEADER_HEIGHT 80
#define MAX_SCHEDULE_ITEMS 50
#define SCROLL_SPEED 30.0f
#define SCROLL_MARGIN 20.0f
#define SCROLLBAR_WIDTH 12
#define SCROLLBAR_MIN_HEIGHT 40

// Add these color definitions at the top
#define LIGHT_BLUE \
  (Color) { 235, 240, 255, 255 }
#define LIGHT_PURPLE \
  (Color) { 245, 235, 255, 255 }
#define PURPLE \
  (Color) { 149, 82, 255, 255 }
#define DARK_GRAY \
  (Color) { 85, 85, 85, 255 }

// Add these new definitions after the existing #defines
#define BASE_WIDTH 800
#define BASE_HEIGHT 900

// Add these global variables to track current window dimensions
static float scale_x = 1.0f;
static float scale_y = 1.0f;
static float scroll_offset = 0.0f;
static float max_scroll = 0.0f;

typedef struct
{
  char title[100];
  time_t start_time;
  time_t end_time;
  bool is_current;
} ScheduleItem;

typedef struct
{
  ScheduleItem items[MAX_SCHEDULE_ITEMS];
  int count;
  time_t current_time;
} Schedule;

Schedule schedule;

float get_item_completion(ScheduleItem *item)
{
  time_t now = time(NULL);
  if (now < item->start_time)
    return 0.0f;
  if (now > item->end_time)
    return 100.0f;

  float duration = item->end_time - item->start_time;
  float elapsed = now - item->start_time;
  return (elapsed / duration) * 100.0f;
}

// Add this function to update scaling factors
void update_scaling(void)
{
  scale_x = (float)GetScreenWidth() / BASE_WIDTH;
  scale_y = (float)GetScreenHeight() / BASE_HEIGHT;
}

// Add this helper function to scale rectangles
Rectangle scale_rect(Rectangle rect)
{
  return (Rectangle){
      rect.x * scale_x,
      rect.y * scale_y,
      rect.width * scale_x,
      rect.height * scale_y};
}

float get_content_height(void)
{
  float scaled_item_height = ITEM_HEIGHT * scale_y;
  float scaled_padding = ITEM_PADDING * scale_y;
  return (scaled_item_height + (2 * scaled_padding)) * schedule.count;
}

Rectangle get_scrollbar_bounds(void)
{
  float scaled_header_height = HEADER_HEIGHT * scale_y;
  return (Rectangle){
      GetScreenWidth() - (SCROLLBAR_WIDTH * scale_x),
      scaled_header_height,
      SCROLLBAR_WIDTH * scale_x,
      GetScreenHeight() - scaled_header_height};
}

Rectangle get_scrollbar_handle_bounds(void)
{
  Rectangle scrollbar = get_scrollbar_bounds();
  float content_height = get_content_height();
  float visible_height = GetScreenHeight() - (HEADER_HEIGHT * scale_y);

  // Calculate handle height
  float handle_height = (visible_height / content_height) * scrollbar.height;
  if (handle_height < SCROLLBAR_MIN_HEIGHT * scale_y)
    handle_height = SCROLLBAR_MIN_HEIGHT * scale_y;

  // Calculate handle position
  float max_scroll = content_height - visible_height;
  float handle_position = 0;
  if (max_scroll > 0)
  {
    handle_position = (scroll_offset / max_scroll) *
                      (scrollbar.height - handle_height);
  }

  return (Rectangle){
      scrollbar.x,
      scrollbar.y + handle_position,
      scrollbar.width,
      handle_height};
}

void update_scroll(void)
{
  float prev_scroll = scroll_offset;
  static bool dragging_scrollbar = false;
  static float drag_start_y = 0;
  static float drag_start_scroll = 0;

  // Handle mouse wheel (natural scrolling)
  float wheel = GetMouseWheelMove();
  if (wheel != 0)
  {
    printf("Mouse wheel: %f\n", wheel);
    scroll_offset -= wheel * SCROLL_SPEED * scale_y; // Changed plus to minus for natural scrolling
  }

  // Support keyboard up/down arrows (natural direction)
  if (IsKeyPressed(KEY_UP))
  {
    printf("Up pressed\n");
    scroll_offset -= ITEM_HEIGHT * scale_y; // Move content up
  }
  if (IsKeyPressed(KEY_DOWN))
  {
    printf("Down pressed\n");
    scroll_offset += ITEM_HEIGHT * scale_y; // Move content down
  }

  // Calculate max scroll based on content height
  float content_height = get_content_height();
  float visible_height = GetScreenHeight() - (HEADER_HEIGHT * scale_y);
  max_scroll = fmaxf(0, content_height - visible_height);

  // Clamp scroll offset between 0 and max_scroll
  if (scroll_offset < 0)
    scroll_offset = 0;
  if (scroll_offset > max_scroll)
    scroll_offset = max_scroll;

  // Handle scrollbar interaction
  Rectangle scrollbar = get_scrollbar_bounds();
  Rectangle handle = get_scrollbar_handle_bounds();
  Vector2 mouse = GetMousePosition();

  // Check if mouse is over scrollbar
  bool mouse_over_scrollbar = CheckCollisionPointRec(mouse, scrollbar);
  bool mouse_over_handle = CheckCollisionPointRec(mouse, handle);

  if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && mouse_over_handle)
  {
    dragging_scrollbar = true;
    drag_start_y = mouse.y;
    drag_start_scroll = scroll_offset;
  }
  else if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
  {
    dragging_scrollbar = false;
  }

  // Handle scrollbar dragging
  if (dragging_scrollbar)
  {
    float drag_delta = mouse.y - drag_start_y;
    float content_height = get_content_height();
    float visible_height = GetScreenHeight() - (HEADER_HEIGHT * scale_y);
    float scroll_range = content_height - visible_height;
    float scrollbar_range = scrollbar.height - handle.height;

    if (scrollbar_range > 0)
    {
      float scroll_delta = (drag_delta / scrollbar_range) * scroll_range;
      scroll_offset = drag_start_scroll + scroll_delta;
    }
  }

  // Handle clicking on scrollbar background
  if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && mouse_over_scrollbar && !mouse_over_handle)
  {
    float click_pos = (mouse.y - scrollbar.y) / scrollbar.height;
    float content_height = get_content_height();
    float visible_height = GetScreenHeight() - (HEADER_HEIGHT * scale_y);
    scroll_offset = (content_height - visible_height) * click_pos;
  }

  // Debug output
  if (wheel != 0 || IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_DOWN))
  {
    printf("Previous scroll: %f\n", prev_scroll);
    printf("New scroll: %f\n", scroll_offset);
    printf("Content height: %f\n", content_height);
    printf("Visible height: %f\n", visible_height);
    printf("Max scroll: %f\n", max_scroll);
    printf("-------------------\n");
  }
}

void draw_schedule_items(void)
{
  update_scaling();
  update_scroll();

  int scaled_header_height = HEADER_HEIGHT * scale_y;
  int scaled_item_height = ITEM_HEIGHT * scale_y;
  int scaled_padding = ITEM_PADDING * scale_y;
  time_t now = time(NULL);

  // Draw header (fixed position)
  DrawText("Daily Schedule",
           40 * scale_x,
           20 * scale_y,
           30 * scale_y,
           BLACK);

  char current_time[20];
  struct tm *tm_now = localtime(&now);
  sprintf(current_time, "Current Time: %02d:%02d", tm_now->tm_hour, tm_now->tm_min);
  DrawText(current_time,
           GetScreenWidth() - (300 * scale_x),
           20 * scale_y,
           30 * scale_y,
           BLACK);

  // Begin scissor mode for scrolling content
  BeginScissorMode(0, scaled_header_height,
                   GetScreenWidth(),
                   GetScreenHeight() - scaled_header_height);

  for (int i = 0; i < schedule.count; i++)
  {
    float y_pos = scaled_header_height +
                  (i * (scaled_item_height + 2 * scaled_padding)) -
                  scroll_offset;

    // Skip items that are outside the visible area
    if (y_pos + scaled_item_height + (2 * scaled_padding) < scaled_header_height ||
        y_pos > GetScreenHeight())
    {
      continue;
    }

    ScheduleItem *item = &schedule.items[i];
    Rectangle itemRect = {
        scaled_padding,
        y_pos + scaled_padding,
        GetScreenWidth() - (2 * scaled_padding),
        scaled_item_height};

    // Rest of the drawing code remains the same
    Color bgColor = (i % 2 == 0) ? LIGHT_BLUE : LIGHT_PURPLE;
    DrawRectangleRounded(itemRect, 0.1f, 8 * scale_y, bgColor);

    if (now >= item->start_time && now <= item->end_time)
    {
      DrawRectangleRoundedLines(itemRect, 0.1f, 8, PURPLE);

      float completion = get_item_completion(item);
      Rectangle progressRect = itemRect;
      progressRect.width = (progressRect.width * completion) / 100.0f;
      DrawRectangleRounded(progressRect, 0.1f, 8 * scale_y,
                           (Color){PURPLE.r, PURPLE.g, PURPLE.b, 40});

      // Draw "Current" tag
      Rectangle tagRect = {
          itemRect.x + itemRect.width - (120 * scale_x),
          itemRect.y + (20 * scale_y),
          100 * scale_x,
          35 * scale_y};
      DrawRectangleRounded(tagRect, 0.3f, 8 * scale_y, PURPLE);
      DrawText("Current",
               tagRect.x + (10 * scale_x),
               tagRect.y + (8 * scale_y),
               20 * scale_y,
               WHITE);

      // Draw completion percentage
      char percent[10];
      sprintf(percent, "%.1f%%", completion);
      DrawText(percent,
               itemRect.x + itemRect.width - (120 * scale_x),
               itemRect.y + itemRect.height - (40 * scale_y),
               20 * scale_y,
               DARK_GRAY);
    }

    // Draw title
    DrawText(item->title,
             itemRect.x + (20 * scale_x),
             itemRect.y + (20 * scale_y),
             30 * scale_y,
             BLACK);

    // Update time display using localtime_r
    char timeText[50];
    struct tm start_tm, end_tm;
    localtime_r(&item->start_time, &start_tm);
    localtime_r(&item->end_time, &end_tm);

    sprintf(timeText, "%02d:%02d - %02d:%02d",
            start_tm.tm_hour, start_tm.tm_min,
            end_tm.tm_hour, end_tm.tm_min);

    DrawText(timeText,
             itemRect.x + (20 * scale_x),
             itemRect.y + itemRect.height - (40 * scale_y),
             25 * scale_y,
             (Color){100, 100, 100, 255});
  }

  EndScissorMode();

  // Draw scrollbar if needed
  float content_height = get_content_height();
  float visible_height = GetScreenHeight() - (HEADER_HEIGHT * scale_y);

  if (content_height > visible_height)
  {
    Rectangle scrollbar = get_scrollbar_bounds();
    Rectangle handle = get_scrollbar_handle_bounds();

    // Draw scrollbar background
    DrawRectangleRec(scrollbar, (Color){230, 230, 230, 255});

    // Draw handle with hover effect
    Color handleColor = PURPLE;
    handleColor.a = CheckCollisionPointRec(GetMousePosition(), handle) ? 255 : 200;
    DrawRectangleRounded(handle, 0.5f, 8, handleColor);
  }

  // Remove the old scroll indicators
}

// Add this function before init_schdl
time_t make_time(int hour, int min)
{
  // Create a new time_t for today at the specified hour/minute
  time_t now = time(NULL);
  struct tm today;
  localtime_r(&now, &today); // Thread-safe version

  // Set the time components
  today.tm_hour = hour;
  today.tm_min = min;
  today.tm_sec = 0;

  // Convert to time_t
  time_t result = mktime(&today);

  printf("Setting time to %02d:%02d\n", hour, min); // Debug print

  return result;
}

void init_schdl()
{
  // SetWindowState(FLAG_WINDOW_RESIZABLE);

  schedule.count = 0;
  schedule.current_time = time(NULL);

  // Breakfast: 8:00 - 9:00
  time_t start = make_time(8, 0);
  time_t end = make_time(9, 0);
  ScheduleItem breakfast = {
      .title = "Breakfast",
      .start_time = start,
      .end_time = end};
  schedule.items[schedule.count++] = breakfast;

  // Morning work: 9:00 - 13:00
  start = make_time(9, 0);
  end = make_time(13, 0);
  ScheduleItem work1 = {
      .title = "Work Session",
      .start_time = start,
      .end_time = end};
  schedule.items[schedule.count++] = work1;

  // Lunch: 13:00 - 14:00
  start = make_time(13, 0);
  end = make_time(14, 0);
  ScheduleItem lunch = {
      .title = "Lunch",
      .start_time = start,
      .end_time = end};
  schedule.items[schedule.count++] = lunch;

  // Afternoon work: 14:00 - 18:00
  start = make_time(14, 0);
  end = make_time(18, 0);
  ScheduleItem work2 = {
      .title = "Work Session",
      .start_time = start,
      .end_time = end};
  schedule.items[schedule.count++] = work2;

  // Dinner: 19:30 - 20:30
  start = make_time(19, 30);
  end = make_time(20, 30);
  ScheduleItem dinner = {
      .title = "Dinner",
      .start_time = start,
      .end_time = end};
  schedule.items[schedule.count++] = dinner;

  // Add more test items
  for (int i = 0; i < 3; i++)
  {
    time_t start = make_time(21 + i, 0);
    time_t end = make_time(22 + i, 0);
    char title[100];
    sprintf(title, "Evening Activity %d", i + 1);
    ScheduleItem item = {
        .start_time = start,
        .end_time = end};
    strcpy(item.title, title);
    schedule.items[schedule.count++] = item;
  }

  // Debug print to verify times
  for (int i = 0; i < schedule.count; i++)
  {
    struct tm start_tm, end_tm;
    localtime_r(&schedule.items[i].start_time, &start_tm);
    localtime_r(&schedule.items[i].end_time, &end_tm);

    printf("Item %d (%s): %02d:%02d - %02d:%02d\n",
           i,
           schedule.items[i].title,
           start_tm.tm_hour, start_tm.tm_min,
           end_tm.tm_hour, end_tm.tm_min);
  }
}

int main()
{
  SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
  SetTraceLogLevel(LOG_WARNING); // Change to see more debug info
  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Daily Schedule");
  SetTargetFPS(60);

  init_schdl();

  while (!WindowShouldClose())
  {
    BeginDrawing();
    ClearBackground(RAYWHITE);

    // Handle minimum window size
    if (GetScreenWidth() < 400)
      SetWindowSize(400, GetScreenHeight());
    if (GetScreenHeight() < 500)
      SetWindowSize(GetScreenWidth(), 500);

    draw_schedule_items();
    EndDrawing();
  }

  CloseWindow();
  return 0;
}