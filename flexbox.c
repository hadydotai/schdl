#include <stdlib.h>
#include <math.h>
#include "scrollable.h"
#include "flexbox.h"
#include "scaling.h"

//------------------------------------------------------------------------------
// Creation and destruction
//------------------------------------------------------------------------------

fbox_context_t fbox_create(Rectangle bounds, fbox_direction_t direction, scrollable_t *scrollable)
{
  fbox_context_t ctx = {
      .bounds = bounds,
      .direction = direction,
      .main_align = fbox_ALIGN_START,
      .cross_align = fbox_ALIGN_START,
      .gap = 0,
      .padding_y = 0,
      .padding_x = 0,
      .expected_items = 1,
      .content_height = 0,
      .content_width = 0,
      .size_mode = fbox_SIZE_FIXED,
      .scrollable = scrollable};
  return ctx;
}

fbox_context_t fbox_create_nested(fbox_context_t *parent, Rectangle bounds)
{
  fbox_context_t ctx = {
      .bounds = bounds,
      .direction = parent->direction,
      .scrollable = parent->scrollable};
  return ctx;
}

void fbox_destroy(fbox_context_t *ctx)
{
  if (ctx->scrollable != NULL)
  {
    float total_height = ctx->content_height;
    ctx->scrollable->last_y_pos = fmaxf(ctx->scrollable->last_y_pos, total_height);
  }
  free(ctx->item_sizes);
}

//------------------------------------------------------------------------------
// Configuration functions
//------------------------------------------------------------------------------

void fbox_set_direction(fbox_context_t *ctx, fbox_direction_t direction)
{
  ctx->direction = direction;
}

void fbox_set_main_align(fbox_context_t *ctx, fbox_align_t align)
{
  ctx->main_align = align;
}

void fbox_set_cross_align(fbox_context_t *ctx, fbox_align_t align)
{
  ctx->cross_align = align;
}

void fbox_set_gap(fbox_context_t *ctx, float gap)
{
  ctx->gap = scaling_apply_y(gap);
}

void fbox_set_padding(fbox_context_t *ctx, float padding)
{
  ctx->padding_x = scaling_apply_x(padding);
  ctx->padding_y = scaling_apply_y(padding);
}

void fbox_set_padding_y(fbox_context_t *ctx, float padding)
{
  ctx->padding_y = scaling_apply_y(padding);
}

void fbox_set_padding_x(fbox_context_t *ctx, float padding)
{
  ctx->padding_x = scaling_apply_x(padding);
}

void fbox_set_expected_items(fbox_context_t *ctx, int count)
{
  ctx->expected_items = count;
}

void fbox_set_size_mode(fbox_context_t *ctx, fbox_size_mode_t mode)
{
  ctx->size_mode = mode;
}

//------------------------------------------------------------------------------
// Layout functions
//------------------------------------------------------------------------------

Rectangle fbox_next(fbox_context_t *ctx, Vector2 size)
{
  // Initialize item tracking on first call
  if (ctx->item_sizes == NULL)
  {
    ctx->max_items = ctx->expected_items;
    ctx->item_sizes = malloc(sizeof(Vector2) * ctx->max_items);
    ctx->item_positions = malloc(sizeof(Rectangle *) * ctx->max_items);
    ctx->item_count = 0;
    ctx->current_pos = ctx->padding_x;
  }

  // Handle size stretching if enabled
  Vector2 final_size = size;
  if (ctx->size_mode == fbox_SIZE_STRETCH)
  {
    if (ctx->direction == fbox_DIRECTION_ROW)
    {
      final_size.y = ctx->bounds.height - (2 * ctx->padding_y);
    }
    else
    {
      final_size.x = ctx->bounds.width - (2 * ctx->padding_x);
    }
  }

  ctx->item_sizes[ctx->item_count] = final_size;

  float x = ctx->bounds.x;
  float y = ctx->bounds.y;

  // Layout items in a row
  if (ctx->direction == fbox_DIRECTION_ROW)
  {
    x += ctx->current_pos;

    // Handle main-axis alignment
    if (ctx->main_align == fbox_ALIGN_CENTER)
    {
      x += (ctx->bounds.width - final_size.x) / 2;
    }
    else if (ctx->main_align == fbox_ALIGN_END)
    {
      x += ctx->bounds.width - final_size.x - ctx->padding_x;
    }
    else if (ctx->main_align == fbox_ALIGN_SPACE_BETWEEN)
    {
      // Calculate total width of all items including current one
      float total_items_width = final_size.x;
      for (int i = 0; i < ctx->item_count; i++)
      {
        total_items_width += ctx->item_sizes[i].x;
      }

      // Calculate space between items
      float space_between = 0;
      if (ctx->expected_items > 1)
      {
        float available_space = ctx->bounds.width - total_items_width - (2 * ctx->padding_x);
        space_between = available_space / (ctx->expected_items - 1);
      }

      x = ctx->bounds.x + ctx->padding_x;

      // Add space and width of previous items to get current x position
      if (ctx->item_count > 0)
      {
        x += ctx->item_count * space_between;
        for (int i = 0; i < ctx->item_count; i++)
        {
          x += ctx->item_sizes[i].x;
        }
      }

      // Update positions of previous items
      for (int i = 0; i < ctx->item_count; i++)
      {
        Rectangle *prev_item = ctx->item_positions[i];
        if (prev_item != NULL)
        {
          prev_item->x = ctx->bounds.x + ctx->padding_x;
          if (i > 0)
          {
            prev_item->x += i * space_between;
            for (int j = 0; j < i; j++)
            {
              prev_item->x += ctx->item_sizes[j].x;
            }
          }
        }
      }
    }
    else
    {
      x += ctx->padding_x;
    }

    // Handle cross-axis alignment
    if (ctx->cross_align == fbox_ALIGN_CENTER)
    {
      y += (ctx->bounds.height - final_size.y) / 2;
    }
    else if (ctx->cross_align == fbox_ALIGN_END)
    {
      y += ctx->bounds.height - final_size.y - ctx->padding_y;
    }
    else if (ctx->cross_align == fbox_ALIGN_SPACE_BETWEEN)
    {
      y += ctx->padding_y;
      float space = (ctx->bounds.height - final_size.y - (2 * ctx->padding_y)) / (ctx->expected_items - 1);
      y += space * ctx->item_count;
    }
    else
    {
      y += ctx->padding_y;
    }

    ctx->current_pos += final_size.x;

    if (ctx->item_count < ctx->expected_items - 1)
    {
      ctx->current_pos += ctx->gap;
    }

    if (ctx->item_count == ctx->expected_items - 1)
    {
      ctx->current_pos += ctx->padding_x;
    }

    ctx->content_width = ctx->current_pos;
  }
  // Layout items in a column
  else
  {
    y += ctx->current_pos;

    // Handle cross-axis alignment
    if (ctx->cross_align == fbox_ALIGN_CENTER)
    {
      x += (ctx->bounds.width - final_size.x) / 2;
    }
    else if (ctx->cross_align == fbox_ALIGN_END)
    {
      x += ctx->bounds.width - final_size.x - ctx->padding_x;
    }
    else
    {
      x += ctx->padding_x;
    }

    ctx->current_pos += final_size.y;

    if (ctx->item_count < ctx->expected_items - 1)
    {
      ctx->current_pos += ctx->gap;
    }

    if (ctx->item_count == ctx->expected_items - 1)
    {
      ctx->current_pos += ctx->padding_y;
    }

    ctx->content_height = ctx->current_pos;

    // Track maximum width for column layout
    if (final_size.x > ctx->content_width)
    {
      ctx->content_width = final_size.x;
    }
  }

  Rectangle *item_position = (Rectangle *)malloc(sizeof(Rectangle));
  item_position->x = x;
  item_position->y = y;
  item_position->width = final_size.x;
  item_position->height = final_size.y;
  ctx->item_positions[ctx->item_count] = item_position;

  ctx->item_count++;

  // Reset tracking after processing all items
  if (ctx->item_count == ctx->expected_items)
  {
    free(ctx->item_sizes);
    free(ctx->item_positions);
    ctx->item_sizes = NULL;
    ctx->item_positions = NULL;
    ctx->item_count = 0;
    ctx->current_pos = ctx->padding_x;
  }

  return *item_position;
}

Vector2 fbox_next_position(fbox_context_t *ctx)
{
  return (Vector2){ctx->current_pos, ctx->current_pos};
}

//------------------------------------------------------------------------------
// Layout information
//------------------------------------------------------------------------------

float fbox_get_content_height(fbox_context_t *ctx)
{
  return ctx->content_height;
}

float fbox_get_content_width(fbox_context_t *ctx)
{
  return ctx->content_width;
}