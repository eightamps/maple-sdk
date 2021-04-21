//
// Created by lukebayes on 4/21/21.
// Original source from @babacar here:
// https://stackoverflow.com/a/23446001/105023
//

#include "log.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>

static LogLevel log_level = LogLevelInfo;

static char *level_to_label(LogLevel level) {
  switch (level) {
    case LogLevelDebug:
      return "DEBUG";
    case LogLevelInfo:
      return "INFO";
    case LogLevelError:
      return "ERROR";
    case LogLevelNone:
      return "NONE";
    default:
      return "";
  }
}

static void log_format(LogLevel level, const char* message, va_list args) {
  if (level >= log_level) {
    FILE *out = (level == LogLevelError) ? stderr : stdout;

    time_t now;
    time(&now);
    char *date = ctime(&now);
    date[strlen(date) - 1] = '\0';
    fprintf(out, "%s [%s] ", date, level_to_label(level));
    vfprintf(out, message, args);
    fprintf(out, "\n");
  }
}

void set_log_level(LogLevel level) {
  log_level = level;
}

void log_error(const char* message, ...) {
  va_list args;
  va_start(args, message);
  log_format(LogLevelError, message, args);
  va_end(args);
}

void log_info(const char* message, ...) {
  va_list args;
  va_start(args, message);
  log_format(LogLevelInfo, message, args);
  va_end(args);
}

void log_debug(const char* message, ...) {
  va_list args;
  va_start(args, message);
  log_format(LogLevelDebug, message, args);
  va_end(args);
}
