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
static int in_report_to_struct(PhonyHidInReport *in_report, uint8_t value) {
  in_report->loop = (value >> 0) & 1;
  in_report->ring = (value >> 1) & 1;
  in_report->ring2 = (value >> 2) & 1;
  in_report->line_in_use = (value >> 3) & 1;
  in_report->polarity = (value >> 4) & 1;

  return EXIT_SUCCESS;
}

char *test_struct_transform(void) {
  PhonyHidInReport *in = calloc(sizeof(struct PhonyHidInReport), 1);
  muAssert(in != NULL, "Expected in report");

  in_report_to_struct(in, 0x01);
  muAssert(in->loop, "Expected loop");
  muAssert(in->ring == 0, "Expected no ring");
  muAssert(in->ring2 == 0, "Expected no ring2");
  muAssert(in->line_in_use == 0, "Expected no line_in_use");
  muAssert(in->polarity == 0, "Expected no polarity");

  in_report_to_struct(in, 0x02);
  muAssert(in->loop == 0, "Expected no loop");
  muAssert(in->ring, "Expected ring");
  muAssert(in->ring2 == 0, "Expected no ring2");
  muAssert(in->line_in_use == 0, "Expected no line_in_use");
  muAssert(in->polarity == 0, "Expected no polarity");

  in_report_to_struct(in, 0x03);
  muAssert(in->loop, "Expected loop");
  muAssert(in->ring, "Expected ring");
  muAssert(in->ring2 == 0, "Expected no ring2");
  muAssert(in->line_in_use == 0, "Expected no line_in_use");
  muAssert(in->polarity == 0, "Expected no polarity");


  /*
  PhonyHidOutReport *out = calloc(sizeof(struct PhonyHidOutReport), 1);
  uint8_t result;

  result = struct_to_out_report_fake(out);
  muAssert(result == 0, "Expected 0");

  out->host_avail = true;
  result = struct_to_out_report_fake(out);
  muAssert(result == 0x01, "Expected binary 1");

  out->host_avail = false;
  result = struct_to_out_report_fake(out);
  muAssert(result == 0, "Expected 0");

  out->off_hook = true;
  result = struct_to_out_report_fake(out);
  muAssert(result == 0x03, "Expected off hook and host avail");

  out->off_hook = false;
  result = struct_to_out_report_fake(out);
  muAssert(result == 0x01, "Expected host avail only");

  // muAssert(0, "SDF");
   */
  return NULL;
}

char *test_phony_hid_state(void) {
  const char *one = phony_hid_state_to_str(PHONY_NOT_READY);
  muAssert(strcmp("Not ready", one) == 0, "Expected Not ready");

  const char *two = phony_hid_state_to_str(PHONY_READY);
  muAssert(strcmp("Ready", two) == 0, "Expected Ready");

  const char *three = phony_hid_state_to_str(PHONY_OFF_HOOK);
  muAssert(strcmp("Off hook", three) == 0, "Expected Off hook");

  const char *four = phony_hid_state_to_str(PHONY_RINGING);
  muAssert(strcmp("Ringing", four) == 0, "Expected Ringing");

  const char *five = phony_hid_state_to_str(PHONY_LINE_NOT_FOUND);
  muAssert(strcmp("Line not found", five) == 0, "Expected Line not found");

  const char *six = phony_hid_state_to_str(PHONY_LINE_IN_USE);
  muAssert(strcmp("Line in use", six) == 0, "Expected Line in use");

  const char *seven = phony_hid_state_to_str(PHONY_HOST_NOT_FOUND);
  muAssert(strcmp("Host not found", seven) == 0, "Expected Host not found");
  return NULL;
}

char *test_phony_hid_new(void) {
  PhonyHidContext *c = phony_hid_new();
  muAssert(c != NULL, "Expected Not Null");
  phony_hid_free(c);
  return NULL;
}

// NOTE(lbayes): This test only passes if a Maple board is connect to the USB
// bus. It's not clear to me (yet) how best to stub libusb services in C.
char *test_phony_hid_open(void) {
  PhonyHidContext *c = phony_hid_new();
  int status = phony_hid_open(c);
  muAssert(status >= 0, "Expected valid status from phony_hid_open");
  muAssert(c->is_open, "Expected is_open");

  phony_hid_free(c);
  return NULL;
}
