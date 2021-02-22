#include "utils.hpp"

namespace emocheck {

bool Is64bit() {
#if defined(_WIN64)
    return true;  // 64-bit programs run only on Win64
#elif defined(_WIN32)
    return false;
#endif
}

std::string WideCharToString(wchar_t *wide_char) {
    int buf_size = WideCharToMultiByte(CP_OEMCP, 0, wide_char, -1, (char *)NULL, 0, NULL, NULL);
    char *char_ptr = new char[buf_size]();

    WideCharToMultiByte(CP_OEMCP, 0, wide_char, -1, char_ptr, buf_size, NULL, NULL);
    std::string result(char_ptr, char_ptr + buf_size - 1);
    delete[] char_ptr;

    return result;
}

const wchar_t *StringToWideChar(std::string str) {
    int len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), NULL, 0);
    wchar_t *wc = new wchar_t[len + 1]();
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), wc, len);
    return wc;
}

std::vector<unsigned char> IntToBytes(unsigned int integer) {
    std::vector<unsigned char> bytes_vector(4);
    for (int i = 0; i < 4; i++)
        bytes_vector[i] = (integer >> (i * 8));
    return bytes_vector;
}

bool contain(std::string s, const char *v) {
    return s.find(v) != std::string::npos;
}

void load_libs() {
    LoadLibraryA("advapi32.dll");
    LoadLibraryA("shlwapi.dll");
    LoadLibraryA("shell32.dll");
    return;
}

void free_libs() {
    FreeLibrary(GetModuleHandleA("advapi32.dll"));
    FreeLibrary(GetModuleHandleA("shlwapi.dll"));
    FreeLibrary(GetModuleHandleA("shell32.dll"));
    return;
}

bool IsWindows7() {
    OSVERSIONINFOEX os_ver;
    ULONGLONG condition = 0;
    os_ver.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    os_ver.dwMajorVersion = 6;
    os_ver.dwMinorVersion = 1;
    VER_SET_CONDITION(condition, VER_MAJORVERSION, VER_EQUAL);

    return VerifyVersionInfoW(&os_ver, VER_MAJORVERSION | VER_MINORVERSION, condition);
}

std::string GetSysDirX86() {
    std::string syspath;
    SYSTEM_INFO si;
    wchar_t path[MAX_PATH];

    GetNativeSystemInfo(&si);
    if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
        SHGetSpecialFolderPathW(NULL, path, CSIDL_SYSTEMX86, 0);
    else
        SHGetSpecialFolderPathW(NULL, path, CSIDL_SYSTEM, 0);

    syspath = WideCharToString(path);
    return syspath;
}

boolean IsWinOSx64() {
    SYSTEM_INFO si;
    GetNativeSystemInfo(&si);
    return si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64;
}

std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return s;
}

std::string RemoveExtraWhitespaces(std::string input) {
    std::string output;
    std::unique_copy(input.begin(), input.end(), std::back_insert_iterator<std::string>(output),
                     [](char a, char b) { return isspace(a) && isspace(b); });
    return output;
}

std::string PathUnification(std::string path) {
    // this function removes double quatation and doubled spaces
    // And then unifies alphabet to lower case.
    path.erase(std::remove(path.begin(), path.end(), '"'), path.end());
    path = RemoveExtraWhitespaces(path);
    return toLower(path);
}

std::string ReplaceString(std::string str, const char *before, const char *after, int cnt) {
    size_t pos = str.find(before);
    while (pos != std::string::npos && cnt != 0) {
        str.replace(pos, strlen(before), after);
        pos = str.find(before, pos + strlen(after));
        cnt--;
    }
    return str;
}

std::vector<std::string> split(std::string str, std::string sep) {
    if (sep == "") return {str};
    std::vector<std::string> res;
    std::string tstr = str + sep;
    size_t l = tstr.length();
    size_t sl = sep.length();
    size_t prev = 0;
    for (size_t pos = 0; pos < l && (pos = tstr.find(sep, pos)) != std::string::npos; prev = (pos += sl)) {
        res.emplace_back(tstr, prev, pos - prev);
    }
    return res;
}

Rundll32Cmd *ParseRundll32CmdLine(std::string cmd_line) {
    LPWSTR *lplpszArgs;
    Rundll32Cmd *rundll32 = new Rundll32Cmd();
    int n;
    lplpszArgs = CommandLineToArgvW(StringToWideChar(cmd_line), &n);
    std::string exe = WideCharToString(lplpszArgs[0]);
    if (contain(exe, "rundll32.exe")) {
        rundll32->bin_path = exe;
        std::vector<std::string> data = split(WideCharToString(lplpszArgs[1]), ",");
        if (data.size() == 2) {
            rundll32->dll = data[0];
            rundll32->export_func = data[1];
        }
        return rundll32;
    } else {
        return nullptr;
    }
}

}  // namespace emocheck