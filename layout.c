#include <stdlib.h>
#include "layout.h"

FlexContext FlexCreate(Rectangle bounds, FlexDirection direction)
{
  FlexContext ctx = {
      .bounds = bounds,
      .direction = direction,
      .mainAlign = ALIGN_START,
      .crossAlign = ALIGN_START,
      .gap = 0,
      .padding = 0,
      .expected_items = 1};
  return ctx;
}

void FlexSetMainAlign(FlexContext *ctx, FlexAlign align)
{
  ctx->mainAlign = align;
}

void FlexSetCrossAlign(FlexContext *ctx, FlexAlign align)
{
  ctx->crossAlign = align;
}

void FlexSetGap(FlexContext *ctx, float gap)
{
  ctx->gap = gap;
}

void FlexSetPadding(FlexContext *ctx, float padding)
{
  ctx->padding = padding;
}

void FlexSetExpectedItems(FlexContext *ctx, int count)
{
  ctx->expected_items = count;
}

static float get_cross_position(FlexContext *ctx, float itemSize, float containerSize)
{
  switch (ctx->crossAlign)
  {
  case ALIGN_CENTER:
    return (containerSize - itemSize) / 2;
  case ALIGN_END:
    return containerSize - itemSize;
  case ALIGN_START:
  default:
    return 0;
  }
}

Rectangle FlexNext(FlexContext *ctx, Vector2 size)
{
  // Static tracking per context
  static FlexContext *last_ctx = NULL;
  static float current_pos = 0;
  static Vector2 *item_sizes = NULL;
  static int item_count = 0;
  static int max_items = 0;

  // Reset tracking when context changes
  if (last_ctx != ctx)
  {
    // Start position should include padding
    current_pos = ctx->padding;
    item_count = 0;

    // Free previous array
    if (item_sizes)
      free(item_sizes);

    max_items = ctx->expected_items;
    item_sizes = malloc(sizeof(Vector2) * max_items);
    last_ctx = ctx;
  }

  // Store current item size
  item_sizes[item_count] = size;

  float x = ctx->bounds.x;
  float y = ctx->bounds.y;

  if (ctx->mainAlign == ALIGN_SPACE_BETWEEN)
  {
    if (ctx->direction == FLEX_DIRECTION_ROW)
    {
      // Account for padding in available space calculation
      float available_space = ctx->bounds.width - (2 * ctx->padding);
      float total_content_width = size.x;
      for (int i = 0; i < item_count; i++)
      {
        total_content_width += item_sizes[i].x;
      }

      float remaining_space = available_space - total_content_width;
      float spacing = remaining_space / (ctx->expected_items - 1);

      // Start with padding
      x = ctx->bounds.x + ctx->padding;
      for (int i = 0; i < item_count; i++)
      {
        x += item_sizes[i].x + spacing;
      }

      // Apply cross alignment with padding
      y = ctx->bounds.y + ctx->padding +
          get_cross_position(ctx, size.y, ctx->bounds.height - 2 * ctx->padding);
    }
    else
    {
      // Similar logic for vertical layout
      float available_space = ctx->bounds.height - (2 * ctx->padding);
      float total_height = size.y;
      for (int i = 0; i < item_count; i++)
      {
        total_height += item_sizes[i].y;
      }

      float remaining_space = available_space - total_height;
      float spacing = remaining_space / (ctx->expected_items - 1);

      // Apply cross alignment with padding
      x = ctx->bounds.x + ctx->padding +
          get_cross_position(ctx, size.x, ctx->bounds.width - 2 * ctx->padding);

      // Start with padding and apply spacing
      y = ctx->bounds.y + ctx->padding;
      for (int i = 0; i < item_count; i++)
      {
        y += item_sizes[i].y + spacing;
      }
    }
  }
  else
  {
    // Normal positioning for other alignment modes
    if (ctx->direction == FLEX_DIRECTION_ROW)
    {
      // Apply current position (includes padding and gaps)
      x += current_pos;

      // Apply cross alignment with padding
      y += ctx->padding + get_cross_position(ctx, size.y,
                                             ctx->bounds.height - 2 * ctx->padding);

      // Update position including gap for next item
      current_pos += size.x + ctx->gap;
    }
    else
    {
      // Apply cross alignment with padding
      x += ctx->padding + get_cross_position(ctx, size.x,
                                             ctx->bounds.width - 2 * ctx->padding);

      // Apply current position (includes padding and gaps)
      y += current_pos;

      // Update position including gap for next item
      current_pos += size.y + ctx->gap;
    }
  }

  item_count++;
  return (Rectangle){x, y, size.x, size.y};
}

FlexContext FlexNested(FlexContext *parent, Vector2 size, FlexDirection direction)
{
  Rectangle next = FlexNext(parent, size);
  return FlexCreate(next, direction);
}