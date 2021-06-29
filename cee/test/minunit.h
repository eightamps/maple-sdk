/**
 ******************************************************************************
 * @file           : debug.h
 * @brief          : Debug macros from, "Learn C The Hard Way"
 * @author         : Zed Shaw
 ******************************************************************************
 */
#ifndef _minunit_h
#define _minunit_h

/**
 * NOTE: This file was copied (and then modified) from this book:
 * https://www.oreilly.com/library/view/learn-c-the/9780133124385/
 */ 
#include "../src/log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define muSuiteStart() char *message = NULL

#define muAssert(test, message) if (!(test)) {\
  log_err_minunit(message); return message; }

/**
 * Perform a strcmp(a, b) == 0 check on the inputs and fail the test while
 * printing the received values if the comparison fails.
 */
#define muAssertStrCmp(a, b, message) if (strcmp(a, b) != 0) {\
  log_err_minunit("%s strcmp(\"%s\", \"%s\") == 0", message, a, b); return message; }

#define muAssertIntEq(a, b, message) if (a != b) {\
  log_err_minunit("%s (%d != %d)", message, a, b); return message; }

/**
 * Run the provided test method.
 */
#define muRunTest(test) message = test(); tests_run++; if (message) return message;

/**
 * Run all of the configured tests inside of a macro-generated main file.
 */
#define RUN_TESTS(name) int main(int argc, char *argv[]) {\
  printf("\n----\nRUNNING: %s\n", argv[0]);\
  char *result = name();\
  if (result != 0) {\
    printf("\033[1m\x1b[31mFAILED:\033[0m \x1b[31m%s\x1b[00m\n", result);\
  }\
  else {\
    printf("\033[1m\x1b[32mALL TESTS PASSED\x1b[00m\033[0m\n");\
  }\
  printf("Tests run: %d\n", tests_run);\
  exit(result != 0);\
}

static int tests_run;

#endif // _minunit_h
