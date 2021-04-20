//
// Created by lukebayes on 4/20/21.
//
#include "gtk/application.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static const char *APP_TITLE = "Demo App";

int main(int argc, char *argv[]) {
  printf("Configuring Application\n");
  Application *app = application_new();
  memcpy(app->title, APP_TITLE, strlen(APP_TITLE) + 1);
  printf("Running Application\n");
  int status = application_run(app, argc, argv);
  application_free(app);
  if (status == 0) {
    printf("Exiting Application with status: 0 (SUCCESS)\n");
  } else {
    printf("ERROR: Application exiting with status: %d (FAILURE)\n", status);
  }
  return status;

  return EXIT_SUCCESS;
}
