#ifndef FLEXBOX_H
#define FLEXBOX_H

#include "raylib.h"

// Layout direction (horizontal or vertical)
typedef enum fbox_direction
{
  fbox_DIRECTION_ROW,   // Items laid out in a row (left to right)
  fbox_DIRECTION_COLUMN // Items laid out in a column (top to bottom)
} fbox_direction_t;

// Alignment options for main and cross axes
typedef enum fbox_align
{
  fbox_ALIGN_START,         // Align items at start of container
  fbox_ALIGN_CENTER,        // Center items in container
  fbox_ALIGN_END,           // Align items at end of container
  fbox_ALIGN_SPACE_BETWEEN, // Evenly space items with no space at ends
  fbox_ALIGN_SPACE_AROUND   // Evenly space items with half-space at ends
} fbox_align_t;

// Size modes for items
typedef enum fbox_size_mode
{
  fbox_SIZE_FIXED,  // Use exact size provided
  fbox_SIZE_STRETCH // Stretch to fill container (respecting padding)
} fbox_size_mode_t;

// Main flexbox context structure
typedef struct fbox_context
{
  Rectangle bounds;           // Container bounds
  fbox_direction_t direction; // Layout direction
  fbox_align_t main_align;    // Alignment along main axis
  fbox_align_t cross_align;   // Alignment along cross axis
  float gap;                  // Space between items
  float padding;              // Space around container edges
  int expected_items;         // Number of items to layout
  Vector2 *item_sizes;        // Array of item sizes
  int item_count;             // Current number of items processed
  int max_items;              // Maximum items that can be stored
  float current_pos;          // Current position along main axis
  float content_height;       // Total height of content
  float content_width;        // Total width of content
  fbox_size_mode_t size_mode; // How items should be sized
  scrollable_t *scrollable;   // Add this field
} fbox_context_t;

// Creation and destruction
fbox_context_t fbox_create(Rectangle bounds, fbox_direction_t direction, scrollable_t *scrollable);
fbox_context_t fbox_create_nested(fbox_context_t *parent, Rectangle bounds);
void fbox_destroy(fbox_context_t *ctx);

// Configuration functions
void fbox_set_direction(fbox_context_t *ctx, fbox_direction_t direction);
void fbox_set_main_align(fbox_context_t *ctx, fbox_align_t align);
void fbox_set_cross_align(fbox_context_t *ctx, fbox_align_t align);
void fbox_set_gap(fbox_context_t *ctx, float gap);
void fbox_set_padding(fbox_context_t *ctx, float padding);
void fbox_set_expected_items(fbox_context_t *ctx, int count);
void fbox_set_size_mode(fbox_context_t *ctx, fbox_size_mode_t mode);

// Layout information
float fbox_get_content_height(fbox_context_t *ctx);
float fbox_get_content_width(fbox_context_t *ctx);

// Layout functions
Rectangle fbox_next(fbox_context_t *ctx, Vector2 size);
Vector2 fbox_next_position(fbox_context_t *ctx);

#endif // FLEXBOX_H