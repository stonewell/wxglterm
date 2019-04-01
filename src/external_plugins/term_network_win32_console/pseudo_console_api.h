#pragma once

#include <windows.h>

typedef void * HPCON;

#define PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE  ProcThreadAttributeValue(22, FALSE, TRUE, FALSE)

HRESULT CreatePseudoConsole(COORD size, HANDLE hInput, HANDLE hOutput, DWORD dwFlags, HPCON * phPC);
HRESULT ResizePseudoConsole(HPCON hPC, COORD size);
HRESULT ClosePseudoConsole(HPCON hPC);

#if _WIN32_WINNT < 0x0600
BOOL InitializeProcThreadAttributeList(
  LPPROC_THREAD_ATTRIBUTE_LIST lpAttributeList,
  DWORD                        dwAttributeCount,
  DWORD                        dwFlags,
  PSIZE_T                      lpSize
);
BOOL UpdateProcThreadAttribute(
  LPPROC_THREAD_ATTRIBUTE_LIST lpAttributeList,
  DWORD                        dwFlags,
  DWORD_PTR                    Attribute,
  PVOID                        lpValue,
  SIZE_T                       cbSize,
  PVOID                        lpPreviousValue,
  PSIZE_T                      lpReturnSize
);
void DeleteProcThreadAttributeList(
  LPPROC_THREAD_ATTRIBUTE_LIST lpAttributeList
);
#endif
