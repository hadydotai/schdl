#ifndef DATA_H
#define DATA_H

#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <time.h>
#include <stdbool.h>

typedef struct schedule_item
{
  char title[100];
  time_t start;
  time_t end;
  bool is_current;
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

schedule_t *create_schedule();
void add_item(schedule_t *schedule, schedule_item_t item);
void remove_item(schedule_t *schedule, int index);
void resize_schedule(schedule_t *schedule, int new_size);
void destroy_schedule(schedule_t *schedule);

time_t make_time(int hour, int min);

#endif // DATA_H