//
// Created by lukebayes on 4/20/21.
//
#include "gtk/application.h"
#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *APP_TITLE = "Demo App";

int main(int argc, char *argv[]) {
  log_info("Configuring ApplicationContext");
  ApplicationContext *app = application_new();
  memcpy(app->title, APP_TITLE, strlen(APP_TITLE) + 1);
  log_info("Running ApplicationContext");
  int status = application_run(app, argc, argv);

  application_free(app);

  if (status == 0) {
    log_info("Exiting ApplicationContext with status: 0 (SUCCESS)");
  } else {
    log_info("ERROR: ApplicationContext exiting with status: %d (FAILURE)", status);
  }
  return status;

  return EXIT_SUCCESS;
}
