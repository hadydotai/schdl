#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "parser.h"

#define MAX_LINE_LENGTH 256

static bool parse_time_range(const char *range_str, time_t *start, time_t *end, parse_error_t *error);
static bool parse_schedule_line(const char *line, schedule_item_t *item, parse_error_t *error);
static void trim(char *str);

schedule_t *parse_schedule_file(const char *filename, parse_error_t *error)
{
  FILE *file = fopen(filename, "r");
  if (!file)
  {
    if (error)
      *error = PARSE_ERROR_FILE_NOT_FOUND;
    return NULL;
  }

  schedule_t *schedule = create_schedule();
  if (!schedule)
  {
    fclose(file);
    if (error)
      *error = PARSE_ERROR_MEMORY;
    return NULL;
  }

  char line[MAX_LINE_LENGTH];
  while (fgets(line, sizeof(line), file))
  {
    // Skip empty lines
    if (strlen(line) <= 1)
      continue;

    // Remove newline
    line[strcspn(line, "\n")] = 0;

    schedule_item_t item;
    if (!parse_schedule_line(line, &item, error))
    {
      destroy_schedule(schedule);
      fclose(file);
      return NULL;
    }

    add_item(schedule, item);
  }

  fclose(file);
  if (error)
    *error = PARSE_SUCCESS;
  return schedule;
}

const char *parse_error_to_string(parse_error_t error)
{
  switch (error)
  {
  case PARSE_ERROR_FILE_NOT_FOUND:
    return "File not found";
  case PARSE_ERROR_INVALID_TIME_FORMAT:
    return "Invalid time format";
  case PARSE_ERROR_INVALID_LINE_FORMAT:
    return "Invalid line format";
  case PARSE_ERROR_MEMORY:
    return "Memory allocation failed";
  case PARSE_SUCCESS:
    return "Success";
  default:
    return "Unknown error";
  }
}

time_t parse_time(const char *time_str, parse_error_t *error)
{
  char str[32];
  memset(str, 0, sizeof(str));
  strncpy(str, time_str, sizeof(str) - 1);
  trim(str);

  int hour = 0, minute = 0;
  bool is_pm = false;

  // Convert to lowercase for easier pm/am checking
  for (char *p = str; *p; p++)
  {
    *p = tolower(*p);
  }

  // Check for AM/PM
  char *am_pm = strstr(str, "pm");
  if (am_pm)
  {
    is_pm = true;
    *am_pm = '\0';
  }
  else
  {
    am_pm = strstr(str, "am");
    if (am_pm)
    {
      *am_pm = '\0';
    }
  }

  trim(str);

  if (sscanf(str, "%d:%d", &hour, &minute) != 2)
  {
    if (error)
      *error = PARSE_ERROR_INVALID_TIME_FORMAT;
    return (time_t)-1;
  }

  if (is_pm && hour != 12)
    hour += 12;
  if (!is_pm && hour == 12)
    hour = 0;

  if (hour < 0 || hour > 23 || minute < 0 || minute > 59)
  {
    if (error)
      *error = PARSE_ERROR_INVALID_TIME_FORMAT;
    return (time_t)-1;
  }

  return make_time(hour, minute);
}

static bool parse_time_range(const char *range_str, time_t *start, time_t *end, parse_error_t *error)
{
  char start_str[32];
  char end_str[32];

  memset(start_str, 0, sizeof(start_str));
  memset(end_str, 0, sizeof(end_str));

  // Split on '-'
  const char *separator = strstr(range_str, "-");
  if (!separator)
  {
    if (error)
      *error = PARSE_ERROR_INVALID_TIME_FORMAT;
    return false;
  }

  size_t start_len = separator - range_str;
  if (start_len >= sizeof(start_str))
  {
    if (error)
      *error = PARSE_ERROR_INVALID_TIME_FORMAT;
    return false;
  }

  strncpy(start_str, range_str, start_len);

  size_t end_len = strlen(separator + 1);
  if (end_len >= sizeof(end_str))
  {
    if (error)
      *error = PARSE_ERROR_INVALID_TIME_FORMAT;
    return false;
  }

  strncpy(end_str, separator + 1, sizeof(end_str) - 1);

  *start = parse_time(start_str, error);
  if (*start == (time_t)-1)
    return false;

  *end = parse_time(end_str, error);
  if (*end == (time_t)-1)
    return false;

  return true;
}

static bool parse_schedule_line(const char *line, schedule_item_t *item, parse_error_t *error)
{
  char title[100];
  char time_range[64];

  // Find the colon that separates title from time range
  const char *colon = strchr(line, ':');
  if (!colon)
  {
    if (error)
      *error = PARSE_ERROR_INVALID_LINE_FORMAT;
    return false;
  }

  // Extract title safely
  size_t title_len = colon - line;
  if (title_len >= sizeof(title))
  {
    if (error)
      *error = PARSE_ERROR_INVALID_LINE_FORMAT;
    return false;
  }

  memset(title, 0, sizeof(title));
  strncpy(title, line, title_len);

  // Extract time range safely
  size_t remaining_len = strlen(colon + 1);
  if (remaining_len >= sizeof(time_range))
  {
    if (error)
      *error = PARSE_ERROR_INVALID_LINE_FORMAT;
    return false;
  }

  memset(time_range, 0, sizeof(time_range));
  strncpy(time_range, colon + 1, sizeof(time_range) - 1);

  // Remove trailing period
  char *period = strrchr(time_range, '.');
  if (period)
    *period = '\0';

  trim(time_range);
  trim(title);

  // Parse the time range
  if (!parse_time_range(time_range, &item->start, &item->end, error))
  {
    return false;
  }

  // Set the title and type safely
  memset(item->title, 0, sizeof(item->title));
  strncpy(item->title, title, sizeof(item->title) - 1);

  // Determine item type (event or break)
  item->type = (title[0] == '-') ? SCHEDULE_ITEM_TYPE_BREAK : SCHEDULE_ITEM_TYPE_EVENT;

  return true;
}

static void trim(char *str)
{
  char *start = str;
  char *end = str + strlen(str) - 1;

  while (isspace(*start))
    start++;
  while (end > start && isspace(*end))
    end--;

  size_t len = end - start + 1;
  memmove(str, start, len);
  str[len] = '\0';
}