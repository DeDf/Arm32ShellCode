
#include "stdafx.h"
#include "ShellCode.h"

void __stdcall test()
{
    OutputDebugStringA("test() ~\n"); 
}

typedef void (__stdcall *test_t)();

ULONG_PTR InjectLibARM32(InjectShellCodeArm32 *InjectParm)
{
    while (InjectParm)
    {
        ULONG_PTR p = 0;
        InjectParm->pLdrLoadDll(0, NULL, (PUNICODE_STRING32)&(InjectParm->dll), (HANDLE*)&p);
        InjectParm = (InjectShellCodeArm32*)InjectParm->next;
    }

    return 0;
}

int main()
{
    printf("sizeof(InjectLibArm32) = %d\n", sizeof(InjectShellCodeArm32));
    HANDLE ProcessHandle = GetCurrentProcess();
    BOOL b;
    ULONG op;

    HMODULE hNtdll = LoadLibraryA("ntdll.dll");
    printf("hNtdll %p\n", hNtdll);
    PVOID Nta64  = GetProcAddress(hNtdll, "NtTestAlert");
    PVOID Lld64  = GetProcAddress(hNtdll, "LdrLoadDll");
    printf("Nta64 %p\n", Nta64);
    printf("Lld64 %p\n", Lld64);

    PInjectShellCodeArm32 parm = (PInjectShellCodeArm32)malloc(4096);
    memset(parm, 0, 4096);
    printf("parm : %p\n", parm);

    *(PVOID*)&parm->pLdrLoadDll = Lld64;

    parm->PushR0_R3  = 0xB40F;
    parm->PushLr     = 0xB500;
    parm->MovR0Pc    = 0x4678;
    parm->SubR0_8    = 0x3808;
    parm->BlInjectFuncHigh = 0xF000;
    parm->BlInjectFuncLow  = 0xF918;
    parm->PopR0      = 0xBC01;
    parm->MovLrR0    = 0x4686;
    parm->PopR0_R3   = 0xBC0F;

//     ULONG_PTR pFuncAddr = (ULONG_PTR)Nta64;
//     if (pFuncAddr & 1)
//         pFuncAddr--;

    //memcpy(&parm->OrigFunc, (PVOID)pFuncAddr, 16);

    parm->OrigFunc[0] = 0x00;  // push {lr}
    parm->OrigFunc[1] = 0xB5;

    parm->OrigFunc[2] = 0x00;  // pop {pc}
    parm->OrigFunc[3] = 0xBD;

    parm->dll.Buffer = parm->dllBuf;
    parm->dll.Length = parm->dll.MaximumLength = sizeof(L"c:\\32.dll") - 2;
    memcpy(parm->dll.Buffer, L"c:\\32.dll", sizeof(L"c:\\32.dll") - 2);

    ULONG_PTR pFuncAddr = (ULONG_PTR)InjectLibARM32;
    if (pFuncAddr & 1)
        pFuncAddr--;
    memcpy(parm->injectFunc, (PVOID)pFuncAddr, 0x100);
    printf("parm->injectFunc = %p\n", parm->injectFunc);

    b = VirtualProtectEx(ProcessHandle, parm, 4096, PAGE_EXECUTE_READWRITE, &op);

    //---------------------------------------------------------------

    ULONG_PTR TestAddr = (ULONG_PTR)&InjectLibARM32;
    ULONG_PTR fixTestAddr = TestAddr & (~1);
    printf("TestAddr = %x, fixTestAddr = %x\n", TestAddr, fixTestAddr);

    UCHAR *c = (UCHAR *)&parm;

    UCHAR *t = (UCHAR*)malloc(4096);
    memset(t, 0, 4096);
    printf("t = %p\n", t);
    {
        t[0] = 0x78;  // bx pc
        t[1] = 0x47;

        t[2] = 0xc0;  // nop
        t[3] = 0x46;

        t[4] = 0x04;  // ldr pc, [pc, #-4]
        t[5] = 0xf0;
        t[6] = 0x1f;
        t[7] = 0xe5;

        t[8] = c[0];  // DCD
        t[9] = c[1];
        t[10] = c[2];
        t[11] = c[3];

        t[8] += 1;
    }

    b = VirtualProtectEx(ProcessHandle, t, 4096, PAGE_EXECUTE_READWRITE, &op);

    t++;
    test_t test1 = (test_t)t;
    printf("test1 = %p\n", test1);

    //__debugbreak();
    test1();
    getchar();
    return 0;
}

