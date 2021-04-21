//
// Created by lukebayes on 4/21/21.
// Original source from @babacar here:
// https://stackoverflow.com/a/23446001/105023
//

#ifndef MAPLE_LOG_H
#define MAPLE_LOG_H

typedef enum LogLevel {
  LogLevelInfo = 0,
  LogLevelDebug,
  LogLevelError,
  LogLevelNone,
}LogLevel;

void set_log_level(LogLevel level);
void log_error(const char* message, ...);
void log_info(const char* message, ...);
void log_debug(const char* message, ...);

#endif //MAPLE_LOG_H
