//
// Created by lukebayes on 4/23/21.
//

#include "phony_hid.h"
#include "phony_hid_test.h"
#include "minunit.h"
#include <string.h>

char *test_hid_in_report_to_struct(void) {
  PhonyHidInReport *in = calloc(sizeof(struct PhonyHidInReport), 1);
  muAssert(in != NULL, "Expected in report");

  phony_hid_in_report_to_struct(in, 0x01);
  muAssert(in->loop, "Expected loop");
  muAssert(in->ring == 0, "Expected no ring");
  muAssert(in->line_not_found == 0, "Expected no line_not_found");
  muAssert(in->line_in_use == 0, "Expected no line_in_use");
  muAssert(in->polarity == 0, "Expected no polarity");

  phony_hid_in_report_to_struct(in, 0x02);
  muAssert(in->loop == 0, "Expected no loop");
  muAssert(in->ring, "Expected ring");
  muAssert(in->line_not_found == 0, "Expected no line_not_found");
  muAssert(in->line_in_use == 0, "Expected no line_in_use");
  muAssert(in->polarity == 0, "Expected no polarity");

  phony_hid_in_report_to_struct(in, 0x03);
  muAssert(in->loop, "Expected loop");
  muAssert(in->ring, "Expected ring");
  muAssert(in->line_not_found == 0, "Expected no line_not_found");
  muAssert(in->line_in_use == 0, "Expected no line_in_use");
  muAssert(in->polarity == 0, "Expected no polarity");

  phony_hid_in_report_to_struct(in, 0x04);
  muAssert(in->loop == 0, "Expected no loop");
  muAssert(in->ring == 0, "Expected ring");
  muAssert(in->line_not_found == 1, "Expected no line_not_found");
  muAssert(in->line_in_use == 0, "Expected no line_in_use");
  muAssert(in->polarity == 0, "Expected no polarity");

  phony_hid_in_report_to_struct(in, 0x05);
  muAssert(in->loop == 1, "Expected no loop");
  muAssert(in->ring == 0, "Expected ring");
  muAssert(in->line_not_found == 0, "Expected no line_not_found");
  muAssert(in->line_in_use == 1, "Expected no line_in_use");
  muAssert(in->polarity == 0, "Expected no polarity");

  free(in);
  return NULL;
}

char *test_phony_hid_new(void) {
  PhonyHidContext *c = phony_hid_new();
  muAssert(c != NULL, "Expected Not Null");
  phony_hid_free(c);
  return NULL;
}

// NOTE(lbayes): This test only passes if a Maple board is connected to the USB
// bus. It's not clear to me (yet) how best to stub libusb services in C.
char *test_phony_hid_open(void) {
  PhonyHidContext *c = phony_hid_new();
  int status = phony_hid_open(c);
  muAssert(status >= 0, "Expected valid status from phony_hid_open");
  muAssert(c->is_open, "Expected is_open");

  phony_hid_free(c);
  return NULL;
}
