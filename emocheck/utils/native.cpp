#include "native.hpp"

namespace emocheck {

NTSTATUS(NTAPI *NtWow64ReadVirtualMemory64)
(
    IN HANDLE ProcessHandle,
    IN UINT64 BaseAddress,
    OUT PVOID Buffer,
    IN ULONG64 Size,
    OUT PULONG64 NumberOfBytesRead) = nullptr;

NTSTATUS(NTAPI *NtWow64QueryInformationProcess64)
(
    IN HANDLE ProcessHandle,
    IN PROCESSINFOCLASS ProcessInformationClass,
    OUT PVOID ProcessInformation,
    IN ULONG ProcessInformationLength,
    OUT PULONG ReturnLength OPTIONAL) = nullptr;

NTSTATUS(WINAPI *NtQueryInformationProcess)
(
    IN HANDLE ProcessHandle,
    IN ULONG ProcessInformationClass,
    OUT PVOID ProcessInformation,
    IN ULONG ProcessInformationLength,
    OUT PULONG ReturnLength) = nullptr;

NTSTATUS(NTAPI *RtlCreateProcessParametersEx)
(
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
    ) = nullptr;

void init_native_func(bool is_wow64) {
    HMODULE lib = LoadLibraryA("ntdll.dll");
    FARPROC proc;
    if (lib == nullptr)
        return;

    proc = GetProcAddress(lib, "RtlCreateProcessParametersEx");
    if (proc)
        RtlCreateProcessParametersEx = (NTSTATUS(NTAPI *)(
            PRTL_USER_PROCESS_PARAMETERS *,
            PUNICODE_STRING,
            PUNICODE_STRING,
            PUNICODE_STRING,
            PUNICODE_STRING,
            PVOID,
            PUNICODE_STRING,
            PUNICODE_STRING,
            PUNICODE_STRING,
            PUNICODE_STRING,
            ULONG))proc;

    proc = GetProcAddress(lib, "NtQueryInformationProcess");
    if (proc)
        NtQueryInformationProcess = (NTSTATUS(NTAPI *)(
            HANDLE,
            ULONG,
            PVOID,
            ULONG,
            PULONG))proc;

    if (is_wow64) {
        proc = GetProcAddress(lib, "NtWow64ReadVirtualMemory64");
        if (proc)
            NtWow64ReadVirtualMemory64 = (NTSTATUS(NTAPI *)(
                HANDLE,
                UINT64,
                PVOID,
                ULONG64,
                PULONG64))proc;

        proc = GetProcAddress(lib, "NtWow64QueryInformationProcess64");
        if (proc)
            NtWow64QueryInformationProcess64 = (NTSTATUS(NTAPI *)(
                HANDLE,
                PROCESSINFOCLASS,
                PVOID,
                ULONG,
                PULONG))proc;
    }
    return;
}

}  // namespace emocheck