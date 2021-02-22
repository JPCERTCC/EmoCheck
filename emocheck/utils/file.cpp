#include "file.hpp"

#include "utils.hpp"

namespace emocheck {

boolean CheckPeExtension(std::string filename) {
    std::string extension = filename.substr(filename.find_last_of(".") + 1);
    std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char c) { return std::tolower(c); });

    if (extension == "exe") {
        return TRUE;
    } else if (extension == "dll") {
        return TRUE;
    } else {
        return FALSE;
    }
}

std::string ConvertDivecePath(wchar_t *imagepath) {
    std::string image_path(WideCharToString(imagepath));

    for (int letter = L'A'; letter <= L'Z'; letter++) {
        wchar_t drive[4] = {wchar_t(letter), L':', 0, 0};
        wchar_t drivepath[MAX_PATH + 1];
        if (0 == QueryDosDeviceW(drive, drivepath, _countof(drivepath))) {
            continue;
        }
        std::string drive_letter = WideCharToString(drive);
        std::string device_path = WideCharToString(drivepath);

        std::string::size_type pos(image_path.find(device_path));
        if (pos != std::string::npos) {
            image_path.replace(pos, device_path.length(), drive_letter);
            return image_path;
        }
    }
    return image_path;
}

unsigned int GetVolumeSerialNumber() {
    wchar_t windows_directory_path[MAX_PATH + 1] = {0};
    wchar_t volumename[MAX_PATH + 1] = {0};
    wchar_t filesystemname[MAX_PATH + 1] = {0};
    std::wstring drive_letter;
    DWORD serialnumber = 0;
    DWORD max_componentlen = 0;
    DWORD filesystem_flags = 0;

    // get drive letter from system drive
    if (GetWindowsDirectoryW(windows_directory_path, MAX_PATH)) {
        for (int i = 0; i < 3; i++) {
            drive_letter += windows_directory_path[i];
        }
    } else {
        drive_letter = std::wstring(L"C:\\");
    }

    if (GetVolumeInformationW(
            drive_letter.c_str(),
            volumename,
            sizeof(volumename),
            &serialnumber,
            &max_componentlen,
            &filesystem_flags,
            filesystemname,
            sizeof(filesystemname)) == TRUE) {
    } else {
        std::clog << "[!] GetVolumeInformation() failed, error " << GetLastError() << std::endl;
        std::exit(1);
    }
    return serialnumber;
}

long long int AcquireFileSize(const char *fileName) {
    HANDLE handle = CreateFileA(
        fileName,
        GENERIC_READ,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);
    if (handle == INVALID_HANDLE_VALUE) {
        return -1LL;
    }

    LARGE_INTEGER fsize;
    if (!GetFileSizeEx(handle, &fsize)) {
        CloseHandle(handle);
        return -1LL;
    }

    CloseHandle(handle);
    return fsize.QuadPart;
}

std::string EscapeBackSlash(std::string s) {
    std::string target = "\\";
    std::string replacement = "\\\\";
    if (!target.empty()) {
        std::string::size_type pos = 0;
        while ((pos = s.find(target, pos)) != std::string::npos) {
            s.replace(pos, target.length(), replacement);
            pos += replacement.length();
        }
    }
    return s;
}

bool ComparePath(std::string path_a, std::string path_b) {
    BY_HANDLE_FILE_INFORMATION file_a_info;
    BY_HANDLE_FILE_INFORMATION file_b_info;

    HANDLE file_handle = CreateFileA(
        path_a.c_str(),
        GENERIC_READ,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (file_handle == INVALID_HANDLE_VALUE)
        return FALSE;

    GetFileInformationByHandle(file_handle, &file_a_info);
    CloseHandle(file_handle);

    file_handle = CreateFileA(
        path_b.c_str(),
        GENERIC_READ,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (file_handle == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    GetFileInformationByHandle(file_handle, &file_b_info);
    CloseHandle(file_handle);

    if (file_a_info.dwVolumeSerialNumber == file_b_info.dwVolumeSerialNumber && file_a_info.nFileIndexLow == file_b_info.nFileIndexLow && file_a_info.nFileIndexHigh == file_b_info.nFileIndexHigh) {
        return TRUE;
    }

    return FALSE;
}

boolean is_pefile(std::string path) {
    HANDLE hFile = CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        return FALSE;

    static DWORD size;
    static LPSTR magic;
    magic = (LPSTR)calloc(1, sizeof(DWORD));

    ReadFile(hFile, magic, 2, &size, NULL);
    CloseHandle(hFile);

    if (magic[0] == 0x4D && magic[1] == 0x5A) {
        return TRUE;
    }
    return FALSE;
}

long long int TakeDiffFileTime(FILETIME *ft1, FILETIME *ft2) {
    // return the diff time of two files or directories.
    // resolution of time: 1sec
    ULARGE_INTEGER ul1;
    ULARGE_INTEGER ul2;
    long long int ll1, ll2;

    ul1.LowPart = ft1->dwLowDateTime;
    ul1.HighPart = ft1->dwHighDateTime;
    ll1 = ul1.QuadPart;
    ul2.LowPart = ft2->dwLowDateTime;
    ul2.HighPart = ft2->dwHighDateTime;
    ll2 = ul2.QuadPart;
    return ll2 - ll1;
}

}  // namespace emocheck