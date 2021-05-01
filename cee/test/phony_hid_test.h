//
// Created by lukebayes on 4/23/21.
//

#ifndef MAPLE_PHONY_HID_TEST_H
#define MAPLE_PHONY_HID_TEST_H

#define LUSB_STATUS_MSG(s, expected) \
  muAssert(0 == chars_match( \
  phony_hid_status_message(phony_hid_status_from_libusb(s)), \
  expected), "Failed");

char *test_phony_hid_new(void);
char *test_phony_hid_open_not_found(void);
char *test_hid_in_report_to_struct(void);
char *test_phony_hid_libusb_error_codes(void);


#endif // MAPLE_PHONY_HID_TEST_H
