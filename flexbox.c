#include <stdlib.h>
#include "flexbox.h"

fb_context_t fb_create(Rectangle bounds, fb_direction_t direction)
{
  fb_context_t ctx = {
      .bounds = bounds,
      .direction = direction,
      .main_align = FB_ALIGN_START,
      .cross_align = FB_ALIGN_START,
      .gap = 0,
      .padding = 0,
      .expected_items = 1};
  return ctx;
}

void fb_set_main_align(fb_context_t *ctx, fb_align_t align)
{
  ctx->main_align = align;
}

void fb_set_cross_align(fb_context_t *ctx, fb_align_t align)
{
  ctx->cross_align = align;
}

void fb_set_gap(fb_context_t *ctx, float gap)
{
  ctx->gap = gap;
}

void fb_set_padding(fb_context_t *ctx, float padding)
{
  ctx->padding = padding;
}

void fb_set_expected_items(fb_context_t *ctx, int count)
{
  ctx->expected_items = count;
}

Rectangle fb_next(fb_context_t *ctx, Vector2 size)
{
  // Initialize item_sizes array if needed
  if (ctx->item_sizes == NULL)
  {
    ctx->max_items = ctx->expected_items;
    ctx->item_sizes = malloc(sizeof(Vector2) * ctx->max_items);
    ctx->item_count = 0;
    ctx->current_pos = ctx->padding;
  }

  // Store current item size
  ctx->item_sizes[ctx->item_count] = size;

  float x = ctx->bounds.x;
  float y = ctx->bounds.y;

  if (ctx->direction == FB_DIRECTION_ROW)
  {
    // Calculate position based on previous items
    x += ctx->current_pos;

    // Apply cross alignment for y position
    if (ctx->cross_align == FB_ALIGN_CENTER)
    {
      y += (ctx->bounds.height - size.y) / 2;
    }
    else if (ctx->cross_align == FB_ALIGN_END)
    {
      y += ctx->bounds.height - size.y - ctx->padding;
    }
    else
    {
      y += ctx->padding;
    }

    // Update current_pos for next item
    ctx->current_pos += size.x + ctx->gap;
  }
  else // FB_DIRECTION_COLUMN
  {
    // Calculate position based on previous items
    y += ctx->current_pos;

    // Apply cross alignment for x position
    if (ctx->cross_align == FB_ALIGN_CENTER)
    {
      x += (ctx->bounds.width - size.x) / 2;
    }
    else if (ctx->cross_align == FB_ALIGN_END)
    {
      x += ctx->bounds.width - size.x - ctx->padding;
    }
    else
    {
      x += ctx->padding;
    }

    // Update current_pos for next item
    ctx->current_pos += size.y;

    // Add gap only if this isn't the last item
    if (ctx->item_count < ctx->expected_items - 1)
    {
      ctx->current_pos += ctx->gap;
    }
  }

  ctx->item_count++;

  // Reset if we've processed all expected items
  if (ctx->item_count == ctx->expected_items)
  {
    free(ctx->item_sizes);
    ctx->item_sizes = NULL;
    ctx->item_count = 0;
    ctx->current_pos = ctx->padding;
  }

  return (Rectangle){x, y, size.x, size.y};
}

float fb_get_content_height(fb_context_t *ctx)
{
  if (ctx->item_count == 0)
  {
    return 0;
  }

  if (ctx->direction == FB_DIRECTION_COLUMN)
  {
    float total_height = 0;

    // Sum up heights of all items
    for (int i = 0; i < ctx->item_count; i++)
    {
      total_height += ctx->item_sizes[i].y;
    }

    // Add gaps between items (not after the last item)
    if (ctx->item_count > 1)
    {
      total_height += ctx->gap * (ctx->item_count - 1);
    }

    // Add padding for top and bottom
    total_height += ctx->padding * 2;

    return total_height;
  }
  else // Row direction
  {
    // Find tallest item
    float max_height = 0;
    for (int i = 0; i < ctx->item_count; i++)
    {
      if (ctx->item_sizes[i].y > max_height)
      {
        max_height = ctx->item_sizes[i].y;
      }
    }

    // Add padding based on alignment
    return max_height + (ctx->padding * 2);
  }
}

float fb_get_content_width(fb_context_t *ctx)
{
  // For column direction, width should be the container width minus padding
  if (ctx->direction == FB_DIRECTION_COLUMN)
  {
    return ctx->bounds.width - (ctx->padding * 2);
  }
  // For row direction, calculate total width of all items plus gaps
  else
  {
    float total_width = 0;

    // Sum up widths of all items
    for (int i = 0; i < ctx->item_count; i++)
    {
      total_width += ctx->item_sizes[i].x;
    }

    // Add gaps between items
    if (ctx->item_count > 1)
    {
      total_width += ctx->gap * (ctx->item_count - 1);
    }

    // Add padding on both sides
    return total_width + (ctx->padding * 2);
  }
}