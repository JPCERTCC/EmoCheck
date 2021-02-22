/*
 * LICENSE
 * Please refer to the LICENSE.txt at https://github.com/JPCERTCC/EmoCheck/
 */

#ifndef EMOCHECK_UTILS_PROCESS_HPP_
#define EMOCHECK_UTILS_PROCESS_HPP_

// standard modules
#include <iostream>
#include <sstream>
#include <tuple>
#include <unordered_set>
#include <vector>

// windows basic modules
#include <Windows.h>

#include "winternl.h"

// windows additional modules
#include <Psapi.h>
#include <TlHelp32.h>

#ifdef DEBUG
#define DBG(X) std::cerr << "[Dbg] " << X << std::endl
#else
#define DBG(X)
#endif

namespace emocheck {

struct Proc {
    int PID;
    std::string name;
    std::string image_path;
    std::string cmd_line;
};

std::string GetImageFileName(DWORD pid);
std::vector<Proc> ListProcess();

}  //namespace emocheck

#endif  //EMOCHECK_UTILS_PROCESS_HPP_