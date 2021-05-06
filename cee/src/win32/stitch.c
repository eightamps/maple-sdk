//
// Created by lukebayes on 4/25/21.
//

#include "stitch.h"
#include <errno.h>

stitch_context_t *stitch_new_with_label(char *label) {
    return NULL;
}

stitch_context_t *stitch_new(void) {
    return NULL;
}

int stitch_init(stitch_context_t *c) {
    return EXIT_SUCCESS;
}

int stitch_set_dtmf(stitch_context_t *c, dtmf_context_t *d) {
    return EXIT_SUCCESS;
}

int stitch_get_default_input_index(stitch_context_t *c) {
    return EXIT_SUCCESS;
}

int stitch_get_default_output_index(stitch_context_t *c) {
    return EXIT_SUCCESS;
}

int stitch_get_matching_input_device_index(stitch_context_t *c, char *name) {
    return EXIT_SUCCESS;
}

int stitch_get_matching_output_device_index(stitch_context_t *c, char *name) {
    return EXIT_SUCCESS;
}

int stitch_start(stitch_context_t *c, int in_index, int out_index) {
    return EXIT_SUCCESS;
}

int stitch_stop(stitch_context_t *c) {
    return EXIT_SUCCESS;
}

int stitch_join(stitch_context_t *c) {
    return EXIT_SUCCESS;
}

void stitch_free(stitch_context_t *c) {
}
