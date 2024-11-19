#ifndef LAYOUT_H
#define LAYOUT_H

#include "raylib.h"

typedef enum
{
  FLEX_DIRECTION_ROW,
  FLEX_DIRECTION_COLUMN
} FlexDirection;

typedef enum
{
  ALIGN_START,
  ALIGN_CENTER,
  ALIGN_END,
  ALIGN_SPACE_BETWEEN
} FlexAlign;

typedef struct
{
  Rectangle bounds;
  FlexDirection direction;
  FlexAlign mainAlign;
  FlexAlign crossAlign;
  float gap;
  float padding;
  int expected_items;
} FlexContext;

// Initialize a new flex context
FlexContext FlexCreate(Rectangle bounds, FlexDirection direction);

// Set alignment properties
void FlexSetMainAlign(FlexContext *ctx, FlexAlign align);
void FlexSetCrossAlign(FlexContext *ctx, FlexAlign align);
void FlexSetGap(FlexContext *ctx, float gap);
void FlexSetPadding(FlexContext *ctx, float padding);
void FlexSetExpectedItems(FlexContext *ctx, int count);

// Get the next item rectangle based on size
Rectangle FlexNext(FlexContext *ctx, Vector2 size);

// Create a nested flex context
FlexContext FlexNested(FlexContext *parent, Vector2 size, FlexDirection direction);

#endif // LAYOUT_H