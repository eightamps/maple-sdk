#include <windows.h>

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
      return MessageBox(NULL, "Hello, World", "caption", 0);
}
