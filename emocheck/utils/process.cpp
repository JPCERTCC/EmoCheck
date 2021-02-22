#include "process.hpp"

#include "file.hpp"
#include "native.hpp"
#include "utils.hpp"

namespace emocheck {

static DWORD pp_offset;
static DWORD cmdline_offset;
static BOOL is_wow64 = FALSE;

void init_arch_info() {
    SYSTEM_INFO si;
    GetNativeSystemInfo(&si);
    pp_offset = si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ? 0x20 : 0x10;
    cmdline_offset = si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ? 0x70 : 0x40;
    return;
}

std::string GetImageFileName(DWORD pid) {
    HANDLE h_process;
    wchar_t imagepath[MAX_PATH + 1];
    h_process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (h_process == NULL) {
        return std::string("");
    }
    if (K32GetProcessImageFileNameW(h_process, imagepath, sizeof(imagepath) / sizeof(*imagepath)) == 0) {
        CloseHandle(h_process);
        return std::string("");
    } else {
        CloseHandle(h_process);
        return ConvertDivecePath(imagepath);
    }
}

std::string GetProcessCmdLine(DWORD pid) {
    std::string cmdline;
    WCHAR* w_cmdline;
    HANDLE h_process;
    LSTATUS err;

    h_process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);

    if (h_process == NULL) {
        return std::string("");
    }

    // get process parameter address
    if (!is_wow64) {
        PROCESS_BASIC_INFORMATION* pbi = new PROCESS_BASIC_INFORMATION();
        PEB* peb = new PEB();
        _RTL_USER_PROCESS_PARAMETERS* pp = new _RTL_USER_PROCESS_PARAMETERS();  // process parameter

        // read PBI
        NtQueryInformationProcess(h_process, 0, pbi, sizeof(PROCESS_BASIC_INFORMATION), NULL);

        // read PEB
        if (!ReadProcessMemory(h_process, pbi->PebBaseAddress, peb, sizeof(PEB), NULL)) {
            err = GetLastError();
            CloseHandle(h_process);
            DBG("[err] Failed to get address of PEB. error_code: " << err);
            return std::string("");
        }
        // read pp
        if (!ReadProcessMemory(h_process, peb->ProcessParameters, pp, sizeof(_RTL_USER_PROCESS_PARAMETERS), NULL)) {
            err = GetLastError();
            CloseHandle(h_process);
            DBG("[err] Failed to get address of process parameters. error_code: " << err);
            return std::string("");
        }
        // ref: https://docs.microsoft.com/ja-jp/windows/win32/api/subauth/ns-subauth-unicode_string
        // Buffer of _UNICODE_STRING might not be null-terminated
        w_cmdline = new WCHAR[(pp->CommandLine.Length) + 1]();
        if (!ReadProcessMemory(h_process, pp->CommandLine.Buffer, w_cmdline, pp->CommandLine.Length, NULL)) {
            CloseHandle(h_process);
            DBG("failed to read command line.");
            return std::string("");
        }

        cmdline = WideCharToString(w_cmdline);
        CloseHandle(h_process);
        // DBG(""
        //     << "[PID]" << std::dec << pid << "\t"
        //     << "[PEB] " << std::hex << pbi->PebBaseAddress << "\t"
        //     << "[PP] " << std::hex << peb->ProcessParameters << "\t"
        //     << "[BUF] " << std::hex << pp->CommandLine.Buffer << "\t"
        //     << "[CMD] " << cmdline);

    } else {
        // Support reading 64bit process memory from WOW64 process
        // (Warning: This function is depending on UNDOCUMENTED APIs.)
        // Read the ProcessParameters address from PEB structure
        // ref: https://docs.microsoft.com/en-us/windows/win32/api/winternl/ns-winternl-peb
        PROCESS_BASIC_INFORMATION_WOW64* pbi64 = new PROCESS_BASIC_INFORMATION_WOW64();
        PEB64* peb64 = new PEB64();
        _RTL_USER_PROCESS_PARAMETERS64* pp64 = new _RTL_USER_PROCESS_PARAMETERS64();
        WCHAR* w_cmdline;
        // read PBI64
        err = NtWow64QueryInformationProcess64(h_process, (PROCESSINFOCLASS)0, pbi64, sizeof(*pbi64), NULL);
        if (err != 0) {
            DWORD err = GetLastError();
            CloseHandle(h_process);
            std::cout << "[err] fail to read PBI64. error_code: " << err << std::endl;
            return std::string("");
        }
        //read PEB64
        err = NtWow64ReadVirtualMemory64(h_process, pbi64->PebBaseAddress, peb64, sizeof(*peb64), NULL);
        if (err != 0) {
            DWORD err = GetLastError();
            CloseHandle(h_process);
            DBG("[err] fail to read PEB64. error_code: " << err);
            return std::string("");
        }
        // read ProcessParameter structure
        err = NtWow64ReadVirtualMemory64(h_process, peb64->ProcessParameters, pp64, sizeof(*pp64), NULL);
        if (err != 0) {
            DWORD err = GetLastError();
            CloseHandle(h_process);
            DBG("[err] fail to read PP64. error_code: " << err);
            return std::string("");
        }

        // read CommandLine from UnicodeStrin64 structure.
        w_cmdline = new WCHAR[pp64->CommandLine.Length + 1]();
        err = NtWow64ReadVirtualMemory64(h_process, pp64->CommandLine.Buffer, w_cmdline, pp64->CommandLine.Length, NULL);
        if (err != 0) {
            DWORD err = GetLastError();
            CloseHandle(h_process);
            DBG("[err] fail to read CommandLine. error_code: " << err);
            return std::string("");
        }
        cmdline = WideCharToString(w_cmdline);
        CloseHandle(h_process);
        // DBG(""
        //     << "[PID] " << std::dec << pid
        //     << "\t[PEB64] " << std::hex << pbi64->PebBaseAddress
        //     << "\t[PP64] " << std::hex << peb64->ProcessParameters
        //     << "\t[buf ptr] " << std::hex << pp64->CommandLine.Buffer
        //     << "\t[buf size] " << std::hex << pp64->CommandLine.Length
        //     << "\t[cmdline] " << cmdline);
    }
    return cmdline;
}

std::vector<Proc> ListProcess() {
    std::vector<Proc> process_list;
    Proc* proc;

    // init offsets
    init_arch_info();

    // determine if the processs is running on WOW64
    IsWow64Process(GetCurrentProcess(), &is_wow64);

    // init Native funcitons
    init_native_func(is_wow64);

    PROCESSENTRY32 pe = {sizeof(PROCESSENTRY32)};
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    BOOL result;

    for (result = Process32FirstW(snapshot, &pe); result == TRUE; result = Process32NextW(snapshot, &pe)) {
        proc = new Proc();
        proc->name = WideCharToString(pe.szExeFile);
        proc->PID = int(pe.th32ProcessID);
        proc->image_path = GetImageFileName(proc->PID);
        if (proc->image_path != "")
            proc->cmd_line = GetProcessCmdLine(proc->PID);

        process_list.push_back(*proc);
    }
    return process_list;
}

}  // namespace emocheck