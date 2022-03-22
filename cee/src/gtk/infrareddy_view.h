//
// Created by lukebayes on 4/21/21.
//

#ifndef MAPLE_INFRAREDDY_VIEW_H
#define MAPLE_INFRAREDDY_VIEW_H

#include <gtk/gtk.h>
#include "../infrareddy.h"

typedef struct {
  infrareddy_context_t *context;
  GtkWidget *widget;
  GtkEntry *codes_view;
  GtkTextView *message_view;
}infrareddy_view_context_t;

infrareddy_view_context_t *infrareddy_view_new(infrareddy_context_t *model);
void infrareddy_view_free(infrareddy_view_context_t *c);

#endif // MAPLE_INFRAREDDY_VIEW_H
