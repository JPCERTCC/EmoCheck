/*
 * LICENSE
 * Please reffer to the LICENSE.txt in the https://github.com/JPCERTCC/EmoCheck/
 */

#include "emocheck.h"

// standard modules
#include <algorithm>
#include <iostream>
#include <sstream>
#include <vector>

// windows basic modules
#include <Windows.h>

// windows additional modules
#include <Psapi.h>
#include <TlHelp32.h>

namespace emocheck {

bool is_debug = false;

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

std::string QueryRegistry(unsigned int serial, HKEY root, std::wstring key_path) {
    DWORD buf_len;
    LSTATUS status;
    HKEY hKey;
    std::wstringstream wstring_serial;
    std::string filename = "";

    // convert integer to wstring stream
    wstring_serial << std::hex << serial;

    status = RegOpenKeyEx(root, key_path.c_str(), 0, KEY_READ, &hKey);
    if (status) {
        // std::cerr << "[*] Emotet registry key not found. status: " << status << std::endl;
        return filename;
    }

    // debug message
    if (is_debug) {
        if (root == HKEY_CURRENT_USER) {
            std::wcout << L"[DEBUG] Open registry: HKCU\\" << key_path << std::endl;
        } else if (root == HKEY_LOCAL_MACHINE) {
            std::wcout << L"[DEBUG] Open registry: HKLM\\" << key_path << std::endl;
        } else {
            std::wcout << L"[DEBUG] Open registry: " << std::hex << root << L" path: " << key_path << std::endl;
        }
    }

    status = RegGetValue(hKey, nullptr, wstring_serial.str().c_str(), RRF_RT_REG_BINARY, nullptr, nullptr, &buf_len);
    if (status) {
        // std::cerr << "[*] Emotet registry key not found. status: " << status << std::endl;
        return filename;
    }

    // debug message
    if (is_debug) {
        std::wcout << L"[DEBUG] Found suspicous subkey: " << wstring_serial.str() << std::endl;
        std::cout << "[DEBUG] Data length: " << buf_len << std::endl;
    }

    // buffur to save binary_data in registry value.
    unsigned char *buffer = new unsigned char[buf_len];

    RegGetValue(hKey, nullptr, wstring_serial.str().c_str(), RRF_RT_REG_BINARY, nullptr, buffer, &buf_len);

    // XOR registry value with drive serial num;
    unsigned char *decoded_chars = new unsigned char[buf_len];
    std::vector<unsigned char> xor_key = IntToBytes(serial);

    // debug message
    if (is_debug) {
        std::cout << "[DEBUG] Binary data:\n[DEBUG] ";
        for (unsigned int i = 0; i < buf_len; i++) {
            printf("0x%02x ", buffer[i]);
        }
        std::cout << std::endl;
    }

    for (unsigned int i = 0; i < buf_len; i++) {
        decoded_chars[i] = (xor_key[i % 4] ^ buffer[i]);
    }

    // debug message
    if (is_debug) {
        std::cout << "[DEBUG] Decoded data:\n[DEBUG] ";
        for (unsigned int i = 0; i < buf_len; i++) {
            printf("0x%02x ", decoded_chars[i]);
        }
        std::cout << std::endl;
    }

    for (unsigned int i = 0; i < buf_len; i++) {
        if (0x20 < int(decoded_chars[i]) && int(decoded_chars[i]) < 0x7e) {
            filename += decoded_chars[i];
        }
    }
    // debug message
    if (is_debug) {
        std::cout << "[DEBUG] Decoded string: " << filename << std::endl;
    }
    return filename;
}

std::vector<std::string> GetEmotetFileNameFromRegistry(int serial) {
    std::vector<std::string> filenames;
    std::string filename;
    std::wstring reg_key_path = L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer";
    std::wstring reg_key_path_admin = L"Software\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Explorer";

    // if emotet runs with user auth.(x64,x32)
    filename = QueryRegistry(serial, HKEY_CURRENT_USER, reg_key_path);
    if (filename.length() > 0) {
        filenames.push_back(filename);
    }

    // if emotet runs with admin auth. (x32)
    filename = QueryRegistry(serial, HKEY_LOCAL_MACHINE, reg_key_path);
    if (filename.length() > 0) {
        filenames.push_back(filename);
    }

    // if emotet runs with admin auth. (x64)
    filename = QueryRegistry(serial, HKEY_LOCAL_MACHINE, reg_key_path_admin);
    if (filename.length() > 0) {
        filenames.push_back(filename);
    }
    return filenames;
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

std::string GetWord(std::string keywords, int ptr, int keylen) {
    std::string keyword;

    for (int i = ptr; i > 0; i--) {
        if (keywords[i] != ',') {
            continue;
        } else {
            ptr = i;
            break;
        }
    }
    if (keywords[ptr] == ',') {
        ptr++;
    }
    for (int i = ptr; i < keylen; i++) {
        if (keywords[i] != ',') {
            keyword += keywords[i];
            ptr++;
        } else {
            break;
        }
    }
    return keyword;
}

std::string GenerateEmotetProcessName() {
    uint32_t q;
    uint32_t seed;
    int keylen;
    int mod;
    std::string keywords;
    std::string keyword;

    seed = GetVolumeSerialNumber();

    keywords =
        "duck,mfidl,targets,ptr,khmer,purge,metrics,acc,inet,msra,symbol,driver,"
        "sidebar,restore,msg,volume,cards,shext,query,roam,etw,mexico,basic,url,"
        "createa,blb,pal,cors,send,devices,radio,bid,format,thrd,taskmgr,timeout,"
        "vmd,ctl,bta,shlp,avi,exce,dbt,pfx,rtp,edge,mult,clr,wmistr,ellipse,vol,"
        "cyan,ses,guid,wce,wmp,dvb,elem,channel,space,digital,pdeft,violet,thunk";

    keylen = int(keywords.length());

    // first round
    q = seed / keylen;
    mod = seed % keylen;
    keyword += GetWord(keywords, mod, keylen);

    // second round
    seed = 0xFFFFFFFF - q;
    mod = seed % keylen;
    keyword += GetWord(keywords, mod, keylen);

    return keyword;
}

std::vector<EmotetProcess> ScanEmotetProcess(std::vector<std::string> keywords) {
    PROCESSENTRY32 pe = {sizeof(PROCESSENTRY32)};
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    BOOL result;
    std::vector<EmotetProcess> emotet_processes;

    for (result = Process32First(snapshot, &pe);
         result == TRUE; result = Process32Next(snapshot, &pe)) {
        std::string process_name = WideCharToString(pe.szExeFile);
        for (unsigned int i = 0; i < keywords.size(); i++) {
            if (process_name == keywords[i] + std::string(".exe")) {
                EmotetProcess emotet_process;
                emotet_process.pid = int(pe.th32ProcessID);
                emotet_process.process_name = process_name;
                emotet_process.image_path = GetImageFileName(emotet_process.pid);
                emotet_processes.push_back(emotet_process);
            }
        }
    }
    return emotet_processes;
}

std::vector<EmotetProcess> ScanEmotet(bool debug) {
    std::vector<std::string> filenames;
    std::string emotet_process_name;
    std::vector<std::string> emotet_process_names;
    std::vector<EmotetProcess> emotet_processes;

    is_debug = debug;

    // old emotet (- 2020/02/05)
    emotet_process_name = GenerateEmotetProcessName();
    emotet_process_names.push_back(emotet_process_name);

    // new emotet (2020/02/06 -)
    filenames = GetEmotetFileNameFromRegistry(GetVolumeSerialNumber());
    for (unsigned int i = 0; i < filenames.size(); i++) {
        if (filenames[i].length() != 0) {
            emotet_process_names.push_back(filenames[i]);
        }
    }

    // unique emotet processes names
    std::sort(emotet_process_names.begin(), emotet_process_names.end());
    emotet_process_names.erase(std::unique(emotet_process_names.begin(), emotet_process_names.end()), emotet_process_names.end());

    // debug message
    if (is_debug) {
        std::cout << "[DEBUG] Search following name(s) from running processes: ";
        for (unsigned int i = 0; i < emotet_process_names.size(); i++) {
            std::cout << emotet_process_names[i] << "  ";
        }
        std::cout << std::endl;
    }
    emotet_processes = ScanEmotetProcess(emotet_process_names);
    return emotet_processes;
}

}  // namespace emocheck
