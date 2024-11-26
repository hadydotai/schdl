#ifndef PARSER_H
#define PARSER_H

#include "data.h"

// Error codes for parsing
typedef enum parse_error
{
  PARSE_SUCCESS = 0,
  PARSE_ERROR_FILE_NOT_FOUND,
  PARSE_ERROR_INVALID_TIME_FORMAT,
  PARSE_ERROR_INVALID_LINE_FORMAT,
  PARSE_ERROR_MEMORY
} parse_error_t;

// Parse a schedule file and return a new schedule
// Returns NULL if parsing fails
schedule_t *parse_schedule_file(const char *filename, parse_error_t *error);

// Convert a parse error to a string
const char *parse_error_to_string(parse_error_t error);

// Helper function to parse a time string (exposed for testing)
time_t parse_time(const char *time_str, parse_error_t *error);

#endif // PARSER_H