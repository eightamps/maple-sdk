//
// Created by lukebayes on 4/25/21.
//
#include "../log.h"
#include "../share.h"
#include "../stitch.h"
#include <stdlib.h>
#include <stdio.h>

#include <mmdeviceapi.h>
#include <audioclient.h>

typedef struct {
  int in_index;
  int out_index;
  IMMDevice *in_device;
  IMMDevice *out_device;
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
  status = CoInitialize(NULL);
  EXIT_ON_ERROR(status, "CoInitialize failed");

  log_info("STITCH INITIALIZED MOTHAFUCKA!");

  goto ExitSuccess;
ExitWithError:
  log_err("stitch_init ExitWithError");
  stitch_free(c);
  return status;
ExitSuccess:
  log_info("stitch_init exiting successfully");
  return EXIT_SUCCESS;
}

DLL_LINK int stitch_set_dtmf(stitch_context_t *c, dtmf_context_t *d) {
  return EXIT_SUCCESS;
}

static int get_default_device(DATADIR datadir, ERole role, IMMDevice **device) {
  IMMDeviceEnumerator *enumerator = NULL;

  HRESULT status = CoCreateInstance(&CLSID_MMDeviceEnumerator, NULL,
      CLSCTX_ALL, &IID_IMMDeviceEnumerator, (void **)&enumerator);
  EXIT_ON_ERROR(status, "CoCreateInstance with p_enumerator failed");

  status = IMMDeviceEnumerator_GetDefaultAudioEndpoint(enumerator, datadir,
      role, device);
  EXIT_ON_ERROR(status, "GetDefaultAudioEndpoint failed");
  log_info("get_default_device returned");

  goto ExitSuccess;
ExitWithError:
  log_err("get_default_device ExitWithError");
ExitSuccess:
  SAFE_FREE(enumerator);
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
  HRESULT status = EXIT_SUCCESS;
  IMMDeviceEnumerator *enumerator = NULL;
  IMMDevice *in_device = NULL;
  IMMDevice *out_device = NULL;

  status = CoCreateInstance(&CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL,
                            &IID_IMMDeviceEnumerator, (void **)&enumerator);
  EXIT_ON_ERROR(status, "CoCreateInstance with p_enumerator failed");

  if (in_index == -1) {
    status = get_default_device(eCapture, eCommunications, &in_device);
    EXIT_ON_ERROR(status, "Failed to get default capture device");
  } else {
    EXIT_ON_ERROR(-100, "stitch_start only supports -1 input index right now");
  }

  if (out_index = -1) {
    status = get_default_device(eRender, eCommunications, &in_device);
    EXIT_ON_ERROR(status, "Failed to get default render device");
  } else {
    EXIT_ON_ERROR(-100, "stitch_start only supports -1 output index right now");
  }

  goto ExitSuccess;
ExitWithError:
  log_err("stitch_start ExitWithError %d", status);
ExitSuccess:
  log_info("stitch_start exiting successfully");
  SAFE_FREE(enumerator);
  SAFE_FREE(in_device);
  SAFE_FREE(out_device);
  return status;
}

DLL_LINK int stitch_stop(stitch_context_t *c) {
  return EXIT_SUCCESS;
}

DLL_LINK int stitch_join(stitch_context_t *c) {
  return EXIT_SUCCESS;
}

DLL_LINK void stitch_free(stitch_context_t *c) {
  if (c != NULL) {
    SAFE_FREE(c->platform);
    SAFE_FREE(c);
    log_info("stitch_free completed successfully");
  }
}
