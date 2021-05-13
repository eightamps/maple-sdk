//
// Created by lukebayes on 4/25/21.
//
#include "log.h"
#include "share.h"
#include "stitch.h"
#include <stdlib.h>
#include <stdio.h>

#include <mmdeviceapi.h>
#include <audioclient.h>

typedef struct {

} stitch_win32_context_t;

// static const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
// static const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
// static const IID IID_IAudioClient = __uuidof(IAudioClient);
// static const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);

DLL_LINK stitch_context_t *stitch_new_with_label(char *label) {
  log_info("stitch_new_with_label called with: %s", label);
  stitch_context_t *c = stitch_new();
  if (c != NULL) {
    c->label = label;
  }
  return c;
}

DLL_LINK stitch_context_t *stitch_new(void) {
  log_info("win32 stitch_new called");
  stitch_context_t *c = calloc(sizeof(stitch_context_t), 1);
  if (c == NULL) {
    log_err("stitch_new failed to allocate instance context");
    return NULL;
  }
  stitch_win32_context_t *wc = calloc(sizeof(stitch_win32_context_t), 1);
  if (wc == NULL) {
    log_err("stitch_new failed to allocate win32 context");
    free(c);
    return NULL;
  }
  c->platform = wc;
  return c;
}

DLL_LINK int stitch_init(stitch_context_t *c) {
  log_info("win32: stitch_init");
  HRESULT status;
  IMMDeviceEnumerator *p_enumerator = NULL;
  IMMDevice *p_device = NULL;

  status = CoInitialize(NULL);
  EXIT_ON_ERROR(status, "CoInitialize failed");

  status = CoCreateInstance(&CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL,
    &IID_IMMDeviceEnumerator, (void **)&p_enumerator);
  EXIT_ON_ERROR(status, "CoCreateInstance with p_enumerator failed");

  status = IMMDeviceEnumerator_GetDefaultAudioEndpoint(p_enumerator, eRender,
    eConsole, &p_device);
  EXIT_ON_ERROR(status, "GetDefaultAudioEndpoint failed");

  log_info("STITCH INITIALIZED MOTHAFUCKA!");

  goto ExitSuccess;

ExitWithError:
  log_err("stitch_init ExitWithError");

ExitSuccess:
  return EXIT_SUCCESS;
}

DLL_LINK int stitch_set_dtmf(stitch_context_t *c, dtmf_context_t *d) {
  return EXIT_SUCCESS;
}

DLL_LINK int stitch_get_default_input_index(stitch_context_t *c) {
  return EXIT_SUCCESS;
}

DLL_LINK int stitch_get_default_output_index(stitch_context_t *c) {
  return EXIT_SUCCESS;
}

DLL_LINK int stitch_get_matching_input_device_index(stitch_context_t *c, char *name) {
  return EXIT_SUCCESS;
}

DLL_LINK int stitch_get_matching_output_device_index(stitch_context_t *c, char *name) {
  return EXIT_SUCCESS;
}

DLL_LINK int stitch_start(stitch_context_t *c, int in_index, int out_index) {
  return EXIT_SUCCESS;
}

DLL_LINK int stitch_stop(stitch_context_t *c) {
  return EXIT_SUCCESS;
}

DLL_LINK int stitch_join(stitch_context_t *c) {
  return EXIT_SUCCESS;
}

DLL_LINK void stitch_free(stitch_context_t *c) {
  if (c != NULL) {
    if (c->platform != NULL) {
      free(c->platform);
    }
    free(c);

    log_info("stitch_free completed successfully");
  }
}
