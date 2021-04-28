//
// Created by lukebayes on 4/20/21.
//

#ifndef MAPLE_APPLICATION_H
#define MAPLE_APPLICATION_H

#include "phony.h"
#include "phony_view.h"

#define APP_TITLE_LEN 256

typedef struct ApplicationContext {
  char title[APP_TITLE_LEN];
  void *native_app;
  PhonyContext *phony_context;
  PhonyViewContext *phony_view_context;
} ApplicationContext;

ApplicationContext *application_new(void);
int application_run(ApplicationContext *app, int argc, char *argv[]);
void application_free(ApplicationContext *c);

#endif //MAPLE_APPLICATION_H
