#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>

int main() int {
  HRESULT status = CoCreateInstance(&CLSID_MMDeviceEnumerator, NULL,
      CLSCTX_ALL, &IID_IMMDeviceEnumerator, (void **)&enumerator);
  EXIT_ON_ERROR(status, "CoCreateInstance with p_enumerator failed");

  status = IMMDeviceEnumerator_GetDefaultAudioEndpoint(enumerator, datadir,
      role, device);
  EXIT_ON_ERROR(status, "GetDefaultAudioEndpoint failed");
  log_info("get_default_device returned");
}
