//
// Created by lukebayes on 4/21/21.
//

#include "../log.h"
#include "../infrareddy.h"
#include "infrareddy_view.h"
#include <stdlib.h>
#include <string.h>

#define ENCODE_ONE "0000 006D 0022 0002 0157 00AB 0015 0016 0015 0016 0015 0041 \
     0015 0016 0015 0016 0015 0016 0015 0016 0015 0016 0015 0041 0015 0041 0015 \
     0016 0015 0041 0015 0041 0015 0041 0015 0041 0015 0041 0015 0041 0015 0016 \
     0015 0016 0015 0016 0015 0041 0015 0016 0015 0016 0015 0016 0015 0016 0015 \
     0041 0015 0041 0015 0041 0015 0016 0015 0041 0015 0041 0015 0041 0015 05F4 \
     0157 0056 0015 0E45"

#define ENCODE_TWO "0000 006D 0022 0002 0157 00AB 0015 0016 0015 0016 0015 0041 \
     0015 0016 0015 0016 0015 0016 0015 0016 0015 0016 0015 0041 0015 0041 0015 \
     0016 0015 0041 0015 0041 0015 0041 0015 0041 0015 0041 0015 0016 0015 0041 \
     0015 0016 0015 0016 0015 0041 0015 0016 0015 0016 0015 0016 0015 0041 0015 \
     0016 0015 0041 0015 0041 0015 0016 0015 0041 0015 0041 0015 0041 0015 05F5 \
     0157 0056 0015 0E46"

/*
static void update(infrareddy_view_context_t *c) {
  gtk_widget_queue_draw(GTK_WIDGET(c->codes_view));
}
*/

static void btn_clicked(GtkWidget *widget, gpointer data) {
  int max_len = 256;
  GtkButton *btn = GTK_BUTTON(widget);
  const gchar *label = gtk_button_get_label(btn);
  int status = EXIT_SUCCESS;

  infrareddy_view_context_t *vc = data;
  infrareddy_context_t *c = vc->context;

  log_info("btn_clicked with label: %s", label);

  if (strncmp(label, "Encode 1", max_len) == 0) {
    log_info(">>>>>>>>> ENCODE 1");
    int len = strlen(ENCODE_ONE);
    status = infrareddy_encode(c, len, (unsigned char *)ENCODE_ONE);
    if (status != EXIT_SUCCESS) {
      log_err("ERROR: failed to encode: %d", status);
    }
  } else if (strncmp(label, "Encode 2", max_len) == 0) {
    log_info(">>>>>>>>> ENCODE 2");
  } else if (strncmp(label, "Decode", max_len) == 0) {
    log_info(">>>>>>>>> DECODE");
  }
   
  // GtkEntryBuffer *b = gtk_entry_get_buffer(entry);
  // size_t b_size = gtk_entry_buffer_get_length(b);
  // gtk_entry_buffer_insert_text(b, b_size, label, 1);

  // update(c);
}

/*
static void show_status(infrareddy_view_context_t *c, int status) {
  GtkTextView *m = c->message_view;
  GtkTextBuffer *b = gtk_text_view_get_buffer(m);

  if (status == EXIT_SUCCESS) {
    gtk_text_buffer_set_text(b, "\n", 1);
  } else {
    char *content = malloc(64);
    sprintf(content, "status: %d", status);
    size_t len = strlen(content);
    gtk_text_buffer_set_text(b, content, (gint)len);
    free(content);
  }
}

static void dial_clicked(__attribute__((unused)) GtkWidget *widget,
                         gpointer data) {
  infrareddy_view_context_t *c = data;
  GtkEntryBuffer *b = gtk_entry_get_buffer(c->phone_number_view);
  const char *text = gtk_entry_buffer_get_text(b);
  log_info("dial_clicked with: %s", text);

  show_status(c, infrareddy_dial(c->context, text));
}

static void hangup_clicked(__attribute__((unused)) GtkWidget *widget,
                           gpointer data) {
  infrareddy_view_context_t *c = data;
  int status = infrareddy_hang_up(c->context);
  show_status(c, status);
}

static void del_clicked(GtkWidget *widget, gpointer data) {
  log_info("del clicked");
  infrareddy_view_context_t *c = data;
  GtkEntryBuffer *b = gtk_entry_get_buffer(c->phone_number_view);
  const char *text = gtk_entry_buffer_get_text(b);
  size_t len = strlen(text);
  if (len > 0) {
    len = len - 1;
    char *new_text = malloc(len);
    strncpy(new_text, text, len);
    gtk_entry_buffer_set_text(b, new_text, len);
  }
}

static void update_button(GtkWidget *btn, bool show) {
  gtk_widget_set_visible(btn, show);
  gtk_widget_set_sensitive(btn, show);
}

static void update_buttons(infrareddy_view_context_t *c, bool show_dial) {
  // GtkWidget *dial_btn = GTK_WIDGET(c->dial_btn);
  // GtkWidget *hang_up_btn = GTK_WIDGET(c->hang_up_btn);

  // update_button(dial_btn, show_dial);
  // update_button(hang_up_btn, !show_dial);
}

static void disable_buttons(infrareddy_view_context_t *c) {
  log_info("disable buttons");
  // GtkWidget *dial_btn = GTK_WIDGET(c->dial_btn);
  // GtkWidget *hang_up_btn = GTK_WIDGET(c->hang_up_btn);

  // gtk_widget_set_sensitive(dial_btn, FALSE);
  // gtk_widget_set_sensitive(hang_up_btn, FALSE);
}

static void box_show_handler(GtkWidget *widget, gpointer data) {
  infrareddy_view_context_t *c = data;
  // update_buttons(c, PV_SHOW_DIAL);
  infrareddy_state state = infrareddy_get_state(c->context);
  // We did not get a USB connection, disable the dial button
  log_info("infrareddy_view box_show_handler found: %d\n", state);
  // if (state == infrareddy_DEVICE_NOT_FOUND) {
    // gtk_widget_set_sensitive(GTK_WIDGET(c->dial_btn), FALSE);
  // }
}
*/

static int infrareddy_state_changed_idle_handler(void *varg) {
  /*
  infrareddy_view_context_t *c = varg;
  infrareddy_context_t *pc = c->context;
  // Only do work on state transitions.
  switch (pc->state) {
  case infrareddy_LINE_NOT_FOUND:
  case infrareddy_DEVICE_NOT_FOUND:
    log_err("LINE OR DEVICE NOT FOUND");
    update_buttons(c, PV_SHOW_DIAL);
    disable_buttons(c);
    break;
  case infrareddy_RINGING:
    log_info("ring ring, ring ring");
    break;
  case infrareddy_READY:
    update_buttons(c, PV_SHOW_DIAL);
    break;
  case infrareddy_OFF_HOOK:
  case infrareddy_LINE_IN_USE:
    update_buttons(c, PV_SHOW_HANG_UP);
    break;
  case infrareddy_NOT_READY:
  case infrareddy_CONNECTED:
  case infrareddy_EXITING:
    break;
  }

  g_idle_remove_by_data(varg);
  */
  return EXIT_SUCCESS;
}

static void infrareddy_state_changed_handler(void *varg) {
  log_info("infrareddy_view - infrareddy_state_changed_handler");
  g_idle_add(&infrareddy_state_changed_idle_handler, varg);
}

infrareddy_view_context_t *infrareddy_view_new(infrareddy_context_t *model) {
  infrareddy_view_context_t *c = calloc(sizeof(infrareddy_view_context_t), 1);
  if (c == NULL) {
    log_err("Failed to allocate infrareddy_view_context_t");
    return NULL;
  }

  int padding = 2;

  infrareddy_on_state_changed(model, infrareddy_state_changed_handler, c);
  c->context = model;

  // Create outer box and rows
  GtkBox *box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0));
  GtkBox *row_1 = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
  GtkBox *row_2 = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));

  // Pack rows into box
  gtk_box_pack_start(box, GTK_WIDGET(row_1), gtk_true(), gtk_true(), padding);
  gtk_box_pack_start(box, GTK_WIDGET(row_2), gtk_true(), gtk_true(), padding);


  // Create views
  GtkWidget *btn_1 = gtk_button_new_with_label("Encode 1");
  GtkWidget *btn_2 = gtk_button_new_with_label("Encode 2");
  GtkWidget *btn_3 = gtk_button_new_with_label("Decode");

  GtkTextView *message_view = GTK_TEXT_VIEW(gtk_text_view_new());
  gtk_text_view_set_editable(message_view, false);
  gtk_text_view_set_monospace(message_view, true);

  // Pack views into rows
  gtk_box_pack_start(row_1, GTK_WIDGET(btn_1), gtk_true(), gtk_true(),
                     padding);
  gtk_box_pack_start(row_1, GTK_WIDGET(btn_2), gtk_true(), gtk_true(),
                     padding);
  gtk_box_pack_start(row_1, GTK_WIDGET(btn_3), gtk_true(), gtk_true(),
                     padding);
  gtk_box_pack_start(row_2, GTK_WIDGET(message_view), gtk_true(), gtk_true(),
                     padding);

  c->message_view = message_view;
  c->widget = GTK_WIDGET(box);

  g_signal_connect(btn_1, "clicked", G_CALLBACK(btn_clicked), c);
  g_signal_connect(btn_2, "clicked", G_CALLBACK(btn_clicked), c);
  g_signal_connect(btn_3, "clicked", G_CALLBACK(btn_clicked), c);

  /*

  GtkBox *box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0));
  GtkBox *row_1 = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
  GtkBox *row_2 = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
  GtkBox *row_3 = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
  GtkBox *row_4 = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
  GtkBox *row_5 = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
  GtkBox *row_6 = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
  GtkBox *row_7 = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));

  gtk_box_pack_start(box, GTK_WIDGET(row_1), gtk_true(), gtk_true(), padding);
  gtk_box_pack_start(box, GTK_WIDGET(row_2), gtk_true(), gtk_true(), padding);
  gtk_box_pack_start(box, GTK_WIDGET(row_3), gtk_true(), gtk_true(), padding);
  gtk_box_pack_start(box, GTK_WIDGET(row_4), gtk_true(), gtk_true(), padding);
  gtk_box_pack_start(box, GTK_WIDGET(row_5), gtk_true(), gtk_true(), padding);
  gtk_box_pack_start(box, GTK_WIDGET(row_6), gtk_true(), gtk_true(), padding);
  gtk_box_pack_start(box, GTK_WIDGET(row_7), gtk_true(), gtk_true(), padding);

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

  GtkButton *del_btn = GTK_BUTTON(gtk_button_new_with_label("Backspace"));
  gtk_box_pack_start(row_6, GTK_WIDGET(del_btn), gtk_true(), gtk_true(),
                     padding);

  GtkTextView *message_view = GTK_TEXT_VIEW(gtk_text_view_new());
  gtk_text_view_set_editable(message_view, false);
  gtk_text_view_set_monospace(message_view, true);
  gtk_box_pack_start(row_7, GTK_WIDGET(message_view), gtk_true(), gtk_true(),
                     padding);
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
  g_signal_connect(del_btn, "clicked", G_CALLBACK(del_clicked), c);

  // Connect to the "show" signal, which will trigger when the application
  // is first displayed
  g_signal_connect(box, "show", G_CALLBACK(box_show_handler), c);

  // Set the default phone number
  gtk_entry_set_text(entry, DEFAULT_8A_PHONE_NUMBER);

  // Apply pointers to the context object
  c->message_view = message_view;
  c->dial_btn = dial_btn;
  c->hang_up_btn = hang_up_btn;
  c->phone_number_view = entry;
  c->widget = GTK_WIDGET(box);

  */

  return c;
}

void infrareddy_view_free(infrareddy_view_context_t *c) {
  if (c != NULL) {
    free(c);
  }
}

