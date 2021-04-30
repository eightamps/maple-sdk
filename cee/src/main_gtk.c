//
// Created by lukebayes on 4/20/21.
//
#include "gtk/application.h"
#include "log.h"
#include <string.h>

static const char *APP_TITLE = "Demo App";

int main(int argc, char *argv[]) {
  log_info("Configuring application_context_t");
  application_context_t *app = application_new();
  memcpy(app->title, APP_TITLE, strlen(APP_TITLE) + 1);
  log_info("Running application_context_t");
  int status = application_run(app, argc, argv);

  application_free(app);

  if (status == 0) {
    log_info("Exiting application_context_t with status: 0 (SUCCESS)");
  } else {
    log_info("ERROR: application_context_t exiting with status: %d (FAILURE)", status);
  }
  return status;
}
