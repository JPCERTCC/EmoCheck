#include "utils.hpp"

// standard modules
#include <iostream>
#include <vector>

// windows basic modules
#include <Windows.h>

// windows additional modules
#include <Psapi.h>

namespace emocheck {

std::string WideCharToString(wchar_t *wide_char) {
    int buf_size = WideCharToMultiByte(CP_OEMCP, 0, wide_char, -1, (char *)NULL, 0, NULL, NULL);
    char *char_ptr = new char[buf_size];

    WideCharToMultiByte(CP_OEMCP, 0, wide_char, -1, char_ptr, buf_size, NULL, NULL);
    std::string result(char_ptr, char_ptr + buf_size - 1);
    delete[] char_ptr;

    return result;
}

std::vector<unsigned char> IntToBytes(unsigned int integer) {
    std::vector<unsigned char> bytes_vector(4);
    for (int i = 0; i < 4; i++)
        bytes_vector[i] = (integer >> (i * 8));
    return bytes_vector;
}

boolean CheckPeExtension(std::string filename) {
    if (filename.substr(filename.find_last_of(".") + 1) == "exe") {
        return TRUE;
    } else if (filename.substr(filename.find_last_of(".") + 1) == "dll") {
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
        if (0 == ::QueryDosDevice(drive, drivepath, _countof(drivepath))) {
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

std::string GetImageFileName(DWORD pid) {
    HANDLE hProcess;
    wchar_t imagepath[MAX_PATH + 1];
    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (hProcess == NULL) {
        std::clog << "[!!] Error: Failed to get handle from pid " << pid << std::endl;
        return std::string("");
    }
    if (GetProcessImageFileName(hProcess, imagepath, sizeof(imagepath) / sizeof(*imagepath)) == 0) {
        std::clog << "[!!] Error: Failed to get image file name from pid " << pid << std::endl;
        return std::string("");
    } else {
        return ConvertDivecePath(imagepath);
    }
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
    if (GetWindowsDirectory(windows_directory_path, MAX_PATH)) {
        for (int i = 0; i < 3; i++) {
            drive_letter += windows_directory_path[i];
        }
    } else {
        drive_letter = std::wstring(L"C:\\");
    }

    if (GetVolumeInformation(
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

}  // namespace emocheck