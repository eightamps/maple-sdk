/**
 ******************************************************************************
 * @file           : debug.h
 * @brief          : Debug macros from, "Learn C The Hard Way"
 * @author         : Zed Shaw
 *
 * Copyright (c) 2020 Eight Amps, LLC
 * All rights reserved.
 ******************************************************************************
 */

#ifndef MAPLE_LOG
#define MAPLE_LOG

#include <stdio.h>
#include <errno.h>
#include <string.h>

#ifdef TEST_MODE
#define debug(M, ...)
#define log_err(M, ...)
#define log_warn(M, ...)
#define log_info(M, ...)
#else
#define debug(M, ...) fprintf(stderr, "DEBUG %s:%d: " M "\n", \
    __FILE__, __LINE__, ##__VA_ARGS__)

#define log_err(M, ...) fprintf(stderr, \
    "[ERROR] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__, \
    clean_errno(), ##__VA_ARGS__)

#define log_warn(M, ...) fprintf(stdout, \
    "[WARN] (%s:%d: errno: %s) " M "\n", \
    __FILE__, __LINE__, clean_errno(), ##__VA_ARGS__)

#define log_info(M, ...) fprintf(stdout, "[INFO] (%s:%d) " M "\n", \
    __FILE__, __LINE__, ##__VA_ARGS__)
#endif

#define clean_errno() (errno == 0 ? "None" : strerror(errno))

#define log_fatal(M, ...) fprintf(stderr, \
    "[FATAL] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__, \
    clean_errno(), ##__VA_ARGS__)

#define log_err_minunit(M, ...) fprintf(stderr, \
    "[ERROR] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__, \
    clean_errno(), ##__VA_ARGS__)

#define check(A, M, ...) if (!(A)) {\
  log_err(M, ##__VA_ARGS__); errno = 0; goto error; }

#define sentinel(M, ...) { log_err(M, ##__VA_ARGS__);\
  errno = 0; goto error; }

#define check_mem(A) check((A), "Out of memory.")

#define check_debug(A, M, ...) if (!(A)) { debug(M, ##__VA_ARGS__);\
  errno = 0; goto error; }

#endif // MAPLE_LOG
