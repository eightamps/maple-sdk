//
// Created by lukebayes on 4/21/21.
//

#include "phony_view.h"
#include "phony.h"
#include "log.h"
#include <stdlib.h>

#define DEFAULT_8A_PHONE_NUMBER "7273392258"

static void update(PhoneViewContext *c) {
  gtk_widget_queue_draw(GTK_WIDGET(c->phone_number_view));
}

static void num_clicked(GtkWidget *widget, gpointer data) {
  GtkButton *btn = GTK_BUTTON(widget);
  PhoneViewContext *c = data;
  GtkEntry *entry = c->phone_number_view;

  const gchar *label = gtk_button_get_label(btn);
  GtkEntryBuffer *b = gtk_entry_get_buffer(entry);
  size_t b_size = gtk_entry_buffer_get_length(b);
  gtk_entry_buffer_insert_text(b, b_size, label, 1);

  update(c);
}

static void show_status(PhoneViewContext *c, int status) {
  GtkTextView *m = c->message_view;
  GtkTextBuffer *b = gtk_text_view_get_buffer(m);

  if (status == EXIT_SUCCESS) {
    gtk_text_buffer_set_text(b, "\n", 1);
  } else {
    char *content = malloc(64);
    sprintf(content, "status: %d", status);
    size_t len = strlen(content);
    gtk_text_buffer_set_text(b, content, len);
    free(content);
  }
}

static void dial_clicked(__attribute__((unused)) GtkWidget *widget,
                         gpointer data) {
  PhoneViewContext *c = data;
  GtkEntryBuffer *b = gtk_entry_get_buffer(c->phone_number_view);
  const char *text = gtk_entry_buffer_get_text(b);
  printf("dial_clicked with: %s\n", text);

  show_status(c, phony_dial(c->phony, text));
}

static void hangup_clicked(__attribute__((unused)) GtkWidget *widget,
                           gpointer data) {
  PhoneViewContext *c = data;
  int status = phony_hang_up(c->phony);
  show_status(c, status);
}

static void phony_state_changed_handler(void *varg) {
  PhoneViewContext *c = varg;
  GtkWidget *dial_btn = c->dial_btn;
  GtkWidget *hang_up_btn = c->hang_up_btn;

  PhonyContext *pc = c->phony;
  switch (pc->state) {
  case PHONY_LINE_IN_USE:
    gtk_widget_set_visible(dial_btn, FALSE);
    gtk_widget_set_visible(hang_up_btn, TRUE);
    break;
  case PHONY_READY:
    gtk_widget_set_visible(dial_btn, TRUE);
    gtk_widget_set_visible(hang_up_btn, FALSE);
    break;
  case PHONY_NOT_READY:
    break;
  }
}

struct PhoneViewContext *phone_view_new(PhonyContext *model) {
  PhoneViewContext *c = calloc(sizeof(PhoneViewContext), 1);
  if (c == NULL) {
    log_err("Failed to allocate PhoneViewContext");
    return NULL;
  }

  phony_set_state_changed(model, phony_state_changed_handler, c);

  c->phony = model;

  int padding = 2;

  GtkBox *box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0));
  GtkBox *row_1 = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
  GtkBox *row_2 = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
  GtkBox *row_3 = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
  GtkBox *row_4 = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
  GtkBox *row_5 = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
  GtkBox *row_6 = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));

  gtk_box_pack_start(box, GTK_WIDGET(row_1), gtk_true(), gtk_true(), padding);
  gtk_box_pack_start(box, GTK_WIDGET(row_2), gtk_true(), gtk_true(), padding);
  gtk_box_pack_start(box, GTK_WIDGET(row_3), gtk_true(), gtk_true(), padding);
  gtk_box_pack_start(box, GTK_WIDGET(row_4), gtk_true(), gtk_true(), padding);
  gtk_box_pack_start(box, GTK_WIDGET(row_5), gtk_true(), gtk_true(), padding);
  gtk_box_pack_start(box, GTK_WIDGET(row_6), gtk_true(), gtk_true(), padding);

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
  GtkButton *hang_up_btn = GTK_BUTTON(gtk_button_new_with_label("Hang Up"));

  gtk_box_pack_start(row_5, GTK_WIDGET(entry), gtk_true(), gtk_true(), padding);
  gtk_box_pack_start(row_5, GTK_WIDGET(dial_btn), gtk_true(), gtk_true(),
                                      padding);
  gtk_box_pack_start(row_5, GTK_WIDGET(hang_up_btn), gtk_true(), gtk_true(),
                     padding);

  GtkTextView *message_view = GTK_TEXT_VIEW(gtk_text_view_new());
  gtk_text_view_set_editable(message_view, false);
  gtk_text_view_set_monospace(message_view, true);
  gtk_box_pack_start(row_6, GTK_WIDGET(message_view), gtk_true(), gtk_true(),
                     padding);
  c->message_view = message_view;
  c->dial_btn = dial_btn;
  c->hang_up_btn = hang_up_btn;

  g_signal_connect(btn_1, "clicked", G_CALLBACK(num_clicked), c);
  g_signal_connect(btn_2, "clicked", G_CALLBACK(num_clicked), c);
  g_signal_connect(btn_3, "clicked", G_CALLBACK(num_clicked), c);
  g_signal_connect(btn_4, "clicked", G_CALLBACK(num_clicked), c);
  g_signal_connect(btn_5, "clicked", G_CALLBACK(num_clicked), c);
  g_signal_connect(btn_6, "clicked", G_CALLBACK(num_clicked), c);
  g_signal_connect(btn_7, "clicked", G_CALLBACK(num_clicked), c);
  g_signal_connect(btn_8, "clicked", G_CALLBACK(num_clicked), c);
  g_signal_connect(btn_9, "clicked", G_CALLBACK(num_clicked), c);
  g_signal_connect(btn_0, "clicked", G_CALLBACK(num_clicked), c);
  g_signal_connect(btn_star, "clicked", G_CALLBACK(num_clicked), c);
  g_signal_connect(btn_hash, "clicked", G_CALLBACK(num_clicked), c);

  g_signal_connect(dial_btn, "clicked", G_CALLBACK(dial_clicked), c);
  g_signal_connect(hang_up_btn, "clicked", G_CALLBACK(hangup_clicked), c);
  c->phone_number_view = entry;

  gtk_entry_set_text(entry, DEFAULT_8A_PHONE_NUMBER);

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
