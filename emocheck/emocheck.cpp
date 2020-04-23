/*
 * LICENSE
 * Please refer to the LICENSE.txt at https://github.com/JPCERTCC/EmoCheck/
 */

#include "emocheck.hpp"

#include "modules/scan_v1.hpp"
#include "modules/scan_v2.hpp"
#include "utils/utils.hpp"

// standard modules
#include <algorithm>
#include <iostream>
#include <sstream>
#include <tuple>
#include <vector>

// windows basic modules
#include <Windows.h>

// windows additional modules
#include <Psapi.h>
#include <TlHelp32.h>

namespace emocheck {

bool is_debug = false;

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

std::tuple<int, std::vector<EmotetProcess>> ScanEmotet(bool debug) {
    std::vector<std::string> filenames;
    std::string emotet_process_name;
    std::vector<std::string> emotet_process_names;
    std::vector<EmotetProcess> emotet_processes;
    static const int NOT_INFECTED = 0;
    static const int INFECTED = 1;
    int is_infected;

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

    if (emotet_processes.empty()) {
        is_infected = NOT_INFECTED;
    } else {
        is_infected = INFECTED;
    }
    return std::forward_as_tuple(is_infected, emotet_processes);
}

}  // namespace emocheck
