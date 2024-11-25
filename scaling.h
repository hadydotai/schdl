#ifndef SCALING_H
#define SCALING_H

#include "raylib.h"

typedef void (*scale_listener_fn)(float scale_x, float scale_y, void *user_data);

typedef struct
{
  scale_listener_fn callback;
  void *user_data;
} scale_listener_t;

void scaling_init(int base_width, int base_height);
void scaling_update(void);
float scaling_get_x(void);
float scaling_get_y(void);
Vector2 scaling_apply(Vector2 vec);
float scaling_apply_x(float x);
float scaling_apply_y(float y);
void scaling_add_listener(scale_listener_fn callback, void *user_data);
void scaling_cleanup(void);

#endif