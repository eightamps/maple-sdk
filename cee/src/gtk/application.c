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

static void activate_callback(GtkApplication *native_app, gpointer userdata) {
  GtkWidget *window = gtk_application_window_new(native_app);
  application_context_t *app = (application_context_t *)userdata;
  gtk_window_set_title(GTK_WINDOW(window), app->title);
  gtk_window_set_default_size(GTK_WINDOW(window), APP_WIDTH, APP_HEIGHT);

  phony_view_context_t *p = phone_view_new(app->phony_context);
  app->phony_view_context = p;

  // Add the phone view
  gtk_container_add(GTK_CONTAINER(window), p->widget);
  gtk_widget_show_all(window);
}

application_context_t *application_new(void) {
  application_context_t *app = malloc(sizeof(application_context_t));
  if (app == NULL) {
    log_err("Could not allocate application_context_t struct");
    exit(1);
  }

  // Create the phony_context_t and related view component
  phony_context_t *phony = phony_new();
  if (phony == NULL) {
    log_err("Unable to instantiate phony service");
    exit(-ENOMEM);
  }
  app->phony_context = phony;

  int status = phony_open_maple(phony);
  if (status != EXIT_SUCCESS) {
    log_err("phony_open_maple failed");
  }

  GtkApplication *native_app = gtk_application_new("com.eightamps.term",
                                   G_APPLICATION_FLAGS_NONE);
  g_signal_connect(native_app, "activate",
                   G_CALLBACK(activate_callback), app);
  app->native_app = native_app;
  return app;
}

int application_run(application_context_t *app, int argc, char *argv[]) {
  GApplication *native_app = G_APPLICATION(app->native_app);
  return g_application_run(native_app, argc, argv);
}

void application_free(application_context_t *c) {
  if (c != NULL) {
    phony_view_free(c->phony_view_context);
    phony_free(c->phony_context);

    GApplication *native_app = G_APPLICATION(c->native_app);
    g_object_unref(native_app);
    free(c);
  }
}
