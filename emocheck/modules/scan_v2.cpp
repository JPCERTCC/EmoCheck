#include "scan.hpp"

namespace emocheck {

EmotetLoader QueryEmotetV2Registry(unsigned int serial, HKEY root, std::wstring key_path) {
    DWORD buf_len;
    LSTATUS status;
    HKEY hKey;
    std::wstringstream wstring_serial;
    EmotetLoader filename;

    // convert integer to wstring stream
    wstring_serial << std::hex << serial;

    status = RegOpenKeyExW(root, key_path.c_str(), 0, KEY_READ, &hKey);
    if (status) {
        // std::cerr << "[*] Emotet registry key not found. status: " << status << std::endl;
        return filename;
    }

    status = RegGetValueW(hKey, nullptr, wstring_serial.str().c_str(), RRF_RT_REG_BINARY, nullptr, nullptr, &buf_len);
    if (status) {
        // std::cerr << "[*] Emotet registry key not found. status: " << status << std::endl;
        CloseHandle(hKey);
        return filename;
    }

    // buffur to save binary_data in registry value.
    unsigned char *buffer = new unsigned char[buf_len]();

    RegGetValueW(hKey, nullptr, wstring_serial.str().c_str(), RRF_RT_REG_BINARY, nullptr, buffer, &buf_len);

    // XOR registry value with drive serial num;
    unsigned char *decoded_chars = new unsigned char[buf_len]();
    std::vector<unsigned char> xor_key = IntToBytes(serial);

    for (unsigned int i = 0; i < buf_len; i++) {
        decoded_chars[i] = (xor_key[i % 4] ^ buffer[i]);
    }

    for (unsigned int i = 0; i < buf_len; i++) {
        if (0x20 < int(decoded_chars[i]) && int(decoded_chars[i]) < 0x7e) {
            filename.filename += decoded_chars[i];
        }
    }
    // set emotet loader version info
    filename.filename = filename.filename + ".exe";
    filename.version = 2;

    CloseHandle(hKey);
    return filename;
}

std::vector<EmotetLoader> GetEmotetV2FileNameFromRegistry(int serial) {
    std::vector<EmotetLoader> suspicious_files;
    EmotetLoader suspicious_file;
    std::wstring reg_key_path = L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer";
    std::wstring reg_key_path_admin = L"Software\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Explorer";

    // if emotet runs with user auth.(x64,x32)
    suspicious_file = QueryEmotetV2Registry(serial, HKEY_CURRENT_USER, reg_key_path);
    if (suspicious_file.filename.length() > 0) {
        suspicious_files.push_back(suspicious_file);
    }

    // if emotet runs with admin auth. (x32)
    suspicious_file = QueryEmotetV2Registry(serial, HKEY_LOCAL_MACHINE, reg_key_path);
    if (suspicious_file.filename.length() > 0) {
        suspicious_files.push_back(suspicious_file);
    }

    // if emotet runs with admin auth. (x64)
    suspicious_file = QueryEmotetV2Registry(serial, HKEY_LOCAL_MACHINE, reg_key_path_admin);
    if (suspicious_file.filename.length() > 0) {
        suspicious_files.push_back(suspicious_file);
    }
    return suspicious_files;
}

}  // namespace emocheck