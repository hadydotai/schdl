#include "raylib.h"
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "layout.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 900
#define ITEM_HEIGHT 100
#define ITEM_PADDING 10
#define HEADER_HEIGHT 60
#define MAX_SCHEDULE_ITEMS 50
#define SCROLL_SPEED 30.0f
#define SCROLL_MARGIN 20.0f
#define SCROLLBAR_WIDTH 18
#define SCROLLBAR_MIN_HEIGHT 40
#define HEADER_PADDING_HORIZONTAL (24)
#define HEADER_PADDING_VERTICAL (12)

#define LIGHT_BLUE \
  (Color) { 235, 240, 255, 255 }
#define LIGHT_PURPLE \
  (Color) { 245, 235, 255, 255 }
#define DARK_GRAY \
  (Color) { 85, 85, 85, 255 }

#define BASE_WIDTH 800
#define BASE_HEIGHT 900

#define BASE_PADDING (16)
#define BASE_GAP (8)
#define ITEM_VERTICAL_PADDING (16)
#define ITEM_HORIZONTAL_PADDING (16)
#define TAG_VERTICAL_PADDING (6)
#define TAG_HORIZONTAL_PADDING (12)

static float scale_x = 1.0f;
static float scale_y = 1.0f;
static float scroll_offset = 0.0f;
static float max_scroll = 0.0f;
static Font custom_font;

#define FONT_SIZE_LARGE (24)
#define FONT_SIZE_MEDIUM (20)
#define FONT_SIZE_SMALL (16)

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

void update_scaling(void)
{
  scale_x = (float)GetScreenWidth() / BASE_WIDTH;
  scale_y = (float)GetScreenHeight() / BASE_HEIGHT;
}

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
      GetScreenWidth() - SCROLLBAR_WIDTH,
      scaled_header_height,
      SCROLLBAR_WIDTH,
      GetScreenHeight() - scaled_header_height};
}

Rectangle get_scrollbar_handle_bounds(void)
{
  Rectangle scrollbar = get_scrollbar_bounds();
  float content_height = get_content_height();
  float visible_height = GetScreenHeight() - (HEADER_HEIGHT * scale_y);

  float handle_height = (visible_height / content_height) * scrollbar.height;
  if (handle_height < SCROLLBAR_MIN_HEIGHT * scale_y)
    handle_height = SCROLLBAR_MIN_HEIGHT * scale_y;

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
  static bool dragging_scrollbar = false;
  static float drag_start_y = 0;
  static float drag_start_scroll = 0;

  float wheel = GetMouseWheelMove();
  if (wheel != 0)
  {
    scroll_offset -= wheel * SCROLL_SPEED * scale_y; // Changed plus to minus for natural scrolling
  }

  // Support keyboard up/down arrows (natural direction)
  if (IsKeyPressed(KEY_UP))
  {
    scroll_offset -= ITEM_HEIGHT * scale_y; // Move content up
  }
  if (IsKeyPressed(KEY_DOWN))
  {
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

  // // Debug output
  // if (wheel != 0 || IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_DOWN))
  // {
  //   printf("Previous scroll: %f\n", prev_scroll);
  //   printf("New scroll: %f\n", scroll_offset);
  //   printf("Content height: %f\n", content_height);
  //   printf("Visible height: %f\n", visible_height);
  //   printf("Max scroll: %f\n", max_scroll);
  //   printf("-------------------\n");
  // }
}

// Helper functions for text measurements and layout
Vector2 get_text_dimensions(const char *text, float font_size)
{
  Vector2 size = MeasureTextEx(custom_font, text, font_size * scale_y, 0);
  return size;
}

// Get vertical metrics for text layout
typedef struct
{
  float ascent;
  float descent;
  float lineHeight;
} TextMetrics;

TextMetrics get_text_metrics(float font_size)
{
  float scaled_size = font_size * scale_y;
  TextMetrics metrics = {
      .ascent = scaled_size * 0.8f, // Approximate metrics based on font size
      .descent = scaled_size * 0.2f,
      .lineHeight = scaled_size * 1.2f};
  return metrics;
}

// Helper to center text vertically in a rectangle
float get_text_vertical_center(Rectangle bounds, float font_size)
{
  TextMetrics metrics = get_text_metrics(font_size);
  return bounds.y + (bounds.height - metrics.lineHeight) / 2 + metrics.ascent;
}

void draw_schedule_items(void)
{
  update_scaling();
  update_scroll();

  // Create main layout context
  FlexContext main = FlexCreate(
      (Rectangle){0, 0, GetScreenWidth(), GetScreenHeight()},
      FLEX_DIRECTION_COLUMN);
  FlexSetGap(&main, 0);
  FlexSetPadding(&main, 0);
  FlexStart(&main);

  // Content area first
  float content_height = GetScreenHeight() - (HEADER_HEIGHT * scale_y);
  FlexNested(&main,
             (Vector2){GetScreenWidth(), content_height},
             FLEX_DIRECTION_COLUMN);

  // Begin scissor mode for scrolling content
  BeginScissorMode(0, HEADER_HEIGHT * scale_y,
                   GetScreenWidth(),
                   GetScreenHeight() - HEADER_HEIGHT * scale_y);

  // Draw schedule items
  for (int i = 0; i < schedule.count; i++)
  {
    float y_pos = HEADER_HEIGHT * scale_y +
                  (i * (ITEM_HEIGHT + ITEM_PADDING) * scale_y) -
                  scroll_offset;

    // Skip items that are outside the visible area
    if (y_pos + (ITEM_HEIGHT + ITEM_PADDING) * scale_y < HEADER_HEIGHT * scale_y ||
        y_pos > GetScreenHeight())
    {
      continue;
    }

    // Create item layout with adjusted padding
    FlexContext item = FlexCreate(
        (Rectangle){
            ITEM_HORIZONTAL_PADDING * scale_x,
            y_pos,
            GetScreenWidth() - (2 * ITEM_HORIZONTAL_PADDING) * scale_x,
            ITEM_HEIGHT * scale_y},
        FLEX_DIRECTION_COLUMN);
    FlexSetPadding(&item, ITEM_HORIZONTAL_PADDING * scale_x);

    ScheduleItem *item_ptr = &schedule.items[i];
    Rectangle itemRect = {
        item.bounds.x,
        item.bounds.y,
        item.bounds.width,
        item.bounds.height};

    // Rest of the drawing code remains the same
    Color bgColor = (i % 2 == 0) ? LIGHT_BLUE : LIGHT_PURPLE;
    DrawRectangleRounded(itemRect, 0.1f, 8 * scale_y, bgColor);

    if (schedule.current_time >= item_ptr->start_time && schedule.current_time <= item_ptr->end_time)
    {
      DrawRectangleRoundedLines(itemRect, 0.1f, 8, PURPLE);

      float completion = get_item_completion(item_ptr);
      Rectangle progressRect = itemRect;
      progressRect.width = (progressRect.width * completion) / 100.0f;
      DrawRectangleRounded(progressRect, 0.1f, 8 * scale_y,
                           (Color){PURPLE.r, PURPLE.g, PURPLE.b, 40});

      // Calculate tag dimensions based on text size
      float tag_font_size = FONT_SIZE_SMALL;
      Vector2 tag_text_size = get_text_dimensions("Current", tag_font_size);

      // Calculate tag height based on text height plus padding
      float tag_width = tag_text_size.x + (2 * TAG_HORIZONTAL_PADDING * scale_x);
      float tag_height = tag_text_size.y + (2 * TAG_VERTICAL_PADDING * scale_y);

      // Position the tag in the top-right corner with proper padding
      Rectangle tagBounds = {
          itemRect.x + itemRect.width - (tag_width + ITEM_HORIZONTAL_PADDING * scale_x),
          itemRect.y + (ITEM_VERTICAL_PADDING * scale_y),
          tag_width,
          tag_height};

      // Draw the purple background
      DrawRectangleRounded(tagBounds, 0.3f, 8 * scale_y, PURPLE);

      // Center text both horizontally and vertically in the tag
      float text_x = tagBounds.x + (tagBounds.width - tag_text_size.x) / 2;
      float text_y = tagBounds.y + (tagBounds.height - tag_text_size.y) / 2;

      DrawTextEx(custom_font, "Current",
                 (Vector2){text_x, text_y},
                 tag_font_size * scale_y, 0, WHITE);

      // Draw completion percentage under the tag
      char percent[10];
      sprintf(percent, "%.1f%%", completion);
      Vector2 percent_size = get_text_dimensions(percent, FONT_SIZE_SMALL);

      float percent_x = tagBounds.x + (tagBounds.width - percent_size.x) / 2;
      float percent_y = tagBounds.y + tagBounds.height + (4 * scale_y);

      DrawTextEx(custom_font, percent,
                 (Vector2){percent_x, percent_y},
                 FONT_SIZE_SMALL * scale_y, 0, DARK_GRAY);
    }

    // Draw title with proper vertical alignment
    float title_y = itemRect.y + (ITEM_VERTICAL_PADDING * scale_y);
    DrawTextEx(custom_font, item_ptr->title,
               (Vector2){itemRect.x + (ITEM_HORIZONTAL_PADDING * scale_x), title_y},
               FONT_SIZE_LARGE * scale_y, 0, BLACK);

    // Draw time with proper vertical alignment
    float time_y = itemRect.y + itemRect.height -
                   (ITEM_VERTICAL_PADDING * scale_y) -
                   (FONT_SIZE_MEDIUM * scale_y);

    char timeText[50];
    struct tm start_tm, end_tm;
    localtime_r(&item_ptr->start_time, &start_tm);
    localtime_r(&item_ptr->end_time, &end_tm);

    sprintf(timeText, "%02d:%02d - %02d:%02d",
            start_tm.tm_hour, start_tm.tm_min,
            end_tm.tm_hour, end_tm.tm_min);

    DrawTextEx(custom_font, timeText,
               (Vector2){itemRect.x + (ITEM_HORIZONTAL_PADDING * scale_x), time_y},
               FONT_SIZE_MEDIUM * scale_y, 0, (Color){100, 100, 100, 255});
  }

  EndScissorMode();

  // Draw scrollbar if needed
  float content_area_height = get_content_height();
  float visible_height = GetScreenHeight() - (HEADER_HEIGHT * scale_y);

  if (content_area_height > visible_height)
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

  // Header section
  FlexContext header = FlexCreate(
      (Rectangle){0, 0, GetScreenWidth(), HEADER_HEIGHT * scale_y},
      FLEX_DIRECTION_ROW);
  FlexSetMainAlign(&header, ALIGN_SPACE_BETWEEN);
  FlexSetCrossAlign(&header, ALIGN_CENTER);
  FlexSetPadding(&header, HEADER_PADDING_HORIZONTAL * scale_x); // Use new padding constant
  FlexSetExpectedItems(&header, 2);
  FlexStart(&header);

  // Draw header background
  DrawRectangleRec(header.bounds, RAYWHITE);

  // Calculate title dimensions using our helper
  Vector2 titleSize = get_text_dimensions("Daily Schedule", FONT_SIZE_LARGE);
  Rectangle titleRect = FlexNext(&header, titleSize);

  // Center text vertically in header with proper padding
  float header_text_y = titleRect.y + (titleRect.height - titleSize.y) / 2;

  DrawTextEx(custom_font, "Daily Schedule",
             (Vector2){titleRect.x, header_text_y},
             FONT_SIZE_LARGE * scale_y, 0, BLACK);

  time_t now = time(NULL);
  struct tm tm_now;
  localtime_r(&now, &tm_now);

  // Current time with proper vertical alignment
  char current_time[20];
  sprintf(current_time, "Current Time: %02d:%02d", tm_now.tm_hour, tm_now.tm_min);

  Vector2 timeSize = get_text_dimensions(current_time, FONT_SIZE_MEDIUM);
  Rectangle timeRect = FlexNext(&header, timeSize);

  float time_header_y = timeRect.y + (timeRect.height - timeSize.y) / 2;

  DrawTextEx(custom_font, current_time,
             (Vector2){timeRect.x, time_header_y},
             FONT_SIZE_MEDIUM * scale_y, 0, BLACK);

  // Clean up at the end
  FlexEnd(&main);
  FlexEnd(&header);
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

  return result;
}

void init_schdl()
{
  // Load the custom font with better parameters
  const int fontSize = 64;  // Base size for font atlas
  const int numChars = 256; // Load full ASCII range
  custom_font = LoadFontEx("fonts/Roboto-Regular.ttf", fontSize, NULL, numChars);
  SetTextureFilter(custom_font.texture, TEXTURE_FILTER_BILINEAR);

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
    strncpy(item.title, title, sizeof(item.title) - 1);
    item.title[sizeof(item.title) - 1] = '\0';
    schedule.items[schedule.count++] = item;
  }

  // // Debug print to verify times
  // for (int i = 0; i < schedule.count; i++)
  // {
  //   struct tm start_tm, end_tm;
  //   localtime_r(&schedule.items[i].start_time, &start_tm);
  //   localtime_r(&schedule.items[i].end_time, &end_tm);

  //   printf("Item %d (%s): %02d:%02d - %02d:%02d\n",
  //          i,
  //          schedule.items[i].title,
  //          start_tm.tm_hour, start_tm.tm_min,
  //          end_tm.tm_hour, end_tm.tm_min);
  // }
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

  UnloadFont(custom_font);
  CloseWindow();
  return 0;
}