#ifndef EMOCHECK_UTILS_NATIVE_HPP_
#define EMOCHECK_UTILS_NATIVE_HPP_
// windows basic modules
#include <Windows.h>

#include "winternl.h"

// windows additional modules
#include <Psapi.h>

namespace emocheck {
// PROCESS_BASIC_INFORMATION for pure 32 and 64-bit processes
typedef struct _PROCESS_BASIC_INFORMATION {
    PVOID Reserved1;
    PVOID PebBaseAddress;
    PVOID Reserved2[2];
    ULONG_PTR UniqueProcessId;
    PVOID Reserved3;
} PROCESS_BASIC_INFORMATION;

// PROCESS_BASIC_INFORMATION for 32-bit process on WOW64
typedef struct _PROCESS_BASIC_INFORMATION_WOW64 {
    NTSTATUS ExitStatus;
    ULONG64 PebBaseAddress;
    ULONG64 AffinityMask;
    KPRIORITY BasePriority;
    ULONG64 UniqueProcessId;
    ULONG64 InheritedFromUniqueProcessId;
} PROCESS_BASIC_INFORMATION_WOW64, *PPROCESS_BASIC_INFORMATION_WOW64;

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR Buffer;
} UNICODE_STRING;

typedef struct _UNICODE_STRING_WOW64 {
    USHORT Length;
    USHORT MaximumLength;
    UINT64 Buffer;
} UNICODE_STRING_WOW64;

typedef struct _CURDIR64 {
    UNICODE_STRING_WOW64 DosPath;
    HANDLE Handle;
} CURDIR64, *PCURDIR64;

typedef struct _RTL_USER_PROCESS_PARAMETERS {
    BYTE Reserved1[16];
    PVOID Reserved2[10];
    UNICODE_STRING ImagePathName;
    UNICODE_STRING CommandLine;
} RTL_USER_PROCESS_PARAMETERS, *PRTL_USER_PROCESS_PARAMETERS;

typedef struct _RTL_DRIVE_LETTER_CURDIR {
    USHORT Flags;
    USHORT Length;
    ULONG TimeStamp;
    STRING DosPath;

} RTL_DRIVE_LETTER_CURDIR, *PRTL_DRIVE_LETTER_CURDIR;

typedef struct _RTL_USER_PROCESS_PARAMETERS64 {
    ULONG MaximumLength;  // Should be set before call RtlCreateProcessParameters
    ULONG Length;         // Length of valid structure
    ULONG Flags;          // Currently only PPF_NORMALIZED (1) is known:
                          //  - Means that structure is normalized by call RtlNormalizeProcessParameters
    ULONG DebugFlags;
    UINT64 ConsoleHandle;  // HWND to console window associated with process (if any).
    ULONG ConsoleFlags;
    DWORD64 StandardInput;
    DWORD64 StandardOutput;
    DWORD64 StandardError;
    CURDIR64 CurrentDirectory;  // Specified in DOS-like symbolic link path, ex: "C:/WinNT/SYSTEM32"

    UNICODE_STRING_WOW64 DllPath;        // DOS-like paths separated by ';' where system should search for DLL files.
    UNICODE_STRING_WOW64 ImagePathName;  // Full path in DOS-like format to process'es file image.
    UNICODE_STRING_WOW64 CommandLine;    // Command line
    UINT64 Environment;                  // Pointer to environment block (see RtlCreateEnvironment)
    ULONG StartingX;
    ULONG StartingY;
    ULONG CountX;
    ULONG CountY;
    ULONG CountCharsX;
    ULONG CountCharsY;
    ULONG FillAttribute;  // Fill attribute for console window
    ULONG WindowFlags;
    ULONG ShowWindowFlags;
    UNICODE_STRING_WOW64 WindowTitle;
    UNICODE_STRING_WOW64 DesktopInfo;  // Name of WindowStation and Desktop objects, where process is assigned
    UNICODE_STRING_WOW64 ShellInfo;
    UNICODE_STRING_WOW64 RuntimeData;
    RTL_DRIVE_LETTER_CURDIR CurrentDirectores[0x20];
    ULONG EnvironmentSize;
} RTL_USER_PROCESS_PARAMETERS64, *PRTL_USER_PROCESS_PARAMETERS64;

// PEB 64:
typedef struct _PEB64 {
    BYTE Reserved[16];
    UINT64 ImageBaseAddress;
    UINT64 LdrData;
    UINT64 ProcessParameters;
} PEB64, *PPEB64;

// externs
extern NTSTATUS(NTAPI *NtWow64ReadVirtualMemory64)(
    IN HANDLE ProcessHandle,
    IN UINT64 BaseAddress,
    OUT PVOID Buffer,
    IN ULONG64 Size,
    OUT PULONG64 NumberOfBytesRead);

extern NTSTATUS(NTAPI *NtWow64QueryInformationProcess64)(
    IN HANDLE ProcessHandle,
    IN PROCESSINFOCLASS ProcessInformationClass,
    OUT PVOID ProcessInformation,
    IN ULONG ProcessInformationLength,
    OUT PULONG ReturnLength OPTIONAL);

extern NTSTATUS(WINAPI *NtQueryInformationProcess)(
    IN HANDLE h_process,
    IN ULONG ProcessInformationClass,
    OUT PVOID ProcessInformation,
    IN ULONG ProcessInformationLength,
    OUT PULONG ReturnLength);

extern NTSTATUS(NTAPI *RtlCreateProcessParametersEx)(
    _Out_ PRTL_USER_PROCESS_PARAMETERS *pProcessParameters,
    _In_ PUNICODE_STRING ImagePathName,
    _In_opt_ PUNICODE_STRING DllPath,
    _In_opt_ PUNICODE_STRING CurrentDirectory,
    _In_opt_ PUNICODE_STRING CommandLine,
    _In_opt_ PVOID Environment,
    _In_opt_ PUNICODE_STRING WindowTitle,
    _In_opt_ PUNICODE_STRING DesktopInfo,
    _In_opt_ PUNICODE_STRING ShellInfo,
    _In_opt_ PUNICODE_STRING RuntimeData,
    _In_ ULONG Flags  // pass RTL_USER_PROC_PARAMS_NORMALIZED to keep parameters normalized
);

// functions
void init_native_func(bool is_wow64);

}  // namespace emocheck
#endif  //EMOCHECK_UTILS_NATIVE_HPP_
