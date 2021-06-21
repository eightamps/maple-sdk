//
// Created by lukebayes on 4/21/21.
//

#ifndef MAPLE_PHONY_VIEW_H
#define MAPLE_PHONY_VIEW_H

#include <gtk/gtk.h>
#include "../phony.h"

typedef struct {
  phony_context_t *phony;
  GtkWidget *widget;
  GtkEntry *phone_number_view;
  GtkTextView *message_view;
  GtkButton *dial_btn;
  GtkButton *hang_up_btn;
}phony_view_context_t;

phony_view_context_t *phone_view_new(phony_context_t *model);
void phony_view_free(phony_view_context_t *c);

#endif //MAPLE_PHONY_VIEW_H
