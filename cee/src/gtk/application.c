//
// Created by lukebayes on 4/15/21.
//

#include "application.h"
#include "log.h"
#include "phony.h"
#include "phony_view.h"
#include <gtk/gtk.h>
#include <stddef.h>

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
    log_err("phony_hid_open failed, but is being ignored for retries");
  }

  PhonyViewContext *p = phone_view_new(pc);
  app->phony_view_context = p;

  // Add the phone view
  gtk_container_add(GTK_CONTAINER(window), p->widget);

  gtk_widget_show_all(window);
}

ApplicationContext *application_new(void) {
  ApplicationContext *app = malloc(sizeof(ApplicationContext));
  if (app == NULL) {
    log_err("Could not allocate ApplicationContext struct");
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

void application_free(ApplicationContext *c) {
  if (c != NULL) {

    phony_view_free(c->phony_view_context);
    phony_free(c->phony_context);

    GApplication *native_app = G_APPLICATION(c->native_app);
    g_object_unref(native_app);
    free(c);
  }
}
