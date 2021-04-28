//
// Created by lukebayes on 4/28/21.
//

#include "stitch_picker_test.h"
#include "stitch_picker.h"
#include "minunit.h"

char *test_stitch_picker_easy_host(void) {
  bool result;

  result = stitch_picker_is_valid_host_device("Analog Output - MM1");
  muAssert(result, "Expected valid");

  result = stitch_picker_is_valid_host_device("ASI Telephone");
  muAssert(result == false, "Expected failure");

  result = stitch_picker_is_valid_host_device("ASI Microphone");
  muAssert(result == false, "Expected failure");

  result = stitch_picker_is_valid_host_device("Way2Call");
  muAssert(result == false, "Expected failure");

  result = stitch_picker_is_valid_host_device("way2call");
  muAssert(result == false, "Expected failure");
  return NULL;
}
