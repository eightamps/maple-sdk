//
// Created by lukebayes on 4/30/21.
//

#ifndef MAPLE_HID_CLIENT_H
#define MAPLE_HID_CLIENT_H

typedef struct HidClientContext {
  int vid;
  int pid;
}HidClientContext;

struct HidClientContext *hid_client_new(void);

int hid_client_open(struct HidClientContext *c);

int hid_client_set_vid(struct HidClientContext *c, int vid);
int hid_client_get_vid(struct HidClientContext *c);

int hid_client_set_pid(struct HidClientContext *c, int pid);
int hid_client_get_pid(struct HidClientContext *c);

void hid_client_free(struct HidClientContext *c);

#endif //MAPLE_HID_CLIENT_H
