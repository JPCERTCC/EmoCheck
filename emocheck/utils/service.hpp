#ifndef EMOCHECK_UTILS_SERVICE_HPP_
#define EMOCHECK_UTILS_SERVICE_HPP_

// standard modules
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

// windows basic modules
#include <Windows.h>

#include "winternl.h"

// windows additional modules
#include <Psapi.h>

#include "utils.hpp"

#ifdef DEBUG
#define DBG(X) std::cerr << "[Dbg] " << X << std::endl
#else
#define DBG(X)
#endif

namespace emocheck {

// string
static const char* ACTIVE = "active";
static const char* NON_ACTIVE = "not active";

// struct
struct WinService {
    std::string name;
    std::string display_name;
    std::string binary_path;
    DWORD start_type;
    DWORD state;
    DWORD type;
};

//function
std::vector<WinService> ListWinServices();
LPQUERY_SERVICE_CONFIG GetWinServiceConfig(std::string name);

}  // namespace emocheck
#endif  // EMOCHECK_UTILS_SERVICE_HPP_