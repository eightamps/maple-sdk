#ifndef MAPLE_STITCH_TEST_H
#define MAPLE_STITCH_TEST_H

char *test_stitch_new(void);
char *test_stitch_init_null(void);
char *test_stitch_init(void);
char *test_stitch_init_sio_create_failed(void);
char *test_stitch_init_sio_connect_failed(void);
char *test_stitch_connect(void);
char *test_stitch_init_custom_backend(void);
char *test_get_default_input(void);
char *test_get_default_valid_input(void);
char *test_get_default_input_allows_asi_mic(void);
char *test_get_default_output_filters_asi_mic(void);

#endif // MAPLE_STITCH_TEST_H
