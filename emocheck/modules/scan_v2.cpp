/*
 * LICENSE
 * Please refer to the LICENSE.txt at https://github.com/JPCERTCC/EmoCheck/
 */
#include "scan_v2.hpp"

#include "../utils/utils.hpp"

// standard modules
#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <tuple>
#include <unordered_set>
#include <vector>

// windows basic modules
#include <Windows.h>

namespace emocheck {

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
    /*
    if (is_debug) {
        if (root == HKEY_CURRENT_USER) {
            std::wcout << L"[DEBUG] Open registry: HKCU\\" << key_path << std::endl;
        } else if (root == HKEY_LOCAL_MACHINE) {
            std::wcout << L"[DEBUG] Open registry: HKLM\\" << key_path << std::endl;
        } else {
            std::wcout << L"[DEBUG] Open registry: " << std::hex << root << L" path: " << key_path << std::endl;
        }
    }
    */

    status = RegGetValue(hKey, nullptr, wstring_serial.str().c_str(), RRF_RT_REG_BINARY, nullptr, nullptr, &buf_len);
    if (status) {
        // std::cerr << "[*] Emotet registry key not found. status: " << status << std::endl;
        return filename;
    }

    // debug message
    /*
    if (is_debug) {
        std::wcout << L"[DEBUG] Found suspicous subkey: " << wstring_serial.str() << std::endl;
        std::cout << "[DEBUG] Data length: " << buf_len << std::endl;
    }
    */

    // buffur to save binary_data in registry value.
    unsigned char *buffer = new unsigned char[buf_len];

    RegGetValue(hKey, nullptr, wstring_serial.str().c_str(), RRF_RT_REG_BINARY, nullptr, buffer, &buf_len);

    // XOR registry value with drive serial num;
    unsigned char *decoded_chars = new unsigned char[buf_len];
    std::vector<unsigned char> xor_key = IntToBytes(serial);

    // debug message
    /*
    if (is_debug) {
        std::cout << "[DEBUG] Binary data:\n[DEBUG] ";
        for (unsigned int i = 0; i < buf_len; i++) {
            printf("0x%02x ", buffer[i]);
        }
        std::cout << std::endl;
    }
    */

    for (unsigned int i = 0; i < buf_len; i++) {
        decoded_chars[i] = (xor_key[i % 4] ^ buffer[i]);
    }

    // debug message
    /*
    if (is_debug) {
        std::cout << "[DEBUG] Decoded data:\n[DEBUG] ";
        for (unsigned int i = 0; i < buf_len; i++) {
            printf("0x%02x ", decoded_chars[i]);
        }
        std::cout << std::endl;
    }
    */

    for (unsigned int i = 0; i < buf_len; i++) {
        if (0x20 < int(decoded_chars[i]) && int(decoded_chars[i]) < 0x7e) {
            filename += decoded_chars[i];
        }
    }
    /*
    // debug message
    if (is_debug) {
        std::cout << "[DEBUG] Decoded string: " << filename << std::endl;
    }
    */
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

}  // namespace emocheck