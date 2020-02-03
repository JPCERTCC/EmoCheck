/**
 * @file   emocheck.cpp
 * @author t-tani (JPCERT/CC)
 */

#include "emocheck.h"

// standard modules
#include <iostream>
#include <vector>

// windows basic modules
#include <Windows.h>

// windows additional modules
#include <Psapi.h>
#include <TlHelp32.h>

namespace emocheck {

std::string WideCharToString(wchar_t *wide_char) {
    int buf_size = WideCharToMultiByte(CP_OEMCP, 0, wide_char, -1, (char *)NULL, 0, NULL, NULL);
    char *char_ptr = new char[buf_size];

    WideCharToMultiByte(CP_OEMCP, 0, wide_char, -1, char_ptr, buf_size, NULL, NULL);
    std::string result(char_ptr, char_ptr + buf_size - 1);
    delete[] char_ptr;

    return (result);
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
        std::clog << "[!!] Error: Fail to get handle from pid " << pid << std::endl;
        std::exit(1);
    }
    if (GetProcessImageFileName(hProcess, imagepath, sizeof(imagepath) / sizeof(*imagepath)) == 0) {
        std::clog << "[!!] Error: Fail to get image file name from pid " << pid << std::endl;
        std::exit(1);
    } else {
        return ConvertDivecePath(imagepath);
    }
}

int GetVolumeSerialNumber() {
    wchar_t volumename[MAX_PATH + 1] = {0};
    wchar_t filesystemname[MAX_PATH + 1] = {0};
    DWORD serialnumber = 0;
    DWORD max_componentlen = 0;
    DWORD filesystem_flags = 0;
    if (GetVolumeInformation(
            L"C:\\",
            volumename,
            sizeof(volumename),
            &serialnumber,
            &max_componentlen,
            &filesystem_flags,
            filesystemname,
            sizeof(filesystemname)) == TRUE) {
        // std::cout << "[debug] Serial Number :" << std::hex << serialnumber << std::endl;
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

    keywords =
        "duck,mfidl,targets,ptr,khmer,purge,metrics,acc,inet,msra,symbol,driver,"
        "sidebar,restore,msg,volume,cards,shext,query,roam,etw,mexico,basic,url,"
        "createa,blb,pal,cors,send,devices,radio,bid,format,thrd,taskmgr,timeout,"
        "vmd,ctl,bta,shlp,avi,exce,dbt,pfx,rtp,edge,mult,clr,wmistr,ellipse,vol,"
        "cyan,ses,guid,wce,wmp,dvb,elem,channel,space,digital,pdeft,violet,thunk";

    keylen = int(keywords.length());

    // first round
    seed = GetVolumeSerialNumber();
    q = seed / keylen;
    mod = seed % keylen;
    keyword += GetWord(keywords, mod, keylen);

    // second round
    seed = 0xFFFFFFFF - q;
    mod = seed % keylen;
    keyword += GetWord(keywords, mod, keylen);

    // std::cout << "[debug] Emotet process name on this host is " << keyword.c_str() << ".exe" << std::endl;
    return keyword;
}

std::vector<EmotetProcess> ScanEmotetProcess(std::string keyword) {
    PROCESSENTRY32 pe = {sizeof(PROCESSENTRY32)};
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    BOOL result;
    std::vector<EmotetProcess> emotet_processes;

    for (result = Process32First(snapshot, &pe);
         result == TRUE; result = Process32Next(snapshot, &pe)) {
        std::string process_name = WideCharToString(pe.szExeFile);
        if (process_name.find(keyword) != std::string::npos) {
            EmotetProcess emotet_process;
            emotet_process.pid = pe.th32ProcessID;
            emotet_process.process_name = process_name;
            emotet_process.image_path = GetImageFileName(emotet_process.pid);
            emotet_processes.push_back(emotet_process);
        }
    }
    return emotet_processes;
}

std::vector<EmotetProcess> ScanEmotet() {
    std::string emotet_process_name;
    std::vector<EmotetProcess> emotet_processes;

    emotet_process_name = GenerateEmotetProcessName();
    emotet_processes = ScanEmotetProcess(emotet_process_name);

    return emotet_processes;
}

}  // namespace emocheck
