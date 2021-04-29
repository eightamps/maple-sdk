//
// Created by lukebayes on 4/21/21.
//

#ifndef MAPLE_PHONY_VIEW_H
#define MAPLE_PHONY_VIEW_H

#include <gtk/gtk.h>
#include "phony.h"

typedef struct PhonyViewContext {
  PhonyContext *phony;
  GtkWidget *widget;
  GtkEntry *phone_number_view;
  GtkTextView *message_view;
  GtkButton *dial_btn;
  GtkButton *hang_up_btn;
}PhonyViewContext;

struct PhonyViewContext *phone_view_new(PhonyContext *model);
void phony_view_free(PhonyViewContext *c);

#endif //MAPLE_PHONY_VIEW_H
