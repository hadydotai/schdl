#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include "data.h"

#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS // No warnings about localtime_s
#endif

schedule_iterator_t *create_iterator(schedule_t *schedule)
{
  schedule_iterator_t *iterator = (schedule_iterator_t *)malloc(sizeof(schedule_iterator_t));
  iterator->schedule = schedule;
  iterator->index = 0;
  return iterator;
}

void destroy_iterator(schedule_iterator_t *iterator)
{
  free(iterator);
}

schedule_item_t *get_current_item(schedule_iterator_t *iterator)
{
  if (iterator->index < 0 || iterator->index >= iterator->schedule->count)
  {
    return NULL;
  }
  return &iterator->schedule->items[iterator->index];
}

schedule_item_t *get_next_item(schedule_iterator_t *iterator)
{
  iterator->index++;
  if (iterator->index >= iterator->schedule->count)
  {
    iterator->index = iterator->schedule->count;
    return NULL;
  }
  return &iterator->schedule->items[iterator->index];
}

schedule_item_t *get_previous_item(schedule_iterator_t *iterator)
{
  if (iterator->index == 0)
  {
    return NULL;
  }
  iterator->index--;
  return &iterator->schedule->items[iterator->index];
}

schedule_t *create_schedule()
{
  schedule_t *schedule = (schedule_t *)malloc(sizeof(schedule_t));
  schedule->items = (schedule_item_t *)malloc(sizeof(schedule_item_t) * 10);
  schedule->count = 0;
  schedule->capacity = 10;
  schedule->current_time = time(NULL);
  return schedule;
}

void add_item(schedule_t *schedule, schedule_item_t item)
{
  if (schedule->count + 1 >= schedule->capacity)
  {
    resize_schedule(schedule, schedule->capacity * 2);
  }
  schedule->items[schedule->count++] = item;
}

void remove_item(schedule_t *schedule, int index)
{
  for (int i = index; i < schedule->count - 1; i++)
  {
    schedule->items[i] = schedule->items[i + 1];
  }
  schedule->count--;
}

void resize_schedule(schedule_t *schedule, int new_size)
{
  schedule->items = (schedule_item_t *)realloc(schedule->items, sizeof(schedule_item_t) * new_size);
  schedule->capacity = new_size;
}

void destroy_schedule(schedule_t *schedule)
{
  free(schedule->items);
  free(schedule);
}

time_t make_time(int hour, int min)
{
  time_t now = time(NULL);
  struct tm today;
#ifdef _WIN32
  localtime_s(&today, &now);
#else
  localtime_r(&now, &today);
#endif
  today.tm_hour = hour;
  today.tm_min = min;
  today.tm_sec = 0;
  return mktime(&today);
}

char *format_time(time_t time)
{
  struct tm tm;
  localtime_r(&time, &tm);
  char *time_str = (char *)malloc(100);
  sprintf(time_str, "%02d:%02d", tm.tm_hour, tm.tm_min);
  return time_str;
}

char *format_duration(time_t start, time_t end)
{
  char *duration_str = (char *)malloc(100);
  sprintf(duration_str, "%s - %s", format_time(start), format_time(end));
  return duration_str;
}

char *format_date(time_t date)
{
  struct tm tm;
  localtime_r(&date, &tm);
  char *date_str = (char *)malloc(100);
  sprintf(date_str, "%02d.%02d.%04d", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900);
  return date_str;
}

#ifdef _WIN32
static inline struct tm *localtime_r(const time_t *timep, struct tm *result)
{
  localtime_s(result, timep);
  return result;
}
#endif