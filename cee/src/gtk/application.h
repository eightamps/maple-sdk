//
// Created by lukebayes on 4/20/21.
//

#ifndef MAPLE_APPLICATION_H
#define MAPLE_APPLICATION_H

#include "../phony.h"
#include "phony_view.h"

#define APP_TITLE_LEN 256

typedef struct {
  char title[APP_TITLE_LEN];
  void *native_app;
  phony_context_t *phony_context;
  phony_view_context_t *phony_view_context;
}application_context_t;

application_context_t *application_new(void);
int application_run(application_context_t *app, int argc, char *argv[]);
void application_free(application_context_t *c);

#endif //MAPLE_APPLICATION_H
