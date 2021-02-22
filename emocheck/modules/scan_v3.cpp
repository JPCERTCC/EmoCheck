#include "scan.hpp"

#pragma comment(lib, "shlwapi.lib")

namespace emocheck {

std::unordered_set<std::string> GenerateKeyWordFromDirectory(std::string path, std::unordered_set<std::string> keywords) {
    /* 
    This module generates keywords set from specified folder.
    */
    HANDLE hFind;
    WIN32_FIND_DATAA win32_find_data;
    std::string search_path = path + "\\*";
    hFind = FindFirstFileA(search_path.c_str(), &win32_find_data);

    if (hFind == INVALID_HANDLE_VALUE) {
        return keywords;
    }
    do {
        if (win32_find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            std::string directory_name = win32_find_data.cFileName;
            if (directory_name != "." & directory_name != "..") {
                // recursive process will come here
            }
        } else {
            std::string filename = win32_find_data.cFileName;
            if (CheckPeExtension(filename)) {
                keywords.insert(filename.erase(filename.find_last_of(".")));
            }
        }
    } while (FindNextFileA(hFind, &win32_find_data));

    FindClose(hFind);

    return keywords;
}

std::unordered_set<std::string> GenerateKeywordlistFromSystemDirectory() {
    std::unordered_set<std::string> emotet_v3_keywords;
    wchar_t system_path[MAX_PATH];
    GetSystemDirectoryW(system_path, MAX_PATH);
    std::string search_path = WideCharToString(system_path);
    emotet_v3_keywords = GenerateKeyWordFromDirectory(search_path, emotet_v3_keywords);
    return emotet_v3_keywords;
}

std::vector<EmotetLoader> SearchSuspicoiusExeFile(std::string path, std::unordered_set<std::string> keywords, std::vector<EmotetLoader> result) {
    HANDLE hFind;
    WIN32_FIND_DATAA win32_find_data;
    EmotetLoader suspicious_file;
    std::string search_path = path + "\\*";
    hFind = FindFirstFileA(search_path.c_str(), &win32_find_data);
    search_path.erase(search_path.find_last_of("*"));

    if (hFind == INVALID_HANDLE_VALUE) {
        return result;
    }

    do {
        if (win32_find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            std::string directory_name = win32_find_data.cFileName;
            if (keywords.find(directory_name) != keywords.end()) {
            }
        } else {
            std::string filename = win32_find_data.cFileName;
            if (filename.substr(filename.find_last_of(".") + 1) == "exe") {
                std::string word = filename.erase(filename.find_last_of("."));
                if (keywords.find(word) != keywords.end()) {
                    suspicious_file.filename = word + ".exe";
                    suspicious_file.filepath = search_path + word + ".exe";
                    suspicious_file.version = 3;
                    result.push_back(suspicious_file);
                }
            }
        }
    } while (FindNextFileA(hFind, &win32_find_data));

    FindClose(hFind);

    return result;
}

std::vector<EmotetLoader> SearchSpecialFolder(int csidl, std::unordered_set<std::string> keywords, std::vector<EmotetLoader> suspicious_files) {
    /* 
    This search supicous folder name from given keyword set.
    */
    wchar_t path[MAX_PATH];

    SHGetSpecialFolderPathW(NULL, path, csidl, 0);

    HANDLE hFind;
    WIN32_FIND_DATAA win32_find_data;
    std::string search_path = WideCharToString(path);
    search_path += "\\*";

    hFind = FindFirstFileA(search_path.c_str(), &win32_find_data);

    if (hFind == INVALID_HANDLE_VALUE) {
        return suspicious_files;
    }

    do {
        if (win32_find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            std::string directory_name = win32_find_data.cFileName;
            if (keywords.find(directory_name) != keywords.end()) {
                suspicious_files = SearchSuspicoiusExeFile(WideCharToString(path) + "\\" + directory_name, keywords, suspicious_files);
            }
        }
    } while (FindNextFileA(hFind, &win32_find_data));

    FindClose(hFind);

    return suspicious_files;
}

std::string ReadRegRunKey(HKEY root, std::string value) {
    DWORD buf_len;
    LSTATUS status;
    HKEY hKey;
    std::string result;
    std::string base_path = "Software\\Microsoft\\Windows\\CurrentVersion\\Run";

    status = RegOpenKeyExA(root, base_path.c_str(), 0, KEY_READ, &hKey);
    if (status) {
        return result;
    };

    status = RegGetValueA(hKey, nullptr, value.c_str(), RRF_RT_REG_SZ, nullptr, nullptr, &buf_len);
    if (status) {
        return result;
    }

    unsigned char *buffer = new unsigned char[buf_len]();
    RegGetValueA(hKey, nullptr, value.c_str(), RRF_RT_REG_SZ, nullptr, buffer, &buf_len);

    result = (const char *)buffer;
    CloseHandle(hKey);
    return result;
}

std::vector<EmotetLoader> SearchRegistryRunKey(std::vector<EmotetLoader> suspicious_files) {
    for (unsigned int i = 0; i < suspicious_files.size(); i++) {
        std::string value = suspicious_files[i].filename;
        value = value.erase(value.find_last_of("."));
        if (suspicious_files[i].filepath == ReadRegRunKey(HKEY_CURRENT_USER, value)) {
            suspicious_files[i].run_key = "HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Run\\" + value;
        }
    }
    return suspicious_files;
}

std::vector<EmotetLoader> SuspicousFileValidation(std::vector<EmotetLoader> suspicious_files) {
    /* If suspicious file is Emotet in high confidence, validation field will be "True". */
    wchar_t system_path[MAX_PATH];
    GetSystemDirectoryW(system_path, MAX_PATH);
    std::string sys_path = WideCharToString(system_path);

    for (unsigned int i = 0; i < suspicious_files.size(); i++) {
        long long int filesize = AcquireFileSize(suspicious_files[i].filepath.c_str());
        // file size check
        if (filesize < 10000) {
            suspicious_files[i].file_validation = FALSE;
            continue;
        }
        // md5 validation
        // CalculateMD5Hash(suspicious_files[i].filepath.c_str());

        // check system directory
        std::string sys_file = sys_path + "\\" + suspicious_files[i].filename;
        if (PathFileExistsA(sys_file.c_str())) {
            long long int sys_filesize = AcquireFileSize(sys_file.c_str());
            if (sys_filesize == filesize) {
                suspicious_files[i].file_validation = FALSE;
            } else {
                suspicious_files[i].file_validation = TRUE;
            }
        } else {
            suspicious_files[i].file_validation = TRUE;
        }
    }
    return suspicious_files;
}

std::vector<EmotetLoader> EmotetScannerV3() {
    std::unordered_set<std::string> keywords = GenerateKeywordlistFromSystemDirectory();
    std::vector<EmotetLoader> suspicious_files;

    suspicious_files = SearchSpecialFolder(CSIDL_LOCAL_APPDATA, keywords, suspicious_files);
    suspicious_files = SearchSpecialFolder(CSIDL_SYSTEMX86, keywords, suspicious_files);
    suspicious_files = SearchRegistryRunKey(suspicious_files);
    suspicious_files = SuspicousFileValidation(suspicious_files);

    return suspicious_files;
}

}  // namespace emocheck