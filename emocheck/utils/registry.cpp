#include "registry.hpp"

#include "utils.hpp"

namespace emocheck {

const static unsigned int MAX_KEY_LENGTH = 255;
const static unsigned int MAX_VALUE_NAME = 16383;

RegKeyValue *LookupRegByName(HKEY root, std::string path, std::string name) {
    DBG("[LookupRegByName] Function called. arg1: " << root << "\targ2: " << path << "\targ3: " << name);
    LSTATUS status;
    HKEY hKey;
    DWORD size;
    RegKeyValue *regkey = new RegKeyValue();

    status = RegOpenKeyExA(root, path.c_str(), 0, KEY_READ, &hKey);
    if (status) {
        DBG("Fail to open Registry handle in runkey. Reg value name: " << name << "\tError code: " << status);
        return nullptr;
    }

    // get size of registry value
    status = RegGetValueA(hKey, nullptr, name.c_str(), RRF_RT_REG_SZ, nullptr, nullptr, &size);
    if (status) {
        DBG("Fail to get size of runkey. Reg value name: " << name << "\tError code: " << status);
        CloseHandle(hKey);
        return nullptr;
    }

    DBG("[LookupRegByName] Reg size: " << size);

    // read value
    unsigned char *buffer;
    buffer = (unsigned char *)calloc(1, size);
    status = RegGetValueA(hKey, nullptr, name.c_str(), RRF_RT_REG_SZ, nullptr, buffer, &size);
    if (status) {
        DBG("Fail to read value of runkey. Reg value name: " << name << "\tError code: " << status);
        CloseHandle(hKey);
        return nullptr;
    }

    DBG("[LookupRegByName] Buffer: " << (const char *)buffer);

    regkey->name = name;
    regkey->value = (const char *)buffer;
    CloseHandle(hKey);
    DBG("[LookupRegByName] Function return.");
    return regkey;
}

std::vector<RegKeyValue *> ListRegSZValues(HKEY root, std::string reg_path) {
    std::vector<RegKeyValue *> keys;
    LSTATUS status;
    HKEY hKey;
    DWORD val_cnts;
    DWORD max_name_len;
    DWORD max_value_size;

    status = RegOpenKeyExA(root, reg_path.c_str(), 0, KEY_QUERY_VALUE, &hKey);

    if (status != ERROR_SUCCESS)
        return keys;

    // check the number of values.
    status = RegQueryInfoKeyA(hKey,
                              NULL, NULL, NULL, NULL, NULL, NULL,
                              &val_cnts,
                              &max_name_len,
                              &max_value_size,
                              NULL, NULL);

    if (status != ERROR_SUCCESS) {
        CloseHandle(hKey);
        return keys;
    }

    DWORD buf_size;
    DWORD type;

    // Enumerate registry REG_SZ values
    for (DWORD idx = 0; idx < val_cnts; idx++) {
        WCHAR *name = new WCHAR[MAX_VALUE_NAME]();
        RegKeyValue *reg_val = new RegKeyValue();
        DWORD size = MAX_KEY_LENGTH;

        // read the registry value name

        status = RegEnumValueW(hKey, idx, name, &size, NULL, &type, NULL, &buf_size);
        if (status != ERROR_SUCCESS || type != REG_SZ) {
            DBG("[ListRegSZValues] error: " << status);
            continue;
        }

        reg_val->name = WideCharToString(name);

        // read the registry value
        unsigned char *buffer = new unsigned char[buf_size]();
        RegGetValueA(hKey, nullptr, reg_val->name.c_str(), RRF_RT_REG_SZ, nullptr, buffer, &buf_size);
        if (status != ERROR_SUCCESS) {
            DBG("[ListRegSZValues] error: " << status);
            continue;
        }
        reg_val->value = (const char *)buffer;

        keys.push_back(reg_val);
    }

    CloseHandle(hKey);

    return keys;
}

}  // namespace emocheck