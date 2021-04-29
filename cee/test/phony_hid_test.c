//
// Created by lukebayes on 4/23/21.
//

#include "phony_hid.h"
#include "phony_hid_test.h"
#include "minunit.h"
#include <string.h>


/*
static uint8_t struct_to_out_report_fake(PhonyHidOutReport *r) {
  printf("struct_to_out_report with:\n");
  printf("host_avail: %d\n", r->host_avail);
  printf("off_hook: %d\n", r->off_hook);
  uint8_t state = 0;

  if (r->off_hook) {
    r->host_avail = true; // We always become available if we're going off hook.
    state = state | (2<<0);
  } else {
    state &= ~(2 << 0);
  }

  if (r->host_avail) {
    state = state | (1<<0);
  } else {
    state &= ~(1 << 0);
  }
  printf("output: 0x%02x\n", state);
  return state;
}
*/

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
