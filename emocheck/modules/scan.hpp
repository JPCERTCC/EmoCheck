#ifndef EMOCHECK_MODULES_SCAN_HPP_
#define EMOCHECK_MODULES_SCAN_HPP_

// standard modules
#include <algorithm>
#include <array>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <tuple>
#include <unordered_set>
#include <utility>
#include <vector>

// windows basic modules
#include <Windows.h>

// windows additional modules
#include <Psapi.h>
#include <TlHelp32.h>
#include <shlobj.h>
#include <shlwapi.h>

#include "../emocheck.hpp"
#include "../utils/file.hpp"
#include "../utils/registry.hpp"
#include "../utils/service.hpp"
#include "../utils/utils.hpp"

namespace emocheck {

// scan_v1.cpp
EmotetLoader GenerateEmotetV1ProcessName();

// scan_v2.cpp
std::vector<EmotetLoader> GetEmotetV2FileNameFromRegistry(int serial);

// scan_v3.cpp
std::vector<EmotetLoader> EmotetScannerV3();

// scan_v4.cpp
std::vector<EmotetLoader> EmotetScannerV4();

//scan_v5.cpp
std::vector<EmotetLoader> EmotetScannerV5();

}  // namespace emocheck

#endif  //EMOCHECK_MODULES_SCAN_V1_HPP_