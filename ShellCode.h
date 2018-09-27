#pragma once
#include <windows.h>

typedef struct _UNICODE_STRING32 {
    USHORT  Length;
    USHORT  MaximumLength;
    WCHAR  * POINTER_32 Buffer;
} UNICODE_STRING32, *PUNICODE_STRING32;

typedef UNICODE_STRING32 UNICODE_STRING;

typedef
NTSTATUS (NTAPI * POINTER_32 LdrLoadDll_t)(PWCHAR PathToFile, ULONG *Flags, UNICODE_STRING *ModuleFileName, HANDLE *ModuleHandle);

#pragma pack(push, 1)

typedef struct _InjectShellCodeArm32 * POINTER_32 PInjectShellCodeArm32;
typedef struct _InjectShellCodeArm32
{
    USHORT           PushR0_R3;     // 0F B4
    USHORT           PushLr;        // 00 B5
    USHORT           MovR0Pc;       // 78 46
    USHORT           SubR0_8;       // 08 38
    USHORT           BlInjectFuncHigh; // 00 F0
    USHORT           BlInjectFuncLow;  // 18 F9
    USHORT           PopR0;         // 01 BC
    USHORT           MovLrR0;       // 86 46
    USHORT           PopR0_R3;      // 0F BC
    USHORT           align;         // 00 00
    UCHAR            OrigFunc[16];  // which code location in user land do we patch?
    PInjectShellCodeArm32  next;          // next dll (if any)
    UNICODE_STRING32 dll;           // dll path/name
    WCHAR            dllBuf[260];   // string buffer for the dll path/name
    LdrLoadDll_t     pLdrLoadDll;   // ntdll.LdrLoadDll
    UCHAR            injectFunc[1]; // will be filled with CInjectLibFunc32 (see below)

} InjectShellCodeArm32;

#pragma pack(pop)