//
// Created by lukebayes on 4/15/21.
//

#include <gtk/gtk.h>
#include <stddef.h>
#include "application.h"
#include "phony.h"
#include "phony_view.h"

#define APP_WIDTH 300
#define APP_HEIGHT 400

static void activate_callback(GtkApplication *native_app, gpointer user_data) {
  GtkWidget *window = gtk_application_window_new(native_app);
  ApplicationContext *app = (ApplicationContext *)user_data;
  gtk_window_set_title(GTK_WINDOW(window), app->title);
  gtk_window_set_default_size(GTK_WINDOW(window), APP_WIDTH, APP_HEIGHT);

  // Create the PhonyContext and related view component
  PhonyContext *pc = phony_new();
  app->phony_context = pc;

  int status = phony_open_maple(pc);
  if (status != EXIT_SUCCESS) {
    fprintf(stderr, "phony_hid_open failed, but is being ignored for "
                    "retries\n");
  }

  PhoneViewContext *p = phone_view_new(pc);

  // Add the phone view
  gtk_container_add(GTK_CONTAINER(window), p->widget);

  gtk_widget_show_all(window);
}

ApplicationContext *application_new(void) {
  ApplicationContext *app = malloc(sizeof(ApplicationContext));
  if (app == NULL) {
    printf("Could not allocate ApplicationContext struct\n");
    exit(1);
  }

  GtkApplication *native_app = gtk_application_new("com.eightamps.term",
                                   G_APPLICATION_FLAGS_NONE);
  g_signal_connect(native_app, "activate",
                   G_CALLBACK(activate_callback), app);
  app->native_app = native_app;
  return app;
}

int application_run(ApplicationContext *app, int argc, char *argv[]) {
  GApplication *native_app = G_APPLICATION(app->native_app);
  return g_application_run(native_app, argc, argv);
}

void application_free(ApplicationContext *app) {
  if (app != NULL) {
    if (app->phony_context != NULL) {
      phony_free(app->phony_context);
    }

    GApplication *native_app = G_APPLICATION(app->native_app);
    g_object_unref(native_app);
    free(app);
  }
}
