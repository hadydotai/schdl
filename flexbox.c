#include <stdlib.h>
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
      .padding = 0,
      .base_gap = (Vector2){0, 0},
      .base_padding = (Vector2){0, 0},
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
  ctx->base_gap = (Vector2){gap, gap};
  ctx->gap = scaling_apply_y(gap);
}

void fbox_set_padding(fbox_context_t *ctx, float padding)
{
  ctx->base_padding = (Vector2){padding, padding};
  ctx->padding = scaling_apply_x(padding);
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
    ctx->item_count = 0;
    ctx->current_pos = ctx->padding;
  }

  // Handle size stretching if enabled
  Vector2 final_size = size;
  if (ctx->size_mode == fbox_SIZE_STRETCH)
  {
    if (ctx->direction == fbox_DIRECTION_ROW)
    {
      final_size.y = ctx->bounds.height - (2 * ctx->padding);
    }
    else
    {
      final_size.x = ctx->bounds.width - (2 * ctx->padding);
    }
  }

  ctx->item_sizes[ctx->item_count] = final_size;

  float x = ctx->bounds.x;
  float y = ctx->bounds.y;

  // Layout items in a row
  if (ctx->direction == fbox_DIRECTION_ROW)
  {
    x += ctx->current_pos;

    // Handle cross-axis alignment
    if (ctx->cross_align == fbox_ALIGN_CENTER)
    {
      y += (ctx->bounds.height - final_size.y) / 2;
    }
    else if (ctx->cross_align == fbox_ALIGN_END)
    {
      y += ctx->bounds.height - final_size.y - ctx->padding;
    }
    else
    {
      y += ctx->padding;
    }

    ctx->current_pos += final_size.x;

    if (ctx->item_count < ctx->expected_items - 1)
    {
      ctx->current_pos += ctx->gap;
    }

    if (ctx->item_count == ctx->expected_items - 1)
    {
      ctx->current_pos += ctx->padding;
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
      x += ctx->bounds.width - final_size.x - ctx->padding;
    }
    else
    {
      x += ctx->padding;
    }

    ctx->current_pos += final_size.y;

    if (ctx->item_count < ctx->expected_items - 1)
    {
      ctx->current_pos += ctx->gap;
    }

    if (ctx->item_count == ctx->expected_items - 1)
    {
      ctx->current_pos += ctx->padding;
    }

    ctx->content_height = ctx->current_pos;

    // Track maximum width for column layout
    if (final_size.x > ctx->content_width)
    {
      ctx->content_width = final_size.x;
    }
  }

  ctx->item_count++;

  // Reset tracking after processing all items
  if (ctx->item_count == ctx->expected_items)
  {
    free(ctx->item_sizes);
    ctx->item_sizes = NULL;
    ctx->item_count = 0;
    ctx->current_pos = ctx->padding;
  }

  return (Rectangle){x, y, final_size.x, final_size.y};
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

//------------------------------------------------------------------------------
// Scaling update handler
//------------------------------------------------------------------------------

static void fbox_update_scaling(fbox_context_t *ctx, float scale_x, float scale_y)
{
  ctx->gap = ctx->base_gap.y * scale_y;
  ctx->padding = ctx->base_padding.x * scale_x;
}