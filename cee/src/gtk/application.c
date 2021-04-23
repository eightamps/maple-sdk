//
// Created by lukebayes on 4/15/21.
//

#include <gtk/gtk.h>
#include <stddef.h>
#include "application.h"
#include "phony.h"
#include "phone_view.h"

#define APP_WIDTH 300
#define APP_HEIGHT 400

static void activate(GtkApplication *native_app, gpointer user_data) {
  GtkWidget *window = gtk_application_window_new(native_app);
  Application *app = (Application *)user_data;
  gtk_window_set_title(GTK_WINDOW(window), app->title);
  gtk_window_set_default_size(GTK_WINDOW(window), APP_WIDTH, APP_HEIGHT);
  // configure_terminal(GTK_WINDOW(window));

  // Create the PhonyContext and related view component
  PhonyContext *pc = phony_new();
  PhoneViewContext *p = phone_view_new(pc);

  // Add the phone view
  gtk_container_add(GTK_CONTAINER(window), p->widget);

  gtk_widget_show_all(window);
}

Application *application_new(void) {
  Application *app = malloc(sizeof(Application));
  if (app == NULL) {
    printf("Could not allocate Application struct\n");
    exit(1);
  }

  GtkApplication *native_app;
  native_app = gtk_application_new("com.eightamps.term",
                                   G_APPLICATION_FLAGS_NONE);
  g_signal_connect(native_app, "activate", G_CALLBACK(activate), app);
  app->native_app = native_app;
  return app;
}

int application_run(Application *app, int argc, char *argv[]) {
  GApplication *native_app = G_APPLICATION(app->native_app);
  return g_application_run(native_app, argc, argv);
}

void application_free(Application *app) {
  GApplication *native_app = G_APPLICATION(app->native_app);
  g_object_unref(native_app);
  free(app);
}
