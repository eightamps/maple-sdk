//
// Created by lukebayes on 4/21/21.
//

#include "phone_view.h"
#include "log.h"
#include <stdlib.h>

struct PhoneViewContext *phone_view_new(void) {
  size_t size = sizeof(PhoneViewContext);
  PhoneViewContext *c = malloc(size);
  if (c == NULL) {
    log_err("Failed to allocate PhoneViewContext");
    return NULL;
  }
  memset(c, 0x0, size);

  int padding = 2;

  GtkBox *box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0));
  GtkBox *row_1 = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
  GtkBox *row_2 = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
  GtkBox *row_3 = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
  GtkBox *row_4 = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
  GtkBox *row_5 = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));

  gtk_box_pack_start(box, GTK_WIDGET(row_1), gtk_true(), gtk_true(), padding);
  gtk_box_pack_start(box, GTK_WIDGET(row_2), gtk_true(), gtk_true(), padding);
  gtk_box_pack_start(box, GTK_WIDGET(row_3), gtk_true(), gtk_true(), padding);
  gtk_box_pack_start(box, GTK_WIDGET(row_4), gtk_true(), gtk_true(), padding);
  gtk_box_pack_start(box, GTK_WIDGET(row_5), gtk_true(), gtk_true(), padding);

  GtkWidget *btn_1 = gtk_button_new_with_label("1");
  GtkWidget *btn_2 = gtk_button_new_with_label("2");
  GtkWidget *btn_3 = gtk_button_new_with_label("3");

  gtk_box_pack_start(row_1, btn_1, gtk_true(), gtk_true(), padding);
  gtk_box_pack_start(row_1, btn_2, gtk_true(), gtk_true(), padding);
  gtk_box_pack_start(row_1, btn_3, gtk_true(), gtk_true(), padding);

  GtkWidget *btn_4 = gtk_button_new_with_label("4");
  GtkWidget *btn_5 = gtk_button_new_with_label("5");
  GtkWidget *btn_6 = gtk_button_new_with_label("6");

  gtk_box_pack_start(row_2, btn_4, gtk_true(), gtk_true(), padding);
  gtk_box_pack_start(row_2, btn_5, gtk_true(), gtk_true(), padding);
  gtk_box_pack_start(row_2, btn_6, gtk_true(), gtk_true(), padding);

  GtkWidget *btn_7 = gtk_button_new_with_label("7");
  GtkWidget *btn_8 = gtk_button_new_with_label("8");
  GtkWidget *btn_9 = gtk_button_new_with_label("9");

  gtk_box_pack_start(row_3, btn_7, gtk_true(), gtk_true(), padding);
  gtk_box_pack_start(row_3, btn_8, gtk_true(), gtk_true(), padding);
  gtk_box_pack_start(row_3, btn_9, gtk_true(), gtk_true(), padding);

  GtkWidget *btn_hash = gtk_button_new_with_label("#");
  GtkWidget *btn_0 = gtk_button_new_with_label("0");
  GtkWidget *btn_star = gtk_button_new_with_label("*");

  gtk_box_pack_start(row_4, btn_hash, gtk_true(), gtk_true(), padding);
  gtk_box_pack_start(row_4, btn_0, gtk_true(), gtk_true(), padding);
  gtk_box_pack_start(row_4, btn_star, gtk_true(), gtk_true(), padding);

  GtkEntry *entry = GTK_ENTRY(gtk_entry_new());
  GtkButton *dial_btn = GTK_BUTTON(gtk_button_new_with_label("Dial"));

  gtk_box_pack_start(row_5, entry, gtk_true(), gtk_true(), padding);
  gtk_box_pack_start(row_5, dial_btn, gtk_true(), gtk_true(), padding);

  /*
  gtk_grid_attach(grid, dial_btn, 0, 4, 1, 1);
  gtk_grid_attach(grid, entry,    1, 4, 2, 1);
  c->widget = GTK_WIDGET(grid);
   */

  c->widget = GTK_WIDGET(box);
  return c;
}

void phone_view_free(PhoneViewContext *c) {
  if (c != NULL) {
    if (c->widget != NULL) {
      gtk_widget_destroy(c->widget);
    }
    free(c);
  }
}
