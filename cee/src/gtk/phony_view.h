//
// Created by lukebayes on 4/21/21.
//

#ifndef MAPLE_PHONY_VIEW_H
#define MAPLE_PHONY_VIEW_H

#include <gtk/gtk.h>
#include "phony.h"

typedef struct PhoneViewContext {
  PhonyContext *phony;
  GtkWidget *widget;
  GtkEntry *phone_number_view;
  GtkTextView *message_view;
  GtkButton *dial_btn;
  GtkButton *hang_up_btn;
}PhoneViewContext;

struct PhoneViewContext *phone_view_new(PhonyContext *model);
void phone_view_free(PhoneViewContext *c);

#endif //MAPLE_PHONY_VIEW_H
