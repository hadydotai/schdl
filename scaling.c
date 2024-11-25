#include <stdlib.h>
#include "scaling.h"

#define MAX_LISTENERS 32

static float scale_x = 1.0f;
static float scale_y = 1.0f;
static int base_width;
static int base_height;

static scale_listener_t listeners[MAX_LISTENERS];
static int listener_count = 0;

void scaling_init(int width, int height)
{
  base_width = width;
  base_height = height;
  listener_count = 0;
}

void scaling_update(void)
{
  float new_scale_x = (float)GetScreenWidth() / base_width;
  float new_scale_y = (float)GetScreenHeight() / base_height;

  if (new_scale_x != scale_x || new_scale_y != scale_y)
  {
    scale_x = new_scale_x;
    scale_y = new_scale_y;

    // Notify all listeners
    for (int i = 0; i < listener_count; i++)
    {
      listeners[i].callback(scale_x, scale_y, listeners[i].user_data);
    }
  }
}

float scaling_get_x(void)
{
  return scale_x;
}

float scaling_get_y(void)
{
  return scale_y;
}

Vector2 scaling_apply(Vector2 vec)
{
  return (Vector2){
      vec.x * scale_x,
      vec.y * scale_y};
}

float scaling_apply_x(float x)
{
  return x * scale_x;
}

float scaling_apply_y(float y)
{
  return y * scale_y;
}

void scaling_add_listener(scale_listener_fn callback, void *user_data)
{
  if (listener_count < MAX_LISTENERS)
  {
    listeners[listener_count] = (scale_listener_t){
        .callback = callback,
        .user_data = user_data};
    listener_count++;
  }
}

void scaling_cleanup(void)
{
  listener_count = 0;
}