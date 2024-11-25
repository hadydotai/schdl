#ifndef DATA_H
#define DATA_H

#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <time.h>
#include <stdbool.h>

typedef enum schedule_item_type
{
  SCHEDULE_ITEM_TYPE_EVENT,
  SCHEDULE_ITEM_TYPE_BREAK
} schedule_item_type_t;

typedef struct schedule_item
{
  char title[100];
  time_t start;
  time_t end;
  schedule_item_type_t type;
} schedule_item_t;

typedef struct schedule
{
  schedule_item_t *items;
  int count;
  int capacity;
  time_t current_time;
} schedule_t;

typedef struct schedule_iterator
{
  schedule_t *schedule;
  int index;
} schedule_iterator_t;

schedule_iterator_t *create_iterator(schedule_t *schedule);
void destroy_iterator(schedule_iterator_t *iterator);
schedule_item_t *get_current_item(schedule_iterator_t *iterator);
schedule_item_t *get_next_item(schedule_iterator_t *iterator);
schedule_item_t *get_previous_item(schedule_iterator_t *iterator);
bool is_item_current(schedule_item_t *item);
bool is_item_past(schedule_item_t *item);

schedule_t *create_schedule();
void add_item(schedule_t *schedule, schedule_item_t item);
void remove_item(schedule_t *schedule, int index);
void resize_schedule(schedule_t *schedule, int new_size);
void destroy_schedule(schedule_t *schedule);

time_t make_time(int hour, int min);
char *format_time(time_t time);
char *format_time_12hr(time_t time);
char *format_duration(time_t start, time_t end);
char *format_duration_12hr(time_t start, time_t end);
char *format_date(time_t date);

#endif // DATA_H