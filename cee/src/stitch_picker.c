//
// Created by lukebayes on 4/28/21.
//

#include "stitch_picker.h"
#include <string.h>
#include <ctype.h>

#define STITCH_ASI_TELEPHONE "ASI Telephone"
#define STITCH_ASI_MICROPHONE "ASI Microphone"
#define STITCH_WAY_2_CALL "Way2Call"
#define STITCH_WAY_2_CALL_LOWER "way2call"
#define STITCH_SPDIF "PDIF"

int stitch_picker_is_valid_host_device(char *name) {
  return (strstr(name, STITCH_ASI_TELEPHONE) == NULL) &&
    (strstr(name, STITCH_ASI_MICROPHONE) == NULL) &&
    (strstr(name, STITCH_SPDIF) == NULL) &&
    (strstr(name, STITCH_WAY_2_CALL) == NULL) &&
    (strstr(name, STITCH_WAY_2_CALL_LOWER) == NULL);
}
